#ifndef LAYERTOKENSDARKENEFFECT_H
#define LAYERTOKENSDARKENEFFECT_H

#include <QGraphicsEffect>
#include <QPainter>

class LayerTokensDarkenEffect : public QGraphicsEffect
{
public:
    explicit LayerTokensDarkenEffect(QObject *parent = nullptr) : QGraphicsEffect(parent) {}

protected:
    QRectF boundingRectFor(const QRectF &sourceRect) const override {
        return sourceRect;
    }

    void draw(QPainter *painter) override {
        QPoint offset;
        QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
        if (pixmap.isNull())
            return;

        // Paint the dark overlay only onto opaque pixels of the source
        QPainter overlayPainter(&pixmap);
        overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        overlayPainter.fillRect(pixmap.rect(), QColor(0, 0, 0, 200));
        overlayPainter.end();

        // Draw in device coords so the dirty region matches exactly
        QTransform restoreTransform = painter->worldTransform();
        painter->setWorldTransform(QTransform());
        painter->drawPixmap(offset, pixmap);
        painter->setWorldTransform(restoreTransform);
    }
};

#endif // LAYERTOKENSDARKENEFFECT_H
