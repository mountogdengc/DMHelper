#ifndef LAYERDRAWENGINE_H
#define LAYERDRAWENGINE_H

#include <QObject>
#include <QPointF>
#include <QPainterPath>

class LayerDraw;
class LayerDrawToolDialog;
class QGraphicsPathItem;

class LayerDrawEngine : public QObject
{
    Q_OBJECT
public:
    explicit LayerDrawEngine(QObject *parent = nullptr);

public slots:
    void setDrawLayer(LayerDraw* drawLayer);
    void setActive(bool active);

    void handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    void handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    void handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);

signals:

private:
    LayerDraw* _drawLayer;
    LayerDrawToolDialog* _toolDialog;

    QPointF _mouseDownPos;
    QPainterPath _drawPath;
};

#endif // LAYERDRAWENGINE_H
