#include "layerwalls.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QPen>
#include <QDebug>

namespace
{
    constexpr qreal kDefaultThickness = 2.0;
    const QColor kDefaultColor(220, 60, 60, 220); // semi-transparent red, visible over most maps
}

LayerWalls::LayerWalls(const QString& name, int order, QObject* parent) :
    Layer(name, order, parent),
    _walls(),
    _wallColor(kDefaultColor),
    _wallThickness(kDefaultThickness),
    _graphicsGroup(nullptr),
    _graphicsLines()
{
}

LayerWalls::~LayerWalls()
{
    cleanupDM();
}

void LayerWalls::inputXML(const QDomElement& element, bool isImport)
{
    _walls.clear();

    const QString colorAttr = element.attribute(QString("wallColor"));
    if(!colorAttr.isEmpty())
        _wallColor = QColor(colorAttr);

    const QString thicknessAttr = element.attribute(QString("wallThickness"));
    if(!thicknessAttr.isEmpty())
        _wallThickness = thicknessAttr.toDouble();

    QDomElement wallsElement = element.firstChildElement(QString("walls"));
    if(!wallsElement.isNull())
    {
        QDomElement wallElement = wallsElement.firstChildElement(QString("wall"));
        while(!wallElement.isNull())
        {
            const qreal x1 = wallElement.attribute(QString("x1")).toDouble();
            const qreal y1 = wallElement.attribute(QString("y1")).toDouble();
            const qreal x2 = wallElement.attribute(QString("x2")).toDouble();
            const qreal y2 = wallElement.attribute(QString("y2")).toDouble();
            _walls.append(QLineF(x1, y1, x2, y2));
            wallElement = wallElement.nextSiblingElement(QString("wall"));
        }
    }

    Layer::inputXML(element, isImport);
    rebuildGraphicsItems();
    emit wallsChanged();
}

QRectF LayerWalls::boundingRect() const
{
    if(_walls.isEmpty())
        return QRectF(_position, _size);

    qreal minX = _walls.first().x1();
    qreal minY = _walls.first().y1();
    qreal maxX = minX;
    qreal maxY = minY;

    for(const QLineF& line : _walls)
    {
        minX = qMin(minX, qMin(line.x1(), line.x2()));
        minY = qMin(minY, qMin(line.y1(), line.y2()));
        maxX = qMax(maxX, qMax(line.x1(), line.x2()));
        maxY = qMax(maxY, qMax(line.y1(), line.y2()));
    }

    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

QImage LayerWalls::getLayerIcon() const
{
    // Reuse an existing asset if available; otherwise fall back to a generic one.
    // The ':/img/data/icon_walls.png' resource is not required for the layer to
    // function — callers tolerate a null icon.
    return QImage(":/img/data/icon_walls.png");
}

DMHelper::LayerType LayerWalls::getType() const
{
    return DMHelper::LayerType_Walls;
}

bool LayerWalls::hasSettings() const
{
    return true;
}

Layer* LayerWalls::clone() const
{
    LayerWalls* copy = new LayerWalls(_name, _order);
    copyBaseValues(copy);
    copy->_walls = _walls;
    copy->_wallColor = _wallColor;
    copy->_wallThickness = _wallThickness;
    return copy;
}

void LayerWalls::applyOrder(int order)
{
    if(_graphicsGroup)
        _graphicsGroup->setZValue(order);
}

void LayerWalls::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsGroup)
        _graphicsGroup->setVisible(layerVisible);
}

void LayerWalls::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerWalls::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;
    if(_graphicsGroup)
        _graphicsGroup->setOpacity(opacity);
}

void LayerWalls::applyPosition(const QPoint& position)
{
    if(_graphicsGroup)
        _graphicsGroup->setPos(position);
}

void LayerWalls::applySize(const QSize& size)
{
    Q_UNUSED(size);
}

const QList<QLineF>& LayerWalls::walls() const
{
    return _walls;
}

void LayerWalls::addWall(const QLineF& segment)
{
    if(segment.length() <= 0.0)
        return;
    _walls.append(segment);
    rebuildGraphicsItems();
    emit wallsChanged();
    emit dirty();
}

void LayerWalls::addWalls(const QList<QLineF>& segments)
{
    bool any = false;
    for(const QLineF& seg : segments)
    {
        if(seg.length() <= 0.0)
            continue;
        _walls.append(seg);
        any = true;
    }
    if(any)
    {
        rebuildGraphicsItems();
        emit wallsChanged();
        emit dirty();
    }
}

void LayerWalls::removeWall(int index)
{
    if(index < 0 || index >= _walls.size())
        return;
    _walls.removeAt(index);
    rebuildGraphicsItems();
    emit wallsChanged();
    emit dirty();
}

void LayerWalls::clearWalls()
{
    if(_walls.isEmpty())
        return;
    _walls.clear();
    rebuildGraphicsItems();
    emit wallsChanged();
    emit dirty();
}

int LayerWalls::wallCount() const
{
    return _walls.size();
}

QColor LayerWalls::wallColor() const
{
    return _wallColor;
}

void LayerWalls::setWallColor(const QColor& color)
{
    if(_wallColor == color)
        return;
    _wallColor = color;
    rebuildGraphicsItems();
    emit dirty();
}

qreal LayerWalls::wallThickness() const
{
    return _wallThickness;
}

void LayerWalls::setWallThickness(qreal thickness)
{
    if(qFuzzyCompare(_wallThickness, thickness))
        return;
    _wallThickness = thickness;
    rebuildGraphicsItems();
    emit dirty();
}

QPolygonF LayerWalls::computeVisionPolygon(const QPointF& viewpoint,
                                           qreal maxRadius,
                                           const QRectF& bounds) const
{
    QRectF useBounds = bounds;
    if(useBounds.isNull() || useBounds.isEmpty())
        useBounds = QRectF(QPointF(0, 0), _size.isValid() ? _size : QSize(1, 1));
    return VisionCalculator::computeVisionPolygon(viewpoint, _walls, useBounds, maxRadius);
}

void LayerWalls::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsGroup)
    {
        qDebug() << "[LayerWalls] ERROR: dmInitialize called although the graphics group already exists!";
        return;
    }

    _graphicsGroup = new QGraphicsItemGroup();
    _graphicsGroup->setHandlesChildEvents(false);
    _graphicsGroup->setPos(_position);
    _graphicsGroup->setZValue(getOrder());
    _graphicsGroup->setOpacity(_opacityReference > 0.0 ? _opacityReference : 1.0);
    scene->addItem(_graphicsGroup);

    rebuildGraphicsItems();

    Layer::dmInitialize(scene);
}

void LayerWalls::dmUninitialize()
{
    cleanupDM();
}

void LayerWalls::dmUpdate()
{
}

void LayerWalls::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    // Walls are DM-only; no player-side rendering. The LOS polygon they drive
    // is applied via the fog-of-war or a future vision mask layer.
    Layer::playerGLInitialize(renderer, scene);
}

void LayerWalls::playerGLUninitialize()
{
}

void LayerWalls::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(functions);
    Q_UNUSED(defaultModelMatrix);
    Q_UNUSED(projectionMatrix);
}

void LayerWalls::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerWalls::playerIsInitialized()
{
    return true;
}

void LayerWalls::initialize(const QSize& sceneSize)
{
    if(getSize().isEmpty())
        setSize(sceneSize);
}

void LayerWalls::uninitialize()
{
}

void LayerWalls::editSettings()
{
    // Settings dialog (color / thickness) will hang off the layer editor once
    // the UI for wall drawing lands. For now, setters are the public API.
}

void LayerWalls::internalOutputXML(QDomDocument& doc, QDomElement& element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("wallColor", _wallColor.name(QColor::HexArgb));
    element.setAttribute("wallThickness", QString::number(_wallThickness));

    if(!_walls.isEmpty())
    {
        QDomElement wallsElement = doc.createElement("walls");
        for(const QLineF& line : _walls)
        {
            QDomElement wallElement = doc.createElement("wall");
            wallElement.setAttribute("x1", QString::number(line.x1()));
            wallElement.setAttribute("y1", QString::number(line.y1()));
            wallElement.setAttribute("x2", QString::number(line.x2()));
            wallElement.setAttribute("y2", QString::number(line.y2()));
            wallsElement.appendChild(wallElement);
        }
        element.appendChild(wallsElement);
    }

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerWalls::rebuildGraphicsItems()
{
    if(!_graphicsGroup)
        return;

    for(QGraphicsLineItem* item : _graphicsLines)
    {
        _graphicsGroup->removeFromGroup(item);
        delete item;
    }
    _graphicsLines.clear();

    QPen pen(_wallColor);
    pen.setWidthF(_wallThickness);
    pen.setCapStyle(Qt::RoundCap);
    pen.setCosmetic(true);

    for(const QLineF& line : _walls)
    {
        QGraphicsLineItem* item = new QGraphicsLineItem(line);
        item->setPen(pen);
        item->setFlag(QGraphicsItem::ItemIsMovable, false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsGroup->addToGroup(item);
        _graphicsLines.append(item);
    }
}

void LayerWalls::cleanupDM()
{
    for(QGraphicsLineItem* item : _graphicsLines)
        delete item;
    _graphicsLines.clear();

    if(_graphicsGroup)
    {
        QGraphicsScene* scene = _graphicsGroup->scene();
        if(scene)
            scene->removeItem(_graphicsGroup);
        delete _graphicsGroup;
        _graphicsGroup = nullptr;
    }
}
