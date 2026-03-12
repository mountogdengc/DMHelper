#ifndef LAYERDRAWSHAPE_H
#define LAYERDRAWSHAPE_H

#include <QGraphicsPathItem>

class LayerDrawObjectPath;

class LayerDrawShapePath : public QGraphicsPathItem
{
public:
    explicit LayerDrawShapePath(const QPainterPath &path, LayerDrawObjectPath* pathObject);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    LayerDrawObjectPath* _object;
};

#endif // LAYERDRAWSHAPE_H
