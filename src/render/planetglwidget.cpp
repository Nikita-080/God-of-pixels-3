#include "planetglwidget.h"
#include "planet.h"
#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QQuaternion>
#include <QtMath>

namespace {
const float kFovDeg = 42.0f;
const float kDefaultFill = 0.6f;
const float kDefaultAzimuth = 0.0f;
const float kDefaultElevation = 18.0f;
const int kCameraResetMs = 1300;
}

static const char *kPlanetVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "attribute vec3 aNormal;\n"
    "attribute vec2 aUv;\n"
    "uniform mat4 uMvp;\n"
    "uniform mat4 uModel;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec2 vUv;\n"
    "void main() {\n"
    "  vec4 wp = uModel * vec4(aPos, 1.0);\n"
    "  vWorld = wp.xyz;\n"
    "  vNormal = mat3(uModel) * aNormal;\n"
    "  vUv = aUv;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kPlanetFrag =
    "#version 120\n"
    "uniform sampler2D uAlbedo;\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform float uShine;\n"
    "uniform float uFillLight;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec2 vUv;\n"
    "void main() {\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  vec3 viewDir = normalize(uCam - vWorld);\n"
    "  float mainI = uShine * 0.1 + 0.5;\n"
    "  float fillI = 0.3 * 0.5;\n"
    "  float lit = max(dot(n, l), 0.0) * mainI;\n"
    "  if (uFillLight > 0.5)\n"
    "    lit += max(dot(n, viewDir), 0.0) * fillI;\n"
    "  vec3 albedo = texture2D(uAlbedo, vUv).rgb;\n"
    "  vec3 col = albedo * lit;\n"
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
    "varying vec2 vUv;\n"
    "void main() {\n"
    "  vec4 wp = uModel * vec4(aPos, 1.0);\n"
    "  vWorld = wp.xyz;\n"
    "  vNormal = mat3(uModel) * aNormal;\n"
    "  vUv = aUv;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kCloudFrag =
    "#version 120\n"
    "uniform sampler2D uClouds;\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform float uShine;\n"
    "uniform float uFillLight;\n"
    "uniform float uCloudAlpha;\n"
    "varying vec3 vWorld;\n"
    "varying vec3 vNormal;\n"
    "varying vec2 vUv;\n"
    "void main() {\n"
    "  vec4 c = texture2D(uClouds, vUv);\n"
    "  float a = c.a * uCloudAlpha;\n"
    "  if (a < 0.02) discard;\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  vec3 viewDir = normalize(uCam - vWorld);\n"
    "  float wrap = max(dot(n, l) * 0.5 + 0.5, 0.0);\n"
    "  float lit = wrap * (uShine * 0.08 + 0.55);\n"
    "  if (uFillLight > 0.5)\n"
    "    lit += max(dot(n, viewDir), 0.0) * 0.2;\n"
    "  gl_FragColor = vec4(c.rgb * lit, a);\n"
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
    "uniform float uShine;\n"
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
    "  float limb = pow(1.0 - ndv, mix(4.8, 0.35, uAtmoSize));\n"
    "  float haze = mix(limb, 1.0, uAtmo) * uAtmo;\n"
    "  float mainI = uShine * 0.1 + 0.5;\n"
    "  float lit = max(dot(n, l), 0.0) * mainI;\n"
    "  if (uFillLight > 0.5)\n"
    "    lit += max(dot(n, viewDir), 0.0) * 0.15;\n"
    "  vec3 col = uAtmoColor * (0.4 + 0.6 * lit);\n"
    "  gl_FragColor = vec4(col, haze);\n"
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
    "varying vec3 vPos;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "  vec3 n = normalize(vNormal);\n"
    "  vec3 l = normalize(uLight);\n"
    "  float ndl = dot(n, l);\n"
    "  if (uTwoSided > 0.5) ndl = abs(ndl);\n"
    "  float shade = 0.45 + 0.55 * max(ndl, 0.0);\n"
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

PlanetGLWidget::PlanetGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , planet(nullptr)
    , sphereVbo(QOpenGLBuffer::VertexBuffer)
    , ringVbo(QOpenGLBuffer::VertexBuffer)
    , rockVbo(QOpenGLBuffer::VertexBuffer)
    , blitVbo(QOpenGLBuffer::VertexBuffer)
    , cityVbo(QOpenGLBuffer::VertexBuffer)
    , sphereVertexCount(0)
    , ringVertexCount(0)
    , rockVertexCount(0)
    , cityVertexCount(0)
    , azimuth(kDefaultAzimuth)
    , elevation(kDefaultElevation)
    , cameraDistance(defaultCameraDistance())
    , camResetTimer(new QTimer(this))
    , camFromAz(kDefaultAzimuth)
    , camFromEl(kDefaultElevation)
    , camFromDist(defaultCameraDistance())
    , camDeltaAz(0.0f)
    , dragging(false)
    , ready(false)
{
    setMinimumSize(257, 257);
    setFocusPolicy(Qt::WheelFocus);
    camResetTimer->setInterval(16);
    connect(camResetTimer, &QTimer::timeout, this, &PlanetGLWidget::tickCameraReset);
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
    if (t >= 1.0f)
    {
        azimuth = kDefaultAzimuth;
        elevation = kDefaultElevation;
        cameraDistance = defaultCameraDistance();
        stopCameraReset();
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
        return;
    }
    camResetClock.start();
    tickCameraReset();
    camResetTimer->start();
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
    planetProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kPlanetFrag);
    planetProg.bindAttributeLocation("aPos", 0);
    planetProg.bindAttributeLocation("aNormal", 1);
    planetProg.bindAttributeLocation("aUv", 2);
    planetProg.link();

    cloudProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kCloudVert);
    cloudProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kCloudFrag);
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

    buildSphere(96, 64);
    rebuildRings();
    buildBlitQuad();
    ready = true;
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

void PlanetGLWidget::ensureSceneFbo(int res)
{
    res = qBound(16, res, 1024);
    if (sceneFbo && sceneFbo->width() == res && sceneFbo->height() == res)
        return;
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::Depth);
    sceneFbo.reset(new QOpenGLFramebufferObject(res, res, fmt));
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
            const int slices = 10;
            const int stacks = 6;
            for (const RingRock &rock : planet->ring_rocks)
            {
                const QVector3D col(rock.color.redF(), rock.color.greenF(), rock.color.blueF());
                const QVector3D c(rock.x, rock.y, rock.z);
                for (int y = 0; y < stacks; ++y)
                {
                    const float phi0 = float(y) / stacks * float(M_PI);
                    const float phi1 = float(y + 1) / stacks * float(M_PI);
                    for (int x = 0; x < slices; ++x)
                    {
                        const float th0 = float(x) / slices * float(2.0 * M_PI);
                        const float th1 = float(x + 1) / slices * float(2.0 * M_PI);
                        const QVector3D n00(qSin(phi0) * qCos(th0), qCos(phi0), qSin(phi0) * qSin(th0));
                        const QVector3D n10(qSin(phi0) * qCos(th1), qCos(phi0), qSin(phi0) * qSin(th1));
                        const QVector3D n01(qSin(phi1) * qCos(th0), qCos(phi1), qSin(phi1) * qSin(th0));
                        const QVector3D n11(qSin(phi1) * qCos(th1), qCos(phi1), qSin(phi1) * qSin(th1));
                        const QVector3D nrm[6] = {n00, n10, n11, n00, n11, n01};
                        for (int k = 0; k < 6; ++k)
                        {
                            const QVector3D p = c + nrm[k] * rock.radius;
                            rockData << p.x() << p.y() << p.z();
                            rockData << nrm[k].x() << nrm[k].y() << nrm[k].z();
                            rockData << col.x() << col.y() << col.z();
                        }
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
    tex.reset(new QOpenGLTexture(img.convertToFormat(QImage::Format_RGBA8888),
                                QOpenGLTexture::DontGenerateMipMaps));
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

QMatrix4x4 PlanetGLWidget::ringBasis() const
{
    QMatrix4x4 m;
    if (!planet)
        return m;
    QVector3D pole(float(planet->x_polar), float(planet->y_polar), float(planet->z_polar));
    if (pole.lengthSquared() < 1e-8f)
        pole = QVector3D(0, 1, 0);
    else
        pole.normalize();
    // QOpenGLTexture mirrors QImage vertically, so map north lands on mesh -Y.
    pole.setY(-pole.y());
    m.rotate(QQuaternion::rotationTo(QVector3D(0, 1, 0), pole));
    return m;
}

void PlanetGLWidget::resizeGL(int, int)
{
}

void PlanetGLWidget::drawScene(const QMatrix4x4 &proj, const QMatrix4x4 &view, const QMatrix4x4 &model)
{
    const QMatrix4x4 mvp = proj * view * model;
    QVector3D light(float(planet->x_shine), float(planet->y_shine), float(planet->z_shine));
    if (light.lengthSquared() < 1e-8f)
        light = QVector3D(0.3f, 0.2f, 1.0f);

    planetProg.bind();
    planetProg.setUniformValue("uMvp", mvp);
    planetProg.setUniformValue("uModel", model);
    planetProg.setUniformValue("uLight", light);
    planetProg.setUniformValue("uCam", cameraPos());
    planetProg.setUniformValue("uShine", float(planet->s.shine));
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
        QMatrix4x4 cloudModel = model;
        cloudModel.scale(1.02f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        cloudProg.bind();
        cloudProg.setUniformValue("uMvp", proj * view * cloudModel);
        cloudProg.setUniformValue("uModel", cloudModel);
        cloudProg.setUniformValue("uLight", light);
        cloudProg.setUniformValue("uCam", cameraPos());
        cloudProg.setUniformValue("uShine", float(planet->s.shine));
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
        QMatrix4x4 atmoModel = model;
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
        atmoProg.setUniformValue("uShine", float(planet->s.shine));
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
        QMatrix4x4 ringModel = model * ringBasis();
        ringProg.bind();
        ringProg.setUniformValue("uMvp", proj * view * ringModel);
        ringProg.setUniformValue("uModel", ringModel);
        ringProg.setUniformValue("uLight", light);
        ringProg.setUniformValue("uCam", cameraPos());
        ringProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
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
}

void PlanetGLWidget::paintGL()
{
    if (!planet || !albedo)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    const int res = currentViewRes();
    ensureSceneFbo(res);

    QMatrix4x4 proj;
    proj.perspective(kFovDeg, 1.0f, 0.1f, qMax(20.0f, cameraDistance + 8.0f));
    QMatrix4x4 view;
    view.lookAt(cameraPos(), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    QMatrix4x4 model;

    sceneFbo->bind();
    glViewport(0, 0, res, res);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawScene(proj, view, model);
    sceneFbo->release();

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    const int dpr = qMax(1, int(devicePixelRatio()));
    glViewport(0, 0, width() * dpr, height() * dpr);
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

QImage PlanetGLWidget::captureView()
{
    if (!sceneFbo || !sceneFbo->isValid())
        return grabFramebuffer();
    const QImage raw = sceneFbo->toImage();
    if (raw.isNull())
        return grabFramebuffer();
    const int side = qMax(width(), 1);
    return raw.scaled(side, side, Qt::IgnoreAspectRatio, Qt::FastTransformation);
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
    event->accept();
}
