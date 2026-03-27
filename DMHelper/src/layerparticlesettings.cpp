#include "layerparticlesettings.h"
#include "ui_layerparticlesettings.h"
#include "colorpushbutton.h"

LayerParticleSettings::LayerParticleSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerParticleSettings)
{
    ui->setupUi(this);

    ui->btnRainColor->setColor(QColor(200, 200, 255, 180));
    ui->btnRainColor->setRotationVisible(false);

    connect(ui->spinCount, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::particleCountChanged);
    connect(ui->slideCount, &QSlider::valueChanged, ui->spinCount, &QSpinBox::setValue);
    connect(ui->spinCount, qOverload<int>(&QSpinBox::valueChanged), ui->slideCount, &QSlider::setValue);

    connect(ui->spinSpeed, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainSpeedChanged);
    connect(ui->slideSpeed, &QSlider::valueChanged, ui->spinSpeed, &QSpinBox::setValue);
    connect(ui->spinSpeed, qOverload<int>(&QSpinBox::valueChanged), ui->slideSpeed, &QSlider::setValue);

    connect(ui->spinDirection, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainDirectionChanged);
    connect(ui->slideDirection, &QSlider::valueChanged, ui->spinDirection, &QSpinBox::setValue);
    connect(ui->spinDirection, qOverload<int>(&QSpinBox::valueChanged), ui->slideDirection, &QSlider::setValue);

    connect(ui->spinAngle, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainAngleChanged);
    connect(ui->slideAngle, &QSlider::valueChanged, ui->spinAngle, &QSpinBox::setValue);
    connect(ui->spinAngle, qOverload<int>(&QSpinBox::valueChanged), ui->slideAngle, &QSlider::setValue);

    connect(ui->spinLength, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainLengthChanged);
    connect(ui->slideLength, &QSlider::valueChanged, ui->spinLength, &QSpinBox::setValue);
    connect(ui->spinLength, qOverload<int>(&QSpinBox::valueChanged), ui->slideLength, &QSlider::setValue);

    connect(ui->spinOpacity, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainOpacityChanged);
    connect(ui->slideOpacity, &QSlider::valueChanged, ui->spinOpacity, &QSpinBox::setValue);
    connect(ui->spinOpacity, qOverload<int>(&QSpinBox::valueChanged), ui->slideOpacity, &QSlider::setValue);

    connect(ui->btnRainColor, &ColorPushButton::colorChanged, this, &LayerParticleSettings::rainColorChanged);
}

LayerParticleSettings::~LayerParticleSettings()
{
    delete ui;
}

int LayerParticleSettings::particleCount() const
{
    return ui->spinCount->value();
}

int LayerParticleSettings::rainSpeed() const
{
    return ui->spinSpeed->value();
}

int LayerParticleSettings::rainDirection() const
{
    return ui->spinDirection->value();
}

int LayerParticleSettings::rainAngle() const
{
    return ui->spinAngle->value();
}

QColor LayerParticleSettings::rainColor() const
{
    return ui->btnRainColor->getColor();
}

int LayerParticleSettings::rainLength() const
{
    return ui->spinLength->value();
}

int LayerParticleSettings::rainOpacity() const
{
    return ui->spinOpacity->value();
}

void LayerParticleSettings::setParticleCount(int count)
{
    ui->spinCount->setValue(count);
}

void LayerParticleSettings::setRainSpeed(int speed)
{
    ui->spinSpeed->setValue(speed);
}

void LayerParticleSettings::setRainDirection(int direction)
{
    ui->spinDirection->setValue(direction);
}

void LayerParticleSettings::setRainAngle(int angle)
{
    ui->spinAngle->setValue(angle);
}

void LayerParticleSettings::setRainColor(const QColor& color)
{
    ui->btnRainColor->setColor(color);
}

void LayerParticleSettings::setRainLength(int length)
{
    ui->spinLength->setValue(length);
}

void LayerParticleSettings::setRainOpacity(int opacity)
{
    ui->spinOpacity->setValue(opacity);
}
