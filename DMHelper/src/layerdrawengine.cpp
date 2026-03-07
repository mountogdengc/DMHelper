#include "layerdrawengine.h"
#include "layerdrawtooldialog.h"
#include "layerdraw.h"
#include <QGraphicsItem>
#include <QPen>

LayerDrawEngine::LayerDrawEngine(QObject *parent) :
    QObject{parent},
    _drawLayer{nullptr},
    _toolDialog{nullptr},
    _mouseDownPos{},
    _activeDrawObject{nullptr},
    _activeItem{nullptr}
{}

void LayerDrawEngine::setDrawLayer(LayerDraw* drawLayer)
{
    _drawLayer = drawLayer;
}

void LayerDrawEngine::setActive(bool active)
{
    if(active)
    {
        if(!_toolDialog)
            _toolDialog = new LayerDrawToolDialog();

        _toolDialog->show();
    }
    else
    {
        if(_toolDialog)
            _toolDialog->hide();

        _activeItem = nullptr;
    }
}

void LayerDrawEngine::handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    if((!_drawLayer) || (buttons == Qt::NoButton))
        return;

    _mouseDownPos = pos;
    if(_activeDrawObject)
        delete _activeDrawObject;

    _activeDrawObject = new LayerDrawObjectPath(pos, _toolDialog->getToolLineColor(), _toolDialog->getToolLineWidth(), _toolDialog->getToolLineType());
}

void LayerDrawEngine::handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    if((!_drawLayer) || (buttons == Qt::NoButton))
        return;

    LayerDrawObjectPath* pathObject = dynamic_cast<LayerDrawObjectPath*>(_activeDrawObject);
    if(!pathObject)
        return;

    pathObject->addPoint(pos);

    if(!_activeItem)
    {
        _activeItem = _drawLayer->createGraphicsItem(_activeDrawObject);
    }
    else
    {
        QGraphicsPathItem* pathItem = dynamic_cast<QGraphicsPathItem*>(_activeItem);
        if(!pathItem)
            return;

        QPainterPath path = pathItem->path();
        path.lineTo(pos);
        pathItem->setPath(path);
    }
}

void LayerDrawEngine::handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    if(!_activeDrawObject)
        return;

    if(!_activeItem)
    {
        delete _activeDrawObject;
        _activeDrawObject = nullptr;
        return;
    }

    if(_drawLayer)
        _drawLayer->addObject(_activeDrawObject);

    _activeDrawObject = nullptr;
    _activeItem = nullptr;
}
