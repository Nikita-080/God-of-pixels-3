#ifndef PLANETGLWIDGET_H
#define PLANETGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QVector3D>
#include <QElapsedTimer>
#include <QWheelEvent>
#include <memory>

class QTimer;

class Planet;

class PlanetGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit PlanetGLWidget(QWidget *parent = nullptr);
    ~PlanetGLWidget() override;

    void setPlanet(const Planet *planet);
    void refreshTextures();
    void refreshAppearance();
    void resetCamera();
    QImage captureView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void buildSphere(int slices, int stacks);
    void rebuildRings();
    void rebuildCities();
    void buildBlitQuad();
    void ensureSceneFbo(int res);
    void uploadTexture(std::unique_ptr<QOpenGLTexture> &tex, const QImage &img, bool repeatU);
    void drawScene(const QMatrix4x4 &proj, const QMatrix4x4 &view, const QMatrix4x4 &model);
    QMatrix4x4 ringBasis() const;
    QVector3D cameraPos() const;
    int currentViewRes() const;
    static float defaultCameraDistance();
    void stopCameraReset();
    void tickCameraReset();
    void rebuildStarfield();
    void drawStarAndSky(const QMatrix4x4 &proj, const QMatrix4x4 &view, const QVector3D &lightDir);

    const Planet *planet;
    QOpenGLShaderProgram planetProg;
    QOpenGLShaderProgram cloudProg;
    QOpenGLShaderProgram atmoProg;
    QOpenGLShaderProgram ringProg;
    QOpenGLShaderProgram blitProg;
    QOpenGLShaderProgram cityProg;
    QOpenGLShaderProgram starProg;
    QOpenGLShaderProgram starfieldProg;
    QOpenGLBuffer sphereVbo;
    QOpenGLBuffer ringVbo;
    QOpenGLBuffer rockVbo;
    QOpenGLBuffer blitVbo;
    QOpenGLBuffer cityVbo;
    QOpenGLBuffer starfieldVbo;
    std::unique_ptr<QOpenGLFramebufferObject> sceneFbo;
    int sphereVertexCount;
    int ringVertexCount;
    int rockVertexCount;
    int cityVertexCount;
    int starfieldCount;
    std::unique_ptr<QOpenGLTexture> albedo;
    std::unique_ptr<QOpenGLTexture> clouds;
    float azimuth;
    float elevation;
    float cameraDistance;
    QTimer *camResetTimer;
    QElapsedTimer camResetClock;
    float camFromAz;
    float camFromEl;
    float camFromDist;
    float camDeltaAz;
    bool dragging;
    QPoint lastPos;
    bool ready;
};

#endif
