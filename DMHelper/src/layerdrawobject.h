#ifndef LAYERDRAWOBJECT_H
#define LAYERDRAWOBJECT_H

#include <QString>
#include <QList>
#include <QPointF>
#include <QColor>
#include <QUuid>

class QDomElement;
class QDomDocument;
class QDir;

class LayerDrawObject
{
public:
    explicit LayerDrawObject();
    virtual ~LayerDrawObject();

    virtual int getType() const = 0;    
    QUuid getId() const;

    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

private:
    QUuid _id;
};



class LayerDrawObjectPath : public LayerDrawObject
{
public:
    explicit LayerDrawObjectPath();
    explicit LayerDrawObjectPath(const QPointF& startPos, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine);
    virtual ~LayerDrawObjectPath();

    virtual int getType() const override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    void addPoint(const QPointF& point);

    const QList<QPointF>& getPoints() const;
    QColor getPenColor() const;
    int getPenWidth() const;
    Qt::PenStyle getPenStyle() const;


protected:
    QColor _penColor;
    int _penWidth;
    Qt::PenStyle _penStyle;
    QList<QPointF> _points;
};


#endif // LAYERDRAWOBJECT_H
