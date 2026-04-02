#include "ribbontabtext.h"
#include "ui_ribbontabtext.h"
#include <QFontDatabase>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

RibbonTabText::RibbonTabText(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabText),
    _btnCheckbox(nullptr),
    _lblCheckbox(nullptr)
{
    ui->setupUi(this);

    connect(ui->frameText, &RibbonFrameText::colorChanged, this, &RibbonTabText::colorChanged);
    connect(ui->frameText, &RibbonFrameText::fontFamilyChanged, this, &RibbonTabText::fontFamilyChanged);
    connect(ui->frameText, &RibbonFrameText::fontSizeChanged, this, &RibbonTabText::fontSizeChanged);
    connect(ui->frameText, &RibbonFrameText::fontBoldChanged, this, &RibbonTabText::fontBoldChanged);
    connect(ui->frameText, &RibbonFrameText::fontItalicsChanged, this, &RibbonTabText::fontItalicsChanged);
    connect(ui->frameText, &RibbonFrameText::fontUnderlineChanged, this, &RibbonTabText::fontUnderlineChanged);
    connect(ui->frameText, &RibbonFrameText::alignmentChanged, this, &RibbonTabText::alignmentChanged);
    connect(ui->btnPasteRich, &QAbstractButton::toggled, this, &RibbonTabText::pasteRichChanged);

    connect(ui->btnHyperlink, &QAbstractButton::clicked, this, &RibbonTabText::hyperlinkClicked);

    // Programmatic checkbox toggle button
    _btnCheckbox = new QPushButton(this);
    _btnCheckbox->setText(QString(QChar(0x2611)));
    _btnCheckbox->setToolTip(QString("Toggle checkbox on current line"));
    _btnCheckbox->setMinimumSize(50, 50);
    _btnCheckbox->setMaximumSize(50, 50);
    _btnCheckbox->setFlat(true);
    QFont checkFont = _btnCheckbox->font();
    checkFont.setPointSize(20);
    _btnCheckbox->setFont(checkFont);

    _lblCheckbox = new QLabel(QString("Checkbox"), this);
    _lblCheckbox->setMinimumSize(0, 15);
    _lblCheckbox->setMaximumSize(16777215, 15);
    _lblCheckbox->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    QVBoxLayout* checkboxLayout = new QVBoxLayout();
    checkboxLayout->setSpacing(0);
    checkboxLayout->addWidget(_btnCheckbox);
    checkboxLayout->addWidget(_lblCheckbox);

    // Insert after the hyperlink button (find its position in the horizontal layout)
    QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(layout());
    if(hLayout)
    {
        // Find index of line_2 separator and insert before it
        int insertIndex = -1;
        for(int i = 0; i < hLayout->count(); ++i)
        {
            QLayoutItem* item = hLayout->itemAt(i);
            if(item && item->widget() == ui->line_2)
            {
                insertIndex = i;
                break;
            }
        }
        if(insertIndex >= 0)
            hLayout->insertLayout(insertIndex, checkboxLayout);
        else
            hLayout->addLayout(checkboxLayout);
    }

    connect(_btnCheckbox, &QAbstractButton::clicked, this, &RibbonTabText::checkboxClicked);

    connect(ui->btnTranslateText, &QAbstractButton::clicked, this, &RibbonTabText::translateTextClicked);
    connect(ui->btnCode, &QAbstractButton::clicked, this, &RibbonTabText::codeViewClicked);

    connect(ui->sliderWidth, &QAbstractSlider::valueChanged, this, &RibbonTabText::widthChanged);
    connect(ui->spinSpeed, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(ui->btnRewind, SIGNAL(clicked()), this, SIGNAL(rewindClicked()));

    connect(ui->btnPlayPause, &QAbstractButton::toggled, this, &RibbonTabText::playPauseClicked);

    connect(ui->btnAnimation, &QAbstractButton::clicked, this, &RibbonTabText::animationClicked);
}

RibbonTabText::~RibbonTabText()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabText::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabText::setAnimation(bool checked)
{
    ui->btnAnimation->setChecked(checked);
    ui->spinSpeed->setEnabled(checked);
    ui->btnRewind->setEnabled(checked);
    ui->btnPlayPause->setEnabled(checked);
}

void RibbonTabText::setColor(const QColor& color)
{
    ui->frameText->setColor(color);
}

void RibbonTabText::setFontFamily(const QString& fontFamily)
{
    ui->frameText->setFontFamily(fontFamily);
}

void RibbonTabText::setFontSize(int fontSize)
{
    ui->frameText->setFontSize(fontSize);
}

void RibbonTabText::setFontBold(bool fontBold)
{
    ui->frameText->setFontBold(fontBold);
}

void RibbonTabText::setFontItalics(bool fontItalics)
{
    ui->frameText->setFontItalics(fontItalics);
}

void RibbonTabText::setFontUnderline(bool fontUnderline)
{
    ui->frameText->setFontUnderline(fontUnderline);
}

void RibbonTabText::setAlignment(Qt::Alignment alignment)
{
    ui->frameText->setAlignment(alignment);
}

void RibbonTabText::setPasteRich(bool pasteRich)
{
    ui->btnPasteRich->setChecked(pasteRich);
}

void RibbonTabText::setWidth(int width)
{
    if((width != ui->sliderWidth->value()) &&
       (width >= ui->sliderWidth->minimum()) &&
       (width <= ui->sliderWidth->maximum()))
    {
        ui->sliderWidth->setValue(width);
    }
}

void RibbonTabText::setSpeed(int speed)
{
    if((speed != ui->spinSpeed->value()) &&
       (speed >= ui->spinSpeed->minimum()) &&
       (speed <= ui->spinSpeed->maximum()))
    {
        ui->spinSpeed->setValue(speed);
    }
}

void RibbonTabText::setPlaying(bool playing)
{
    ui->lblPlayPause->setText(playing ? QString("Pause") : QString("Play"));
    ui->btnPlayPause->setIcon(playing ? QIcon(":/img/data/icon_pause.png") : QIcon(":/img/data/icon_play.png"));
    ui->btnPlayPause->setChecked(playing);
}

void RibbonTabText::setHyperlinkActive(bool active)
{
    ui->btnHyperlink->setEnabled(active);
}

void RibbonTabText::setTranslationActive(bool active)
{
    ui->btnTranslateText->setChecked(active);
}

void RibbonTabText::setCodeView(bool active)
{
    ui->btnCode->setChecked(active);
}

void RibbonTabText::showCodeView(bool visible)
{
    ui->lblTranslateText->setVisible(!visible);
    ui->btnTranslateText->setVisible(!visible);
    ui->lblCode->setVisible(visible);
    ui->btnCode->setVisible(visible);
}

void RibbonTabText::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setLineHeight(*ui->line_1, frameHeight);
    setStandardButtonSize(*ui->lblPasteRich, *ui->btnPasteRich, frameHeight);
    setStandardButtonSize(*ui->lblHyperlink, *ui->btnHyperlink, frameHeight);
    setStandardButtonSize(*_lblCheckbox, *_btnCheckbox, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblAnimation, *ui->btnAnimation, frameHeight);
    setStandardButtonSize(*ui->lblPlayPause, *ui->btnPlayPause, frameHeight);
    setStandardButtonSize(*ui->lblRewind, *ui->btnRewind, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblTranslateText, *ui->btnTranslateText, frameHeight);
    setStandardButtonSize(*ui->lblCode, *ui->btnCode, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
}

