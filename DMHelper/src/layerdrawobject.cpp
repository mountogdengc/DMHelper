#include "layerdrawobject.h"
#include "dmconstants.h"
#include <QDomElement>

LayerDrawObject::LayerDrawObject() :
    _id(QUuid::createUuid())
{
}

LayerDrawObject::~LayerDrawObject()
{
}

QUuid LayerDrawObject::getId() const
{
    return _id;
}

void LayerDrawObject::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _id = QUuid(element.attribute(QString("id")));
}

void LayerDrawObject::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("id", _id.toString());
    element.setAttribute("type", QString::number(getType()));
}

LayerDrawObjectPath::LayerDrawObjectPath() :
    LayerDrawObject(),
    _penColor{Qt::black},
    _penWidth{1},
    _penStyle{Qt::SolidLine},
    _points{}
{
}

LayerDrawObjectPath::LayerDrawObjectPath(const QPointF& startPos, const QColor& penColor, int penWidth, Qt::PenStyle penStyle) :
    LayerDrawObject(),
    _penColor{penColor},
    _penWidth{penWidth},
    _penStyle{penStyle},
    _points{startPos}
{
}

LayerDrawObjectPath::~LayerDrawObjectPath()
{
}

int LayerDrawObjectPath::getType() const
{
    return DMHelper::ActionType_Path;
}

void LayerDrawObjectPath::inputXML(const QDomElement &element, bool isImport)
{
    LayerDrawObject::inputXML(element, isImport);

    // Parse pen attributes
    _penColor = QColor(element.attribute("penColor", "#000000"));
    _penWidth = element.attribute("penWidth", "1").toInt();
    _penStyle = static_cast<Qt::PenStyle>(element.attribute("penStyle", "1").toInt());

    // Parse points
    _points.clear();
    QDomElement pointElement = element.firstChildElement("point");
    while(!pointElement.isNull())
    {
        qreal x = pointElement.attribute("x").toDouble();
        qreal y = pointElement.attribute("y").toDouble();
        _points.append(QPointF(x, y));
        pointElement = pointElement.nextSiblingElement("point");
    }
}

void LayerDrawObjectPath::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    LayerDrawObject::outputXML(doc, element, targetDirectory, isExport);

    // Set pen attributes
    element.setAttribute("penColor", _penColor.name());
    element.setAttribute("penWidth", QString::number(_penWidth));
    element.setAttribute("penStyle", QString::number(static_cast<int>(_penStyle)));

    // Add points as child elements
    for(const QPointF& point : std::as_const(_points))
    {
        QDomElement pointElement = doc.createElement("point");
        pointElement.setAttribute("x", QString::number(point.x()));
        pointElement.setAttribute("y", QString::number(point.y()));
        element.appendChild(pointElement);
    }
}

void LayerDrawObjectPath::addPoint(const QPointF& point)
{
    _points.append(point);
}

const QList<QPointF>& LayerDrawObjectPath::getPoints() const
{
    return _points;
}

QColor LayerDrawObjectPath::getPenColor() const
{
    return _penColor;
}

int LayerDrawObjectPath::getPenWidth() const
{
    return _penWidth;
}

Qt::PenStyle LayerDrawObjectPath::getPenStyle() const
{
    return static_cast<Qt::PenStyle>(_penStyle);
}
