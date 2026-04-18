#ifndef LAYERWALLS_H
#define LAYERWALLS_H

#include "layer.h"
#include "visioncalculator.h"
#include <QColor>
#include <QLineF>
#include <QList>

class QGraphicsItemGroup;
class QGraphicsLineItem;

/*
 * LayerWalls - stores and renders vision-blocking wall segments.
 *
 * Walls are kept as a list of QLineF segments in scene coordinates. This
 * layer serves two roles:
 *   1. DM-side editing surface: walls are drawn as thin colored lines on
 *      top of the map so the DM can see and edit them. Not published to
 *      the player view.
 *   2. Geometry source: VisionCalculator consumes walls() to produce
 *      per-token vision polygons, which a companion layer (or the fog-of-
 *      war layer) uses to reveal only what each token can see.
 *
 * Intentionally minimal for this first pass: no per-wall door/window flags
 * yet, no OpenGL player rendering (walls are DM-only), no drawing tool UI.
 * Those layer on top once the data model and geometry engine are stable.
 */
class LayerWalls : public Layer
{
    Q_OBJECT
public:
    explicit LayerWalls(const QString& name = QString(), int order = 0, QObject* parent = nullptr);
    virtual ~LayerWalls() override;

    virtual void inputXML(const QDomElement& element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual bool hasSettings() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    // Wall data access
    const QList<QLineF>& walls() const;
    void addWall(const QLineF& segment);
    void addWalls(const QList<QLineF>& segments);
    void removeWall(int index);
    void clearWalls();
    int wallCount() const;

    QColor wallColor() const;
    void setWallColor(const QColor& color);

    qreal wallThickness() const;
    void setWallThickness(qreal thickness);

    // Compute the visible polygon for a viewpoint (delegates to VisionCalculator).
    // bounds defaults to boundingRect() if empty.
    QPolygonF computeVisionPolygon(const QPointF& viewpoint,
                                   qreal maxRadius = 0.0,
                                   const QRectF& bounds = QRectF()) const;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface - walls are editor-only; stubs below.
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;
    virtual void editSettings() override;

signals:
    void wallsChanged();

protected:
    virtual void internalOutputXML(QDomDocument& doc, QDomElement& element, QDir& targetDirectory, bool isExport) override;

    void rebuildGraphicsItems();
    void cleanupDM();

    QList<QLineF> _walls;
    QColor _wallColor;
    qreal _wallThickness;

    // DM rendering: a group of QGraphicsLineItems, one per wall.
    QGraphicsItemGroup* _graphicsGroup;
    QList<QGraphicsLineItem*> _graphicsLines;
};

#endif // LAYERWALLS_H
