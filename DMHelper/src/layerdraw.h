#ifndef LAYERDRAW_H
#define LAYERDRAW_H

#include "layer.h"
#include "layerdrawstate.h"
#include <QImage>
#include <QPainterPath>
#include <QHash>

class PublishGLBattleBackground;
class QGraphicsItem;
class QGraphicsPathItem;
class QPainter;
class QUndoStack;

class LayerDraw : public Layer
{
    Q_OBJECT
public:
    explicit LayerDraw(const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerDraw() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;
    //virtual void copyBaseValues(Layer *other) const;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    QImage getImage() const;
    LayerDrawState& getDrawState();

    QGraphicsItem* createGraphicsItem(LayerDrawObject* drawObject);

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;

//    QPainterPath* beginPainting();
//    void endPainting();

    void addObject(LayerDrawObject* drawObject);
    void removeObject(LayerDrawObject* drawObject);

protected slots:
    void handleObjectAdded(LayerDrawObject* object, int index);
    void handleObjectRemoved(LayerDrawObject* object, int index);

protected:
    // QObject overrides

    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    void createShaders();
    void destroyShaders();
    void createObjects();
    void destroyObjects();

    // Generic Methods

    // DM Window Members
    QHash<LayerDrawObject*, QGraphicsItem*> _graphicsItems;
    //QGraphicsPixmapItem* _graphicsItem;
    //QGraphicsPathItem* _pathItem;

    // Player Window Members
    PublishGLBattleBackground* _drawGLObject;
    PublishGLScene* _scene;

    // Core contents
    LayerDrawState _layerDrawState;
    //QPainterPath _drawPath;
    //QImage _imageLayer;
    //QPainter* _imagePainter;

    QUndoStack* _undoStack;
    //QList<UndoFowBase*> _undoItems;

};

#endif // LAYERDRAW_H
