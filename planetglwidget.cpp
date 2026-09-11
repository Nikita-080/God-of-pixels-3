#include "planetglwidget.h"
#include "planet.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QQuaternion>
#include <QtMath>

namespace {
const float kFovDeg = 42.0f;
const float kDefaultFill = 0.6f;
const float kDefaultAzimuth = 0.0f;
const float kDefaultElevation = 18.0f;
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
    "uniform sampler2D uClouds;\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform vec3 uAtmoColor;\n"
    "uniform float uShine;\n"
    "uniform float uFillLight;\n"
    "uniform float uHasClouds;\n"
    "uniform float uCloudAlpha;\n"
    "uniform float uHasAtmo;\n"
    "uniform float uAtmo;\n"
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
    "  if (uHasClouds > 0.5) {\n"
    "    vec4 c = texture2D(uClouds, vUv);\n"
    "    float k = c.a * uCloudAlpha;\n"
    "    albedo = mix(albedo, c.rgb, k);\n"
    "  }\n"
    "  vec3 col = albedo * lit;\n"
    "  if (uHasAtmo > 0.5) {\n"
    "    float fres = pow(1.0 - abs(dot(n, viewDir)), 2.2);\n"
    "    col = mix(col, uAtmoColor, fres * uAtmo);\n"
    "  }\n"
    "  gl_FragColor = vec4(col, 1.0);\n"
    "}\n";

static const char *kRingVert =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "attribute float aBand;\n"
    "uniform mat4 uMvp;\n"
    "uniform mat4 uModel;\n"
    "varying float vBand;\n"
    "varying vec3 vPos;\n"
    "void main() {\n"
    "  vBand = aBand;\n"
    "  vPos = (uModel * vec4(aPos, 1.0)).xyz;\n"
    "  gl_Position = uMvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *kRingFrag =
    "#version 120\n"
    "uniform vec3 uLight;\n"
    "uniform vec3 uCam;\n"
    "uniform float uFillLight;\n"
    "uniform vec3 uColor0;\n"
    "uniform vec3 uColor1;\n"
    "varying float vBand;\n"
    "varying vec3 vPos;\n"
    "void main() {\n"
    "  vec3 l = normalize(uLight);\n"
    "  vec3 n = normalize(vPos);\n"
    "  float shade = 0.45 + 0.55 * max(dot(n, l), 0.0);\n"
    "  if (uFillLight > 0.5)\n"
    "    shade += 0.55 * 0.3 * 0.5 * max(dot(n, normalize(uCam)), 0.0);\n"
    "  vec3 col = mix(uColor0, uColor1, vBand) * shade;\n"
    "  gl_FragColor = vec4(col, 0.92);\n"
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

PlanetGLWidget::PlanetGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , planet(nullptr)
    , sphereVbo(QOpenGLBuffer::VertexBuffer)
    , ringVbo(QOpenGLBuffer::VertexBuffer)
    , blitVbo(QOpenGLBuffer::VertexBuffer)
    , sphereVertexCount(0)
    , ringVertexCount(0)
    , azimuth(kDefaultAzimuth)
    , elevation(kDefaultElevation)
    , cameraDistance(defaultCameraDistance())
    , dragging(false)
    , ready(false)
{
    setMinimumSize(257, 257);
    setFocusPolicy(Qt::WheelFocus);
}

PlanetGLWidget::~PlanetGLWidget()
{
    makeCurrent();
    sceneFbo.reset();
    sphereVbo.destroy();
    ringVbo.destroy();
    blitVbo.destroy();
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

void PlanetGLWidget::resetCamera()
{
    azimuth = kDefaultAzimuth;
    elevation = kDefaultElevation;
    cameraDistance = defaultCameraDistance();
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
    planetProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kPlanetFrag);
    planetProg.bindAttributeLocation("aPos", 0);
    planetProg.bindAttributeLocation("aNormal", 1);
    planetProg.bindAttributeLocation("aUv", 2);
    planetProg.link();

    ringProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kRingVert);
    ringProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kRingFrag);
    ringProg.bindAttributeLocation("aPos", 0);
    ringProg.bindAttributeLocation("aBand", 1);
    ringProg.link();

    blitProg.addShaderFromSourceCode(QOpenGLShader::Vertex, kBlitVert);
    blitProg.addShaderFromSourceCode(QOpenGLShader::Fragment, kBlitFrag);
    blitProg.bindAttributeLocation("aPos", 0);
    blitProg.bindAttributeLocation("aUv", 1);
    blitProg.link();

    buildSphere(96, 64);
    buildRings(160);
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

void PlanetGLWidget::buildRings(int segments)
{
    QVector<float> data;
    const float inner = 1.25f;
    const float outer = 2.05f;
    for (int i = 0; i < segments; ++i)
    {
        const float t0 = float(i) / segments * float(2.0 * M_PI);
        const float t1 = float(i + 1) / segments * float(2.0 * M_PI);
        const QVector3D i0(inner * qCos(t0), 0, inner * qSin(t0));
        const QVector3D o0(outer * qCos(t0), 0, outer * qSin(t0));
        const QVector3D i1(inner * qCos(t1), 0, inner * qSin(t1));
        const QVector3D o1(outer * qCos(t1), 0, outer * qSin(t1));
        const QVector3D pts[6] = {i0, o0, o1, i0, o1, i1};
        const float band[6] = {0, 1, 1, 0, 1, 0};
        for (int k = 0; k < 6; ++k)
        {
            data << pts[k].x() << pts[k].y() << pts[k].z() << band[k];
        }
    }
    ringVertexCount = data.size() / 4;
    if (ringVbo.isCreated())
        ringVbo.destroy();
    ringVbo.create();
    ringVbo.bind();
    ringVbo.allocate(data.constData(), data.size() * int(sizeof(float)));
    ringVbo.release();
}

void PlanetGLWidget::uploadTexture(std::unique_ptr<QOpenGLTexture> &tex, const QImage &img, bool repeatU)
{
    tex.reset();
    if (img.isNull())
        return;
    tex.reset(new QOpenGLTexture(img, QOpenGLTexture::DontGenerateMipMaps));
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
    doneCurrent();
}

void PlanetGLWidget::refreshAppearance()
{
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
    planetProg.setUniformValue("uAtmoColor", QVector3D(planet->s.atmo_color.redF(),
                                                       planet->s.atmo_color.greenF(),
                                                       planet->s.atmo_color.blueF()));
    planetProg.setUniformValue("uShine", float(planet->s.shine));
    planetProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
    planetProg.setUniformValue("uHasClouds", planet->s.is_cloud ? 1.0f : 0.0f);
    planetProg.setUniformValue("uCloudAlpha", float(qBound(0.0, 1.0 - 0.09 * planet->s.cloud_transparent, 1.0)));
    planetProg.setUniformValue("uHasAtmo", planet->s.is_atmo ? 1.0f : 0.0f);
    planetProg.setUniformValue("uAtmo", float(qBound(0.0, 1.0 - 0.1 * planet->s.atmo_transparent, 1.0)));
    if (albedo)
    {
        albedo->bind(0);
        planetProg.setUniformValue("uAlbedo", 0);
    }
    if (clouds)
    {
        clouds->bind(1);
        planetProg.setUniformValue("uClouds", 1);
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

    if (planet->s.is_ring)
    {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        QMatrix4x4 ringModel = model * ringBasis();
        float scale = float((planet->ring_inner + planet->ring_outer) * 0.5 / 1.65);
        ringModel.scale(scale);
        ringProg.bind();
        ringProg.setUniformValue("uMvp", proj * view * ringModel);
        ringProg.setUniformValue("uModel", ringModel);
        ringProg.setUniformValue("uLight", light);
        ringProg.setUniformValue("uCam", cameraPos());
        ringProg.setUniformValue("uFillLight", planet->s.is_fill_light ? 1.0f : 0.0f);
        QColor c0 = planet->ring_colors.isEmpty() ? planet->s.ring_color : planet->ring_colors.first();
        QColor c1 = planet->ring_colors.size() > 1 ? planet->ring_colors.last() : c0.darker(130);
        ringProg.setUniformValue("uColor0", QVector3D(c0.redF(), c0.greenF(), c0.blueF()));
        ringProg.setUniformValue("uColor1", QVector3D(c1.redF(), c1.greenF(), c1.blueF()));
        ringVbo.bind();
        ringProg.enableAttributeArray(0);
        ringProg.enableAttributeArray(1);
        ringProg.setAttributeBuffer(0, GL_FLOAT, 0, 3, 4 * sizeof(float));
        ringProg.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 1, 4 * sizeof(float));
        glDrawArrays(GL_TRIANGLES, 0, ringVertexCount);
        ringVbo.release();
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
    cameraDistance *= qPow(0.9f, steps);
    cameraDistance = qBound(1.25f, cameraDistance, 12.0f);
    update();
    event->accept();
}
