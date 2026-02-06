#include "layerdrawengine.h"
#include "layerdrawtooldialog.h"
#include "layerdraw.h"
#include <QGraphicsPathItem>
#include <QPainter>

LayerDrawEngine::LayerDrawEngine(QObject *parent) :
    QObject{parent},
    _drawLayer{nullptr},
    _toolDialog{nullptr},
    _mouseDownPos{},
    _drawPath{}
{}

void LayerDrawEngine::setDrawLayer(LayerDraw* drawLayer)
{
    _drawLayer = drawLayer;
}

void LayerDrawEngine::setActive(bool active)
{
    if (active)
    {
        if (!_toolDialog)
            _toolDialog = new LayerDrawToolDialog();

        _toolDialog->show();
    }
    else
    {
        if (_toolDialog)
            _toolDialog->hide();
    }
}

void LayerDrawEngine::handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    _mouseDownPos = pos;
    _drawPath.clear();
    _drawPath.moveTo(pos);
}

void LayerDrawEngine::handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    if((!_drawLayer) || (buttons == Qt::NoButton) || (_drawPath.isEmpty()))
        return;

    QPainter* painter = _drawLayer->beginPainting();
    painter->setPen(QPen(Qt::red, 5));
    painter->drawLine(_drawPath.currentPosition(), pos);
    _drawLayer->endPainting();

    _drawPath.lineTo(pos);
}

void LayerDrawEngine::handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
}





/*
bool BattleDialogGraphicsSceneMouseHandlerDistance::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if((!_distanceLine) || (!_distanceText) || (_scale <= 0.0) || (mouseEvent->buttons() == Qt::NoButton))
        return false;

    QLineF line = _distanceLine->line();
    QPointF position = mouseEvent->scenePos();
    if(_scene.getModel())
    {
        LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(_scene.getModel()->getLayerScene().getFirst(DMHelper::LayerType_Grid));
        if((gridLayer) && (gridLayer->getConfig().isSnapToGrid()))
        {
            // Snap the current position to the grid
            QPointF offset = gridLayer->getConfig().getGridOffset() * gridLayer->getConfig().getGridScale() / 100.0;
            position -= offset;
            qreal gridSize = _scene.getModel()->getLayerScene().getScale();
            int intGridSize = static_cast<int>(gridSize);
            position.setX((static_cast<qreal>(static_cast<int>(position.x()) / intGridSize) * gridSize) + gridSize);
            position.setY((static_cast<qreal>(static_cast<int>(position.y()) / intGridSize) * gridSize) + gridSize);
            position += offset;
            //return mapToParent(mapFromScene(newPos));
        }
    }
    //line.setP2(mouseEvent->scenePos());
    line.setP2(position);
    _distanceLine->setLine(line);

    updateDistance();

    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(_distanceLine)
        delete _distanceLine;

    QPointF textposition = mouseEvent->scenePos();
    if(_scene.getModel())
    {
        LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(_scene.getModel()->getLayerScene().getFirst(DMHelper::LayerType_Grid));
        if((gridLayer) && (gridLayer->getConfig().isSnapToGrid()))
        {
            // Snap the current position to the grid
            QPointF offset = gridLayer->getConfig().getGridOffset() * gridLayer->getConfig().getGridScale() / 100.0;
            textposition -= offset;
            qreal gridSize = _scene.getModel()->getLayerScene().getScale();
            int intGridSize = static_cast<int>(gridSize);
            textposition.setX((static_cast<qreal>(static_cast<int>(textposition.x()) / intGridSize) * gridSize) + gridSize);
            textposition.setY((static_cast<qreal>(static_cast<int>(textposition.y()) / intGridSize) * gridSize) + gridSize);
            textposition += offset;
            //return mapToParent(mapFromScene(newPos));
        }
    }

    _distanceLine = _scene.addLine(QLineF(textposition, textposition), QPen(QBrush(_color), _lineWidth, static_cast<Qt::PenStyle>(_lineType)));
    _distanceLine->setPen(QPen(QBrush(_color), _lineWidth, static_cast<Qt::PenStyle>(_lineType)));
    _distanceLine->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);

    if(_distanceText)
        delete _distanceText;
    _distanceText = _scene.addSimpleText(QString("0"));
    _distanceText->setBrush(QBrush(_color));
    _distanceText->setPos(textposition);
    _distanceText->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _distanceText->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    QFont textFont = _distanceText->font();
    textFont.setPointSize(10);
    _distanceText->setFont(textFont);

    emit distanceItemChanged(_distanceLine, _distanceText);

    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mouseEvent->accept();
    return false;
}

void BattleDialogGraphicsSceneMouseHandlerDistance::updateDistance()
{
    if((!_distanceLine) || (!_distanceText) || (_scale <= 0.0))
        return;

    QLineF line = _distanceLine->line();
    qreal lineDistance = 5.0 * line.length() / _scale;

    QString distanceText = createDistanceString(lineDistance);

    _distanceText->setText(distanceText);
    _distanceText->setPos(line.center());
    emit distanceChanged(distanceText);
}
*/
