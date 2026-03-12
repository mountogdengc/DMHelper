#include "layerdrawshape.h"
#include "layerdrawobject.h"

LayerDrawShapePath::LayerDrawShapePath(const QPainterPath &path, LayerDrawObjectPath* pathObject) :
    QGraphicsPathItem(path),
    _object(pathObject)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant LayerDrawShapePath::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemScenePositionHasChanged) && (_object))
    {
        QPointF newPos = value.toPointF();
        _object->setPosition(newPos);
    }

    return QGraphicsItem::itemChange(change, value);
}
