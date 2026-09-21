#include "planetglwidget.h"
#include "planet.h"
#include "starspectrum.h"
#include <QJsonObject>
#include <QImage>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <QWheelEvent>
#include <QQuaternion>
#include <QHash>
#include <QPair>
#include <QByteArray>
#include <QtMath>
#include <cmath>

namespace {
const float kFovDeg = 42.0f;
const float kDefaultFill = 0.6f;
const float kDefaultAzimuth = 0.0f;
const float kDefaultElevation = 18.0f;
const int kCameraResetMs = 1300;
const float kPlanetSpinDps = 15.0f;
const float kCloudSpinDps = 6.0f;
const float kRingSpinDps = 4.0f;

float wrapDeg(float a)
{
    a = std::fmod(a, 360.0f);
    if (a < 0.0f)
        a += 360.0f;
    return a;
}

quint32 wangHash(quint32 x)
{
    x = (x ^ 61u) ^ (x >> 16);
    x *= 9u;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2du;
    x = x ^ (x >> 15);
    return x;
}

float hashDir(const QVector3D &n, quint32 seed)
{
    const quint32 hx = quint32(qint32(qRound(n.x() * 4096.0f)));
    const quint32 hy = quint32(qint32(qRound(n.y() * 4096.0f)));
    const quint32 hz = quint32(qint32(qRound(n.z() * 4096.0f)));
    const quint32 h = wangHash(hx * 73856093u ^ hy * 19349663u ^ hz * 83492791u ^ seed);
    return float(h & 0x00ffffffu) / float(0x00ffffffu);
}

float rockNoise(const QVector3D &n, quint32 seed)
{
    const float a = hashDir(n, seed);
    const float b = hashDir(n, seed ^ 0x9e3779b9u);
    return (a * 0.65f + b * 0.35f) * 2.0f - 1.0f;
}

struct IcoMesh
{
    QVector<QVector3D> v;
    QVector<int> idx;
};

int icoMid(QVector<QVector3D> &v, QHash<QPair<int, int>, int> &mids, int a, int b)
{
    const QPair<int, int> key(qMin(a, b), qMax(a, b));
    const auto it = mids.constFind(key);
    if (it != mids.cend())
        return it.value();
    const QVector3D p = (v[a] + v[b]).normalized();
    const int id = v.size();
    v.append(p);
    mids.insert(key, id);
    return id;
}

const IcoMesh &rockIcoMesh()
{
    static IcoMesh mesh;
    static bool ready = false;
    if (ready)
        return mesh;
    const float t = 0.5f * (1.0f + std::sqrt(5.0f));
    const QVector3D raw[] = {
        QVector3D(-1, t, 0), QVector3D(1, t, 0), QVector3D(-1, -t, 0), QVector3D(1, -t, 0),
        QVector3D(0, -1, t), QVector3D(0, 1, t), QVector3D(0, -1, -t), QVector3D(0, 1, -t),
        QVector3D(t, 0, -1), QVector3D(t, 0, 1), QVector3D(-t, 0, -1), QVector3D(-t, 0, 1)
    };
    mesh.v.reserve(42);
    for (const QVector3D &p : raw)
        mesh.v.append(p.normalized());
    const int faces[][3] = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };
    QVector<int> idx;
    idx.reserve(60);
    for (int i = 0; i < 20; ++i)
    {
        idx.append(faces[i][0]);
        idx.append(faces[i][1]);
        idx.append(faces[i][2]);
    }
    QHash<QPair<int, int>, int> mids;
    mesh.idx.reserve(240);
    for (int i = 0; i + 2 < idx.size(); i += 3)
    {
        const int a = idx[i];
        const int b = idx[i + 1];
        const int c = idx[i + 2];
        const int ab = icoMid(mesh.v, mids, a, b);
        const int bc = icoMid(mesh.v, mids, b, c);
        const int ca = icoMid(mesh.v, mids, c, a);
        mesh.idx << a << ab << ca << b << bc << ab << c << ca << bc << ab << bc << ca;
    }
    ready = true;
    return mesh;
}

QVector3D deformRockPoint(const RingRock &rock, const QVector3D &unitN)
{
    const QVector3D n = unitN.normalized();
    const float k = 1.0f + 0.28f * rockNoise(n, rock.noiseSeed);
    const QVector3D local(n.x() * rock.sx * k, n.y() * rock.sy * k, n.z() * rock.sz * k);
    const QQuaternion rot = QQuaternion::fromEulerAngles(rock.pitch, rock.yaw, rock.roll);
    return QVector3D(rock.x, rock.y, rock.z) + rot.rotatedVector(local * rock.radius);
}

}

static const char *kEquirectUv =
    "vec2 equirectUv(vec3 p) {\n"
    "  p = normalize(p);\n"
    "  float lon = atan(p.z, p.x);\n"
    "  float lat = asin(clamp(p.y, -1.0, 1.0));\n"
    "  float u = lon * 0.15915494309189535;\n"
    "  if (u < 0.0) u += 1.0;\n"
    "  float v = 0.5 - lat * 0.3183098861837907;\n"
    "  return vec2(u, v);\n"
    "}\n";

static const char *kPlanetVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "attribute vec3 aNormal;\n"
    "attribute vec2 aUv;\n"
    "uniform mat4 uMvp;\n"
    "uniform mat4 uModel;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vLocal;\n"
    "void main() {\n"
    "  vec4 wp = uModel * vec4(aPos, 1.0);\n"
    "  vWorld = wp.xyz;\n"
    "  vLocal = aPos;\n"
    "  vNormal = mat3(uModel) * aNormal;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kPlanetFrag =
    "#version 120\n"
    "uniform sampler2D uAlbedo;\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform float uLightI;\n"
    "uniform vec3 uLightColor;\n"
    "uniform float uFillLight;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vLocal;\n";

static const char *kPlanetFragBody =
    "void main() {\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  vec3 viewDir = normalize(uCam - vWorld);\n"
    "  float fillI = 0.3 * 0.5;\n"
    "  vec3 tint = uLightI > 0.001 ? uLightColor : vec3(1.0);\n"
    "  float lit = max(dot(n, l), 0.0) * uLightI;\n"
    "  if (uFillLight > 0.5)\n"
    "    lit += max(dot(n, viewDir), 0.0) * fillI;\n"
    "  vec3 albedo = texture2D(uAlbedo, equirectUv(vLocal)).rgb;\n"
    "  vec3 col = albedo * lit * tint;\n"
    "  gl_FragColor = vec4(col, 1.0);\n"
    "}\n";

static const char *kCloudVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "attribute vec3 aNormal;\n"
    "attribute vec2 aUv;\n"
    "uniform mat4 uMvp;\n"
    "uniform mat4 uModel;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vLocal;\n"
    "void main() {\n"
    "  vec4 wp = uModel * vec4(aPos, 1.0);\n"
    "  vWorld = wp.xyz;\n"
    "  vLocal = aPos;\n"
    "  vNormal = mat3(uModel) * aNormal;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kCloudFrag =
    "#version 120\n"
    "uniform sampler2D uClouds;\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform float uLightI;\n"
    "uniform vec3 uLightColor;\n"
    "uniform float uFillLight;\n"
    "uniform float uCloudAlpha;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vLocal;\n";

static const char *kCloudFragBody =
    "void main() {\n"
    "  vec4 c = texture2D(uClouds, equirectUv(vLocal));\n"
    "  float a = c.a * uCloudAlpha;\n"
    "  if (a < 0.02) discard;\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  vec3 viewDir = normalize(uCam - vWorld);\n"
    "  vec3 tint = uLightI > 0.001 ? uLightColor : vec3(1.0);\n"
    "  float wrap = max(dot(n, l) * 0.5 + 0.5, 0.0);\n"
    "  float lit = wrap * uLightI;\n"
    "  if (uFillLight > 0.5)\n"
    "    lit += max(dot(n, viewDir), 0.0) * 0.2;\n"
    "  gl_FragColor = vec4(c.rgb * lit * tint, a);\n"
    "}\n";

static const char *kAtmoVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "attribute vec3 aNormal;\n"
    "uniform mat4 uMvp;\n"
    "uniform mat4 uModel;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "void main() {\n"
    "  vec4 wp = uModel * vec4(aPos, 1.0);\n"
    "  vWorld = wp.xyz;\n"
    "  vNormal = mat3(uModel) * aNormal;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kAtmoFrag =
    "#version 120\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform vec3 uAtmoColor;\n"
    "uniform float uLightI;\n"
    "uniform vec3 uLightColor;\n"
    "uniform float uFillLight;\n"
    "uniform float uAtmo;\n"
    "uniform float uAtmoSize;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "void main() {\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  vec3 viewDir = normalize(uCam - vWorld);\n"
    "  float ndv = max(dot(n, viewDir), 0.0);\n"
    "  float ndl = dot(n, l);\n"
    "  float lambert = max(ndl, 0.0);\n"
    "  float wrap = ndl * 0.5 + 0.5;\n"
    "  float diffuse = mix(lambert, wrap * wrap, uAtmo);\n"
    "  float limb = pow(1.0 - ndv, mix(4.8, 0.35, uAtmoSize));\n"
    "  float density = mix(limb, 1.0, uAtmo) * uAtmo;\n"
    "  float lit = max(diffuse, 0.0) * uLightI;\n"
    "  if (uFillLight > 0.5)\n"
    "    lit += ndv * 0.15;\n"
    "  float haze = density;\n"
    "  if (uAtmo < 0.995)\n"
    "  {\n"
    "    haze *= smoothstep(-0.12, 0.2, ndl);\n"
    "    if (uFillLight > 0.5)\n"
    "      haze = max(haze, density * 0.25);\n"
    "  }\n"
    "  vec3 tint = uLightI > 0.001 ? uLightColor : vec3(1.0);\n"
    "  vec3 col = uAtmoColor * tint * lit;\n"
    "  gl_FragColor = vec4(col, clamp(haze, 0.0, 1.0));\n"
    "}\n";

static const char *kRingVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "attribute vec3 aNormal;\n"
    "attribute vec3 aColor;\n"
    "uniform mat4 uMvp;\n"
    "uniform mat4 uModel;\n"
    "varying vec3 vPos;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "  vPos = (uModel * vec4(aPos, 1.0)).xyz;\n"
    "  vNormal = mat3(uModel) * aNormal;\n"
    "  vColor = aColor;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kRingFrag =
    "#version 120\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform float uFillLight;\n"
    "uniform float uTwoSided;\n"
    "uniform float uPlanetR;\n"
    "varying vec3 vPos;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  float ndl = dot(n, l);\n"
    "  if (uTwoSided > 0.5) ndl = abs(ndl);\n"
    "  float pb = dot(vPos, l);\n"
    "  float impact = length(vPos - l * pb);\n"
    "  float shadow = 1.0;\n"
    "  if (pb < 0.0)\n"
    "    shadow = smoothstep(uPlanetR * 0.92, uPlanetR * 1.08, impact);\n"
    "  float shade = (0.45 + 0.55 * max(ndl, 0.0)) * shadow;\n"
    "  if (uFillLight > 0.5)\n"
    "    shade += 0.12 * max(dot(normalize(vPos), normalize(uCam)), 0.0);\n"
    "  gl_FragColor = vec4(vColor * shade, 0.92);\n"
    "}\n";

static const char *kBlitVert =
    "#version 120\n"
    "attribute vec2 aPos;\n"
    "attribute vec2 aUv;\n"
    "varying vec2 vUv;\n"
    "void main() {\n"
    "  vUv = aUv;\n"
    "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char *kBlitFrag =
    "#version 120\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 vUv;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(uTex, vUv);\n"
    "}\n";

static const char *kCityVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "uniform mat4 uMvp;\n"
    "uniform float uSize;\n"
    "void main() {\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "  gl_PointSize = uSize;\n"
    "}\n";

static const char *kCityFrag =
    "#version 120\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "  vec2 d = gl_PointCoord - vec2(0.5);\n"
    "  float a = exp(-dot(d, d) * 14.0);\n"
    "  gl_FragColor = vec4(uColor, a);\n"
    "}\n";

static const char *kStarVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "uniform mat4 uMvp;\n"
    "void main() {\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kStarFrag =
    "#version 120\n"
    "uniform vec3 uColor;\n"
    "uniform float uAlpha;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(uColor, uAlpha);\n"
    "}\n";

static const char *kDiskVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "uniform mat4 uMvp;\n"
    "varying vec2 vXZ;\n"
    "void main() {\n"
    "  vXZ = aPos.xy;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kDiskFrag =
    "#version 120\n"
    "varying vec2 vXZ;\n"
    "uniform float uTime;\n"
    "void main() {\n"
    "  float r = length(vXZ);\n"
    "  float inner = 1.05;\n"
    "  float outer = 3.2;\n"
    "  if (r < inner || r > outer) discard;\n"
    "  float t = (r - inner) / (outer - inner);\n"
    "  float ang = atan(vXZ.y, vXZ.x);\n"
    "  ang += uTime * pow(max(r, inner), -1.5);\n"
    "  float spokes = 0.5 + 0.5 * sin(ang * 9.0 + r * 4.0);\n"
    "  float grain = 0.5 + 0.5 * sin(ang * 21.0 - r * 8.0);\n"
    "  vec3 hot = vec3(1.0, 0.85, 0.35);\n"
    "  vec3 midc = vec3(1.0, 0.45, 0.08);\n"
    "  vec3 cool = vec3(0.55, 0.08, 0.02);\n"
    "  vec3 col = mix(hot, midc, smoothstep(0.0, 0.45, t));\n"
    "  col = mix(col, cool, smoothstep(0.45, 1.0, t));\n"
    "  col *= 0.55 + 0.45 * spokes;\n"
    "  col *= 0.7 + 0.3 * grain;\n"
    "  col *= 0.75 + 0.45 * clamp(vXZ.x / max(r, 0.01), -1.0, 1.0);\n"
    "  float alpha = 0.95 * (1.0 - smoothstep(0.75, 1.0, t));\n"
    "  alpha *= smoothstep(inner, inner + 0.12, r);\n"
    "  gl_FragColor = vec4(col, alpha);\n"
    "}\n";

static const char *kStarfieldVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "uniform mat4 uMvp;\n"
    "void main() {\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "  gl_Position.z = gl_Position.w * 0.999;\n"
    "  gl_PointSize = 1.8;\n"
    "}\n";

static const char *kStarfieldFrag =
    "#version 120\n"
    "void main() {\n"
    "  vec2 d = gl_PointCoord - vec2(0.5);\n"
    "  float a = exp(-dot(d, d) * 18.0);\n"
    "  gl_FragColor = vec4(0.9, 0.92, 1.0, a);\n"
    "}\n";

PlanetGLWidget::PlanetGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , planet(nullptr)
    , sphereVbo(QOpenGLBuffer::VertexBuffer)
    , ringVbo(QOpenGLBuffer::VertexBuffer)
    , rockVbo(QOpenGLBuffer::VertexBuffer)
    , blitVbo(QOpenGLBuffer::VertexBuffer)
    , cityVbo(QOpenGLBuffer::VertexBuffer)
    , starfieldVbo(QOpenGLBuffer::VertexBuffer)
    , diskVbo(QOpenGLBuffer::VertexBuffer)
    , sphereVertexCount(0)
    , ringVertexCount(0)
    , rockVertexCount(0)
    , cityVertexCount(0)
    , starfieldCount(0)
    , diskVertexCount(0)
    , azimuth(kDefaultAzimuth)
    , elevation(kDefaultElevation)
    , cameraDistance(defaultCameraDistance())
    , camResetTimer(new QTimer(this))
    , spinTimer(new QTimer(this))
    , camFromAz(kDefaultAzimuth)
    , camFromEl(kDefaultElevation)
    , camFromDist(defaultCameraDistance())
    , camDeltaAz(0.0f)
    , planetSpinDeg(0.0f)
    , cloudSpinDeg(0.0f)
    , ringSpinDeg(0.0f)
    , spinning(false)
    , dragging(false)
    , ready(false)
{
    setMinimumSize(257, 257);
    setFocusPolicy(Qt::WheelFocus);
    camResetTimer->setInterval(16);
    connect(camResetTimer, &QTimer::timeout, this, &PlanetGLWidget::tickCameraReset);
    spinTimer->setInterval(16);
    connect(spinTimer, &QTimer::timeout, this, &PlanetGLWidget::tickSpin);
}

PlanetGLWidget::~PlanetGLWidget()
{
    makeCurrent();
    sceneFbo.reset();
    sphereVbo.destroy();
    ringVbo.destroy();
    rockVbo.destroy();
    blitVbo.destroy();
    cityVbo.destroy();
    starfieldVbo.destroy();
    diskVbo.destroy();
    albedo.reset();
    clouds.reset();
    doneCurrent();
}

void PlanetGLWidget::setPlanet(const Planet *p)
{
    planet = p;
    if (ready)
    {
        refreshTextures();
        rebuildRings();
        rebuildCities();
        rebuildStarfield();
        refreshAppearance();
        update();
    }
}

int PlanetGLWidget::currentViewRes() const
{
    if (!planet)
        return 256;
    return planet->viewResolution();
}

float PlanetGLWidget::defaultCameraDistance()
{
    const float half = qDegreesToRadians(kFovDeg * kDefaultFill * 0.5f);
    return 1.0f / qSin(half);
}

void PlanetGLWidget::stopCameraReset()
{
    camResetTimer->stop();
}

void PlanetGLWidget::tickCameraReset()
{
    const float t = qBound(0.0f, float(camResetClock.elapsed()) / float(kCameraResetMs), 1.0f);
    const float e = t * t * (3.0f - 2.0f * t);
    azimuth = camFromAz + camDeltaAz * e;
    elevation = camFromEl + (kDefaultElevation - camFromEl) * e;
    cameraDistance = camFromDist + (defaultCameraDistance() - camFromDist) * e;
    update();
    emit cameraChanged();
    if (t >= 1.0f)
    {
        azimuth = kDefaultAzimuth;
        elevation = kDefaultElevation;
        cameraDistance = defaultCameraDistance();
        stopCameraReset();
        emit cameraChanged();
    }
}

void PlanetGLWidget::resetCamera()
{
    stopCameraReset();
    camFromAz = azimuth;
    camFromEl = elevation;
    camFromDist = cameraDistance;
    camDeltaAz = kDefaultAzimuth - camFromAz;
    while (camDeltaAz > 180.0f)
        camDeltaAz -= 360.0f;
    while (camDeltaAz < -180.0f)
        camDeltaAz += 360.0f;
    if (qAbs(camDeltaAz) < 0.05f
        && qAbs(camFromEl - kDefaultElevation) < 0.05f
        && qAbs(camFromDist - defaultCameraDistance()) < 0.002f)
    {
        azimuth = kDefaultAzimuth;
        elevation = kDefaultElevation;
        cameraDistance = defaultCameraDistance();
        update();
        emit cameraChanged();
        return;
    }
    camResetClock.start();
    tickCameraReset();
    camResetTimer->start();
}

void PlanetGLWidget::setSpinning(bool on)
{
    if (spinning == on)
        return;
    spinning = on;
    if (spinning)
    {
        spinClock.restart();
        spinTimer->start();
    }
    else
    {
        spinTimer->stop();
    }
}

void PlanetGLWidget::tickSpin()
{
    if (!spinning)
        return;
    const float dt = float(spinClock.restart()) * 0.001f;
    planetSpinDeg = wrapDeg(planetSpinDeg + kPlanetSpinDps * dt);
    cloudSpinDeg = wrapDeg(cloudSpinDeg + kCloudSpinDps * dt);
    ringSpinDeg = wrapDeg(ringSpinDeg + kRingSpinDps * dt);
    update();
}

QVector3D PlanetGLWidget::cameraPos() const
{
    const float el = qDegreesToRadians(qBound(-89.0f, elevation, 89.0f));
    const float az = qDegreesToRadians(azimuth);
    const float dist = cameraDistance;
    const float ce = qCos(el);
    return QVector3D(dist * ce * qSin(az), dist * qSin(el), dist * ce * qCos(az));
}

void PlanetGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 1);

    planetProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kPlanetVert);
    planetProg.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                      QByteArray(kPlanetFrag) + kEquirectUv + kPlanetFragBody);
    planetProg.bindAttributeLocation("aPos", 0);
    planetProg.bindAttributeLocation("aNormal", 1);
    planetProg.bindAttributeLocation("aUv", 2);
    planetProg.link();

    cloudProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kCloudVert);
    cloudProg.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     QByteArray(kCloudFrag) + kEquirectUv + kCloudFragBody);
    cloudProg.bindAttributeLocation("aPos", 0);
    cloudProg.bindAttributeLocation("aNormal", 1);
    cloudProg.bindAttributeLocation("aUv", 2);
    cloudProg.link();

    atmoProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kAtmoVert);
    atmoProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kAtmoFrag);
    atmoProg.bindAttributeLocation("aPos", 0);
    atmoProg.bindAttributeLocation("aNormal", 1);
    atmoProg.link();

    ringProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kRingVert);
    ringProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kRingFrag);
    ringProg.bindAttributeLocation("aPos", 0);
    ringProg.bindAttributeLocation("aNormal", 1);
    ringProg.bindAttributeLocation("aColor", 2);
    ringProg.link();

    blitProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kBlitVert);
    blitProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kBlitFrag);
    blitProg.bindAttributeLocation("aPos", 0);
    blitProg.bindAttributeLocation("aUv", 1);
    blitProg.link();

    cityProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kCityVert);
    cityProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kCityFrag);
    cityProg.bindAttributeLocation("aPos", 0);
    cityProg.link();

    starProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kStarVert);
    starProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kStarFrag);
    starProg.bindAttributeLocation("aPos", 0);
    starProg.link();

    diskProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kDiskVert);
    diskProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kDiskFrag);
    diskProg.bindAttributeLocation("aPos", 0);
    diskProg.link();

    starfieldProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kStarfieldVert);
    starfieldProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kStarfieldFrag);
    starfieldProg.bindAttributeLocation("aPos", 0);
    starfieldProg.link();

    buildSphere(96, 64);
    buildAccretionDisk();
    rebuildRings();
    buildBlitQuad();
    ready = true;
    rebuildStarfield();
    if (planet)
        refreshTextures();
}

void PlanetGLWidget::buildBlitQuad()
{
    const float quad[] = {
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f,
        -1.f,  1.f, 0.f, 1.f,
         1.f,  1.f, 1.f, 1.f
    };
    if (blitVbo.isCreated())
        blitVbo.destroy();
    blitVbo.create();
    blitVbo.bind();
    blitVbo.allocate(quad, sizeof(quad));
    blitVbo.release();
}

void PlanetGLWidget::ensureSceneFbo(int w, int h)
{
    w = qBound(16, w, 2048);
    h = qBound(16, h, 2048);
    if (sceneFbo && sceneFbo->width() == w && sceneFbo->height() == h)
        return;
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::Depth);
    sceneFbo.reset(new QOpenGLFramebufferObject(w, h, fmt));
}

void PlanetGLWidget::buildSphere(int slices, int stacks)
{
    QVector<float> data;
    data.reserve(stacks * slices * 6 * 8);
    for (int y = 0; y < stacks; ++y)
    {
        const float v0 = float(y) / stacks;
        const float v1 = float(y + 1) / stacks;
        const float phi0 = v0 * float(M_PI);
        const float phi1 = v1 * float(M_PI);
        for (int x = 0; x < slices; ++x)
        {
            const float u0 = float(x) / slices;
            const float u1 = float(x + 1) / slices;
            const float th0 = u0 * float(2.0 * M_PI);
            const float th1 = u1 * float(2.0 * M_PI);
            const QVector3D p00(qSin(phi0) * qCos(th0), qCos(phi0), qSin(phi0) * qSin(th0));
            const QVector3D p10(qSin(phi0) * qCos(th1), qCos(phi0), qSin(phi0) * qSin(th1));
            const QVector3D p01(qSin(phi1) * qCos(th0), qCos(phi1), qSin(phi1) * qSin(th0));
            const QVector3D p11(qSin(phi1) * qCos(th1), qCos(phi1), qSin(phi1) * qSin(th1));
            const QVector3D tri[6] = {p00, p10, p11, p00, p11, p01};
            const float uv[6][2] = {{u0, v0}, {u1, v0}, {u1, v1}, {u0, v0}, {u1, v1}, {u0, v1}};
            for (int i = 0; i < 6; ++i)
            {
                data << tri[i].x() << tri[i].y() << tri[i].z();
                data << tri[i].x() << tri[i].y() << tri[i].z();
                data << uv[i][0] << uv[i][1];
            }
        }
    }
    sphereVertexCount = data.size() / 8;
    if (sphereVbo.isCreated())
        sphereVbo.destroy();
    sphereVbo.create();
    sphereVbo.bind();
    sphereVbo.allocate(data.constData(), data.size() * int(sizeof(float)));
    sphereVbo.release();
}

void PlanetGLWidget::buildAccretionDisk()
{
    const int segs = 96;
    const float inner = 1.05f;
    const float outer = 3.2f;
    QVector<float> data;
    data.reserve(segs * 6 * 3);
    for (int i = 0; i < segs; ++i)
    {
        const float a0 = float(i) / float(segs) * float(2.0 * M_PI);
        const float a1 = float(i + 1) / float(segs) * float(2.0 * M_PI);
        const float c0 = qCos(a0);
        const float s0 = qSin(a0);
        const float c1 = qCos(a1);
        const float s1 = qSin(a1);
        const float p[6][2] = {
            {inner * c0, inner * s0}, {outer * c0, outer * s0}, {outer * c1, outer * s1},
            {inner * c0, inner * s0}, {outer * c1, outer * s1}, {inner * c1, inner * s1}
        };
        for (int k = 0; k < 6; ++k)
            data << p[k][0] << p[k][1] << 0.0f;
    }
    diskVertexCount = data.size() / 3;
    if (diskVbo.isCreated())
        diskVbo.destroy();
    diskVbo.create();
    diskVbo.bind();
    diskVbo.allocate(data.constData(), data.size() * int(sizeof(float)));
    diskVbo.release();
}

void PlanetGLWidget::rebuildRings()
{
    if (!ready)
        return;
    makeCurrent();
    ringVertexCount = 0;
    rockVertexCount = 0;
    QVector<float> ringData;
    QVector<float> rockData;
    if (planet && planet->s.is_ring)
    {
        if (planet->s.ring_material != 1)
        {
            const int segments = 160;
            for (const RingBand &band : planet->ring_bands)
            {
                if (band.empty)
                    continue;
                const QVector3D col(band.color.redF(), band.color.greenF(), band.color.blueF());
                for (int i = 0; i < segments; ++i)
                {
                    const float t0 = float(i) / segments * float(2.0 * M_PI);
                    const float t1 = float(i + 1) / segments * float(2.0 * M_PI);
                    const QVector3D i0(band.inner * qCos(t0), 0, band.inner * qSin(t0));
                    const QVector3D o0(band.outer * qCos(t0), 0, band.outer * qSin(t0));
                    const QVector3D i1(band.inner * qCos(t1), 0, band.inner * qSin(t1));
                    const QVector3D o1(band.outer * qCos(t1), 0, band.outer * qSin(t1));
                    const QVector3D pts[6] = {i0, o0, o1, i0, o1, i1};
                    for (int k = 0; k < 6; ++k)
                    {
                        ringData << pts[k].x() << pts[k].y() << pts[k].z();
                        ringData << 0.f << 1.f << 0.f;
                        ringData << col.x() << col.y() << col.z();
                    }
                }
            }
        }
        else
        {
            const IcoMesh &ico = rockIcoMesh();
            for (const RingRock &rock : planet->ring_rocks)
            {
                const QVector3D col(rock.color.redF(), rock.color.greenF(), rock.color.blueF());
                QVector<QVector3D> deformed;
                deformed.reserve(ico.v.size());
                for (const QVector3D &n : ico.v)
                    deformed.append(deformRockPoint(rock, n));
                for (int i = 0; i + 2 < ico.idx.size(); i += 3)
                {
                    const QVector3D &a = deformed[ico.idx[i]];
                    const QVector3D &b = deformed[ico.idx[i + 1]];
                    const QVector3D &c = deformed[ico.idx[i + 2]];
                    QVector3D nrm = QVector3D::crossProduct(b - a, c - a);
                    if (nrm.lengthSquared() > 1e-12f)
                        nrm.normalize();
                    const QVector3D pts[3] = {a, b, c};
                    for (const QVector3D &p : pts)
                    {
                        rockData << p.x() << p.y() << p.z();
                        rockData << nrm.x() << nrm.y() << nrm.z();
                        rockData << col.x() << col.y() << col.z();
                    }
                }
            }
        }
    }
    ringVertexCount = ringData.size() / 9;
    if (ringVbo.isCreated())
        ringVbo.destroy();
    if (!ringData.isEmpty())
    {
        ringVbo.create();
        ringVbo.bind();
        ringVbo.allocate(ringData.constData(), ringData.size() * int(sizeof(float)));
        ringVbo.release();
    }
    rockVertexCount = rockData.size() / 9;
    if (rockVbo.isCreated())
        rockVbo.destroy();
    if (!rockData.isEmpty())
    {
        rockVbo.create();
        rockVbo.bind();
        rockVbo.allocate(rockData.constData(), rockData.size() * int(sizeof(float)));
        rockVbo.release();
    }
}

void PlanetGLWidget::rebuildCities()
{
    if (!ready)
        return;
    makeCurrent();
    cityVertexCount = 0;
    QVector<float> data;
    if (planet && planet->s.is_civ)
    {
        data.reserve(planet->cities.size() * 3);
        for (const CityLight &c : planet->cities)
        {
            data << c.x << c.y << c.z;
        }
    }
    cityVertexCount = data.size() / 3;
    if (cityVbo.isCreated())
        cityVbo.destroy();
    if (!data.isEmpty())
    {
        cityVbo.create();
        cityVbo.bind();
        cityVbo.allocate(data.constData(), data.size() * int(sizeof(float)));
        cityVbo.release();
    }
}

void PlanetGLWidget::uploadTexture(std::unique_ptr<QOpenGLTexture> &tex, const QImage &img, bool repeatU)
{
    tex.reset();
    if (img.isNull())
        return;
    const QImage rgba = img.convertToFormat(QImage::Format_RGBA8888);
    tex.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
    tex->setFormat(QOpenGLTexture::RGBA8_UNorm);
    tex->setSize(rgba.width(), rgba.height());
    tex->setAutoMipMapGenerationEnabled(false);
    tex->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
    tex->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, rgba.constBits());
    tex->setMinificationFilter(QOpenGLTexture::Linear);
    tex->setMagnificationFilter(QOpenGLTexture::Linear);
    tex->setWrapMode(QOpenGLTexture::DirectionS, repeatU ? QOpenGLTexture::Repeat : QOpenGLTexture::ClampToEdge);
    tex->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
}

void PlanetGLWidget::refreshTextures()
{
    if (!planet || !ready)
        return;
    makeCurrent();
    uploadTexture(albedo, planet->img, true);
    uploadTexture(clouds, planet->img_clouds, true);
    rebuildCities();
    doneCurrent();
}

void PlanetGLWidget::refreshAppearance()
{
    if (ready)
        rebuildRings();
    update();
}

QVector3D PlanetGLWidget::poleAxis() const
{
    QVector3D pole(0, 1, 0);
    if (planet)
    {
        pole = QVector3D(float(planet->x_polar), float(planet->y_polar), float(planet->z_polar));
        if (pole.lengthSquared() < 1e-8f)
            pole = QVector3D(0, 1, 0);
        else
            pole.normalize();
        // Map row 0 is north = +Y; albedo is uploaded unflipped so shader v = 0.5 - lat/π.
    }
    return pole;
}

QMatrix4x4 PlanetGLWidget::spinAroundPole(float degrees) const
{
    QMatrix4x4 m;
    if (!qFuzzyIsNull(degrees))
        m.rotate(degrees, poleAxis());
    return m;
}

QMatrix4x4 PlanetGLWidget::ringBasis() const
{
    const QVector3D n = poleAxis();
    QVector3D helper(0.0f, 1.0f, 0.0f);
    if (qAbs(QVector3D::dotProduct(helper, n)) > 0.92f)
        helper = QVector3D(1.0f, 0.0f, 0.0f);
    const QVector3D x = QVector3D::crossProduct(helper, n).normalized();
    const QVector3D z = QVector3D::crossProduct(n, x).normalized();
    QMatrix4x4 m;
    m.setColumn(0, QVector4D(x, 0.0f));
    m.setColumn(1, QVector4D(n, 0.0f));
    m.setColumn(2, QVector4D(z, 0.0f));
    m.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
    return m;
}

void PlanetGLWidget::resizeGL(int, int)
{
}

void PlanetGLWidget::drawScene(const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
    const QMatrix4x4 planetModel = spinAroundPole(planetSpinDeg);
    const QMatrix4x4 mvp = proj * view * planetModel;
    QVector3D light(float(planet->x_shine), float(planet->y_shine), float(planet->z_shine));
    if (light.lengthSquared() < 1e-8f)
        light = QVector3D(0.3f, 0.2f, 1.0f);
    const float lightI = float(planet->s.visibleLight());
    const QVector3D lightCol = starLightRgb(planet->s.star_spectrum);

    drawStarAndSky(proj, view, light);

    planetProg.bind();
    planetProg.setUniformValue("uMvp", mvp);
    planetProg.setUniformValue("uModel", planetModel);
    planetProg.setUniformValue("uLight", light);
    planetProg.setUniformValue("uCam", cameraPos());
    planetProg.setUniformValue("uLightI", lightI);
    planetProg.setUniformValue("uLightColor", lightCol);
    planetProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
    if (albedo)
    {
        albedo->bind(0);
        planetProg.setUniformValue("uAlbedo", 0);
    }
    sphereVbo.bind();
    planetProg.enableAttributeArray(0);
    planetProg.enableAttributeArray(1);
    planetProg.enableAttributeArray(2);
    planetProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
    planetProg.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 8 * sizeof(float));
    planetProg.setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(float), 2, 8 * sizeof(float));
    glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
    sphereVbo.release();
    planetProg.release();

    if (cityVertexCount > 0 && cityVbo.isCreated() && planet->s.is_civ)
    {
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE 0x8861
#endif
#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE 0x8642
#endif
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        cityProg.bind();
        cityProg.setUniformValue("uMvp", mvp);
        cityProg.setUniformValue("uSize", 5.5f);
        cityProg.setUniformValue("uColor", QVector3D(planet->s.civ_color.redF(),
                                                     planet->s.civ_color.greenF(),
                                                     planet->s.civ_color.blueF()));
        cityVbo.bind();
        cityProg.enableAttributeArray(0);
        cityProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
        glDrawArrays(GL_POINTS, 0, cityVertexCount);
        cityVbo.release();
        cityProg.release();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDisable(GL_POINT_SPRITE);
        glDisable(GL_PROGRAM_POINT_SIZE);
    }

    if (planet->s.is_cloud && clouds)
    {
        QMatrix4x4 cloudModel = spinAroundPole(cloudSpinDeg);
        cloudModel.scale(1.02f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        cloudProg.bind();
        cloudProg.setUniformValue("uMvp", proj * view * cloudModel);
        cloudProg.setUniformValue("uModel", cloudModel);
        cloudProg.setUniformValue("uLight", light);
        cloudProg.setUniformValue("uCam", cameraPos());
        cloudProg.setUniformValue("uLightI", lightI);
        cloudProg.setUniformValue("uLightColor", lightCol);
        cloudProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
        cloudProg.setUniformValue("uCloudAlpha", float(qBound(0.0, 1.0 - 0.1 * planet->s.cloud_transparent, 1.0)));
        clouds->bind(0);
        cloudProg.setUniformValue("uClouds", 0);
        sphereVbo.bind();
        cloudProg.enableAttributeArray(0);
        cloudProg.enableAttributeArray(1);
        cloudProg.enableAttributeArray(2);
        cloudProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
        cloudProg.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 8 * sizeof(float));
        cloudProg.setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(float), 2, 8 * sizeof(float));
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
        sphereVbo.release();
        cloudProg.release();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    if (planet->s.is_atmo)
    {
        const float cover = float(qBound(0.0, 1.0 - 0.1 * planet->s.atmo_transparent, 1.0));
        const float sizeK = float(qBound(0.0, (planet->s.atmo_size - 1) / 9.0, 1.0));
        const float atmoScale = 1.0f + 0.03f * float(qBound(1, planet->s.atmo_size, 10));
        QMatrix4x4 atmoModel = planetModel;
        atmoModel.scale(atmoScale);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        atmoProg.bind();
        atmoProg.setUniformValue("uMvp", proj * view * atmoModel);
        atmoProg.setUniformValue("uModel", atmoModel);
        atmoProg.setUniformValue("uLight", light);
        atmoProg.setUniformValue("uCam", cameraPos());
        atmoProg.setUniformValue("uAtmoColor", QVector3D(planet->s.atmo_color.redF(),
                                                         planet->s.atmo_color.greenF(),
                                                         planet->s.atmo_color.blueF()));
        atmoProg.setUniformValue("uLightI", lightI);
        atmoProg.setUniformValue("uLightColor", lightCol);
        atmoProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
        atmoProg.setUniformValue("uAtmo", cover);
        atmoProg.setUniformValue("uAtmoSize", sizeK);
        sphereVbo.bind();
        atmoProg.enableAttributeArray(0);
        atmoProg.enableAttributeArray(1);
        atmoProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
        atmoProg.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 8 * sizeof(float));
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
        sphereVbo.release();
        atmoProg.release();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    if (planet->s.is_ring && (ringVertexCount > 0 || rockVertexCount > 0))
    {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        QMatrix4x4 ringModel = spinAroundPole(ringSpinDeg) * ringBasis();
        ringProg.bind();
        ringProg.setUniformValue("uMvp", proj * view * ringModel);
        ringProg.setUniformValue("uModel", ringModel);
        ringProg.setUniformValue("uLight", light);
        ringProg.setUniformValue("uCam", cameraPos());
        ringProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
        ringProg.setUniformValue("uPlanetR", 1.0f);
        auto drawLit = [this](QOpenGLBuffer &vbo, int count) {
            if (count <= 0 || !vbo.isCreated())
                return;
            vbo.bind();
            ringProg.enableAttributeArray(0);
            ringProg.enableAttributeArray(1);
            ringProg.enableAttributeArray(2);
            ringProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 9 * sizeof(float));
            ringProg.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 9 * sizeof(float));
            ringProg.setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(float), 3, 9 * sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, count);
            vbo.release();
        };
        if (planet->s.ring_material == 1)
        {
            ringProg.setUniformValue("uTwoSided", 0.0f);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            drawLit(rockVbo, rockVertexCount);
        }
        else
        {
            ringProg.setUniformValue("uTwoSided", 1.0f);
            drawLit(ringVbo, ringVertexCount);
        }
        ringProg.release();
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    if (planet->s.has_star && sphereVertexCount > 0 && sphereVbo.isCreated())
    {
        QVector3D dir = light.normalized();
        const float radius = 0.04f + 0.38f * (qBound(0, planet->s.star_size, 5) / 5.0f);
        QMatrix4x4 starModel;
        starModel.translate(dir * 6.0f);
        starModel.scale(radius);
        const bool blackHole = isStarBlackHole(planet->s.star_spectrum);
        if (blackHole)
        {
            QVector3D up(0.0f, 1.0f, 0.0f);
            if (qAbs(QVector3D::dotProduct(up, dir)) > 0.92f)
                up = QVector3D(1.0f, 0.0f, 0.0f);
            const QVector3D right = QVector3D::crossProduct(up, dir).normalized();
            const QVector3D diskN = QQuaternion::fromAxisAndAngle(right, 65.0f).rotatedVector(dir);
            QMatrix4x4 diskModel;
            diskModel.translate(dir * 6.0f);
            diskModel.rotate(QQuaternion::rotationTo(QVector3D(0.0f, 0.0f, 1.0f), diskN));
            diskModel.scale(radius);

            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
            starProg.bind();
            starProg.setUniformValue("uMvp", proj * view * starModel);
            starProg.setUniformValue("uColor", QVector3D(0.0f, 0.0f, 0.0f));
            starProg.setUniformValue("uAlpha", 1.0f);
            sphereVbo.bind();
            starProg.enableAttributeArray(0);
            starProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
            sphereVbo.release();
            starProg.release();

            if (diskVertexCount > 0 && diskVbo.isCreated())
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);
                diskProg.bind();
                diskProg.setUniformValue("uMvp", proj * view * diskModel);
                diskProg.setUniformValue("uTime", ringSpinDeg * float(M_PI / 180.0));
                diskVbo.bind();
                diskProg.enableAttributeArray(0);
                diskProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
                glDrawArrays(GL_TRIANGLES, 0, diskVertexCount);
                diskVbo.release();
                diskProg.release();
                glEnable(GL_CULL_FACE);
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
            }
        }
        else
        {
            QMatrix4x4 haloModel = starModel;
            haloModel.scale(1.85f);
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE 0x8861
#endif
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            starProg.bind();
            starProg.setUniformValue("uMvp", proj * view * haloModel);
            starProg.setUniformValue("uColor", lightCol);
            starProg.setUniformValue("uAlpha", 0.35f);
            sphereVbo.bind();
            starProg.enableAttributeArray(0);
            starProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            starProg.setUniformValue("uMvp", proj * view * starModel);
            starProg.setUniformValue("uAlpha", 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
            sphereVbo.release();
            starProg.release();
        }
    }
}

void PlanetGLWidget::rebuildStarfield()
{
    starfieldCount = 0;
    if (!ready)
        return;
    makeCurrent();
    const quint32 seed = planet ? quint32(planet->seed) : 1u;
    QRandomGenerator rng(seed ^ 0x5A14u);
    const int n = 900;
    QVector<float> data;
    data.reserve(n * 3);
    for (int i = 0; i < n; ++i)
    {
        QVector3D p(float(rng.generateDouble() * 2.0 - 1.0),
                    float(rng.generateDouble() * 2.0 - 1.0),
                    float(rng.generateDouble() * 2.0 - 1.0));
        if (p.lengthSquared() < 1e-6f)
            p = QVector3D(0, 1, 0);
        p.normalize();
        p *= 16.0f;
        data << p.x() << p.y() << p.z();
    }
    starfieldCount = n;
    if (starfieldVbo.isCreated())
        starfieldVbo.destroy();
    starfieldVbo.create();
    starfieldVbo.bind();
    starfieldVbo.allocate(data.constData(), data.size() * int(sizeof(float)));
    starfieldVbo.release();
}

void PlanetGLWidget::drawStarAndSky(const QMatrix4x4 &proj, const QMatrix4x4 &view, const QVector3D &)
{
    if (!planet || !planet->s.is_starfield || starfieldCount <= 0 || !starfieldVbo.isCreated())
        return;
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE 0x8861
#endif
#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE 0x8642
#endif
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    starfieldProg.bind();
    starfieldProg.setUniformValue("uMvp", proj * view);
    starfieldVbo.bind();
    starfieldProg.enableAttributeArray(0);
    starfieldProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    glDrawArrays(GL_POINTS, 0, starfieldCount);
    starfieldVbo.release();
    starfieldProg.release();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void PlanetGLWidget::paintGL()
{
    if (!planet || !albedo)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    const int dpr = qMax(1, int(devicePixelRatio()));
    const int pixelW = qMax(1, width() * dpr);
    const int pixelH = qMax(1, height() * dpr);
    const int maxSide = qBound(16, currentViewRes(), 1024);
    const int maxDim = qMax(pixelW, pixelH);
    const float scale = (maxDim > maxSide) ? (float(maxSide) / float(maxDim)) : 1.0f;
    const int fboW = qMax(16, qRound(pixelW * scale));
    const int fboH = qMax(16, qRound(pixelH * scale));
    ensureSceneFbo(fboW, fboH);

    QMatrix4x4 proj;
    const float aspect = float(qMax(1, width())) / float(qMax(1, height()));
    proj.perspective(kFovDeg, aspect, 0.1f, cameraDistance + 32.0f);
    QMatrix4x4 view;
    view.lookAt(cameraPos(), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

    sceneFbo->bind();
    glViewport(0, 0, fboW, fboH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawScene(proj, view);
    sceneFbo->release();

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, pixelW, pixelH);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT);

    blitProg.bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneFbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    blitProg.setUniformValue("uTex", 0);
    blitVbo.bind();
    blitProg.enableAttributeArray(0);
    blitProg.enableAttributeArray(1);
    blitProg.setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(float));
    blitProg.setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 4 * sizeof(float));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    blitVbo.release();
    blitProg.release();
}

namespace {
QImage cropCenterSquare(const QImage &img)
{
    if (img.isNull())
        return img;
    const int side = qMin(img.width(), img.height());
    if (side <= 0)
        return img;
    const int x = (img.width() - side) / 2;
    const int y = (img.height() - side) / 2;
    return img.copy(x, y, side, side);
}
}

QImage PlanetGLWidget::captureView()
{
    QImage shot = grabFramebuffer();
    if (shot.isNull() && sceneFbo && sceneFbo->isValid())
    {
        makeCurrent();
        shot = sceneFbo->toImage(true);
    }
    return cropCenterSquare(shot);
}

float PlanetGLWidget::azimuthAngle() const
{
    return azimuth;
}

float PlanetGLWidget::elevationAngle() const
{
    return elevation;
}

float PlanetGLWidget::zoomPercent() const
{
    const float dist = qMax(0.01f, cameraDistance);
    return 100.0f * defaultCameraDistance() / dist;
}

QJsonObject PlanetGLWidget::viewToJson() const
{
    QJsonObject view;
    view.insert(QStringLiteral("azimuth"), double(azimuth));
    view.insert(QStringLiteral("elevation"), double(elevation));
    view.insert(QStringLiteral("distance"), double(cameraDistance));
    view.insert(QStringLiteral("planetSpin"), double(planetSpinDeg));
    view.insert(QStringLiteral("cloudSpin"), double(cloudSpinDeg));
    view.insert(QStringLiteral("ringSpin"), double(ringSpinDeg));
    return view;
}

void PlanetGLWidget::applyViewJson(const QJsonObject &view)
{
    stopCameraReset();
    if (view.contains(QStringLiteral("azimuth")))
        azimuth = wrapDeg(float(view.value(QStringLiteral("azimuth")).toDouble()));
    if (view.contains(QStringLiteral("elevation")))
        elevation = qBound(-89.0f, float(view.value(QStringLiteral("elevation")).toDouble()), 89.0f);
    if (view.contains(QStringLiteral("distance")))
        cameraDistance = qBound(1.25f, float(view.value(QStringLiteral("distance")).toDouble()), 12.0f);
    if (view.contains(QStringLiteral("planetSpin")))
        planetSpinDeg = wrapDeg(float(view.value(QStringLiteral("planetSpin")).toDouble()));
    if (view.contains(QStringLiteral("cloudSpin")))
        cloudSpinDeg = wrapDeg(float(view.value(QStringLiteral("cloudSpin")).toDouble()));
    if (view.contains(QStringLiteral("ringSpin")))
        ringSpinDeg = wrapDeg(float(view.value(QStringLiteral("ringSpin")).toDouble()));
    update();
    emit cameraChanged();
}

void PlanetGLWidget::mousePressEvent(QMouseEvent *event)
{
    stopCameraReset();
    dragging = true;
    lastPos = event->pos();
}

void PlanetGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging)
        return;
    const QPoint d = event->pos() - lastPos;
    lastPos = event->pos();
    azimuth -= d.x() * 0.4f;
    elevation += d.y() * 0.4f;
    elevation = qBound(-89.0f, elevation, 89.0f);
    update();
    emit cameraChanged();
}

void PlanetGLWidget::mouseReleaseEvent(QMouseEvent *)
{
    dragging = false;
}

void PlanetGLWidget::wheelEvent(QWheelEvent *event)
{
    const float steps = float(event->angleDelta().y()) / 120.0f;
    if (qFuzzyIsNull(steps))
        return;
    stopCameraReset();
    cameraDistance *= qPow(0.9f, steps);
    cameraDistance = qBound(1.25f, cameraDistance, 12.0f);
    update();
    emit cameraChanged();
    event->accept();
}
