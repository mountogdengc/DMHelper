#ifndef LAYERPARTICLESETTINGS_H
#define LAYERPARTICLESETTINGS_H

#include <QDialog>
#include <QColor>

namespace Ui {
class LayerParticleSettings;
}

class LayerParticleSettings : public QDialog
{
    Q_OBJECT

public:
    explicit LayerParticleSettings(QWidget *parent = nullptr);
    ~LayerParticleSettings();

    int particleCount() const;
    int rainSpeed() const;
    int rainAngle() const;
    QColor rainColor() const;
    int rainLength() const;

signals:
    void particleCountChanged(int count);
    void rainSpeedChanged(int speed);
    void rainAngleChanged(int angle);
    void rainColorChanged(const QColor& color);
    void rainLengthChanged(int length);

public slots:
    void setParticleCount(int count);
    void setRainSpeed(int speed);
    void setRainAngle(int angle);
    void setRainColor(const QColor& color);
    void setRainLength(int length);

private:
    Ui::LayerParticleSettings *ui;
};

#endif // LAYERPARTICLESETTINGS_H
