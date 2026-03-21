#include "layerdrawtooldialog.h"
#include "ui_layerdrawtooldialog.h"
#include "colorpushbutton.h"
#include <QMenu>
#include <QFontComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

LayerDrawToolDialog::LayerDrawToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerDrawToolDialog),
    _currentLineType{Qt::SolidLine},
    _btnStraightLine{nullptr},
    _btnText{nullptr},
    _fillCheck{nullptr},
    _btnFillColor{nullptr},
    _fontLabel{nullptr},
    _fontCombo{nullptr},
    _fontSizeSpin{nullptr}
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Line type menu
    QMenu* lineTypeMenu = new QMenu(this);
    LayerDrawToolDialog_LineTypeAction* solidAction = new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pensolid.png"), QString("Solid Line"), Qt::SolidLine);
    lineTypeMenu->addAction(solidAction);
    ui->btnLineType->setIcon(solidAction->icon());
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendash.png"), QString("Dashed Line"), Qt::DashLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendot.png"), QString("Dotted Line"), Qt::DotLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendashdot.png"), QString("Dash Dot Line"), Qt::DashDotLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendashdotdot.png"), QString("Dash Dot Dot Line"), Qt::DashDotDotLine));
    ui->btnLineType->setMenu(lineTypeMenu);
    connect(lineTypeMenu, &QMenu::triggered, this, &LayerDrawToolDialog::handleLineTypeTriggered);

    ui->btnColor->setRotationVisible(false);

    // Add straight line button (row 2, col 0)
    _btnStraightLine = new QPushButton(this);
    _btnStraightLine->setMinimumSize(50, 50);
    _btnStraightLine->setMaximumSize(50, 50);
    _btnStraightLine->setToolTip(QString("Straight Line"));
    _btnStraightLine->setIcon(QIcon(":/img/data/icon_neweffectline.png"));
    _btnStraightLine->setIconSize(QSize(40, 40));
    _btnStraightLine->setCheckable(true);
    ui->gridLayout->addWidget(_btnStraightLine, 2, 0);
    ui->buttonGroup->addButton(_btnStraightLine);

    // Add text button (row 2, col 1)
    _btnText = new QPushButton(this);
    _btnText->setMinimumSize(50, 50);
    _btnText->setMaximumSize(50, 50);
    _btnText->setToolTip(QString("Text"));
    _btnText->setText(QString("T"));
    QFont textBtnFont = _btnText->font();
    textBtnFont.setPointSize(20);
    textBtnFont.setBold(true);
    _btnText->setFont(textBtnFont);
    _btnText->setCheckable(true);
    ui->gridLayout->addWidget(_btnText, 2, 1);
    ui->buttonGroup->addButton(_btnText);

    // Fill color picker
    _fillCheck = new QCheckBox(QString("Fill"), this);
    _fillCheck->setChecked(false);

    _btnFillColor = new ColorPushButton(this);
    _btnFillColor->setMinimumSize(50, 50);
    _btnFillColor->setMaximumSize(50, 50);
    _btnFillColor->setToolTip(QString("Fill Color"));
    _btnFillColor->setColor(Qt::white);
    _btnFillColor->setRotationVisible(false);
    _btnFillColor->setEnabled(false);

    connect(_fillCheck, &QCheckBox::toggled, this, &LayerDrawToolDialog::updateControlStates);

    QHBoxLayout* fillLayout = new QHBoxLayout();
    fillLayout->addWidget(_fillCheck);
    fillLayout->addWidget(_btnFillColor);
    ui->verticalLayout->addLayout(fillLayout);

    // Font controls
    _fontLabel = new QLabel(QString("Font:"), this);
    _fontCombo = new QFontComboBox(this);
    _fontCombo->setCurrentFont(QFont("Arial"));

    _fontSizeSpin = new QSpinBox(this);
    _fontSizeSpin->setMinimum(4);
    _fontSizeSpin->setMaximum(200);
    _fontSizeSpin->setValue(12);
    _fontSizeSpin->setSuffix(QString("pt"));

    QHBoxLayout* fontLayout = new QHBoxLayout();
    fontLayout->addWidget(_fontLabel);
    fontLayout->addWidget(_fontCombo);
    fontLayout->addWidget(_fontSizeSpin);
    ui->verticalLayout->addLayout(fontLayout);

    // Connect button group to update enabled state
    connect(ui->buttonGroup, &QButtonGroup::idClicked, this, &LayerDrawToolDialog::handleToolChanged);

    updateControlStates();
}

LayerDrawToolDialog::~LayerDrawToolDialog()
{
    delete ui;
}

DMHelper::DrawToolType LayerDrawToolDialog::getToolType() const
{
    QAbstractButton* checked = ui->buttonGroup->checkedButton();
    if(!checked)
        return DMHelper::DrawToolType_Path;

    if(checked == ui->btnLine)
        return DMHelper::DrawToolType_Path;
    if(checked == ui->btnErase)
        return DMHelper::DrawToolType_Eraser;
    if(checked == ui->btnCircle)
        return DMHelper::DrawToolType_Ellipse;
    if(checked == ui->btnRectangle)
        return DMHelper::DrawToolType_Rect;
    if(checked == _btnStraightLine)
        return DMHelper::DrawToolType_Line;
    if(checked == _btnText)
        return DMHelper::DrawToolType_Text;

    return DMHelper::DrawToolType_Path;
}

Qt::PenStyle LayerDrawToolDialog::getToolLineType() const
{
    return _currentLineType;
}

QColor LayerDrawToolDialog::getToolLineColor() const
{
    return ui->btnColor->getColor();
}

int LayerDrawToolDialog::getToolLineWidth() const
{
    return ui->spinSize->value();
}

QColor LayerDrawToolDialog::getToolFillColor() const
{
    if(!_btnFillColor)
        return Qt::transparent;
    return _btnFillColor->getColor();
}

bool LayerDrawToolDialog::isToolFilled() const
{
    if(!_fillCheck)
        return false;
    return _fillCheck->isChecked();
}

QString LayerDrawToolDialog::getToolFontFamily() const
{
    if(!_fontCombo)
        return QString("Arial");
    return _fontCombo->currentFont().family();
}

int LayerDrawToolDialog::getToolFontSize() const
{
    if(!_fontSizeSpin)
        return 12;
    return _fontSizeSpin->value();
}

void LayerDrawToolDialog::handleLineTypeTriggered(QAction* action)
{
    LayerDrawToolDialog_LineTypeAction* lineAction = dynamic_cast<LayerDrawToolDialog_LineTypeAction*>(action);
    if(!lineAction)
        return;

    _currentLineType = static_cast<Qt::PenStyle>(lineAction->getLineType());
    ui->btnLineType->setIcon(action->icon());
}

void LayerDrawToolDialog::handleToolChanged()
{
    updateControlStates();
}

void LayerDrawToolDialog::updateControlStates()
{
    DMHelper::DrawToolType tool = getToolType();

    bool isPen    = (tool != DMHelper::DrawToolType_Text) && (tool != DMHelper::DrawToolType_Eraser);
    bool isFill   = (tool == DMHelper::DrawToolType_Rect) || (tool == DMHelper::DrawToolType_Ellipse);
    bool isFont   = (tool == DMHelper::DrawToolType_Text);
    bool isEraser = (tool == DMHelper::DrawToolType_Eraser);

    ui->btnColor->setEnabled(!isEraser);
    ui->btnLineType->setEnabled(isPen);
    ui->lblSize->setEnabled(isPen);
    ui->spinSize->setEnabled(isPen);

    if(_fillCheck)
        _fillCheck->setEnabled(isFill);
    if(_btnFillColor)
        _btnFillColor->setEnabled(isFill && _fillCheck && _fillCheck->isChecked());

    if(_fontLabel)
        _fontLabel->setEnabled(isFont);
    if(_fontCombo)
        _fontCombo->setEnabled(isFont);
    if(_fontSizeSpin)
        _fontSizeSpin->setEnabled(isFont);
}





LayerDrawToolDialog_LineTypeAction::LayerDrawToolDialog_LineTypeAction(const QIcon &icon, const QString &text, int lineType, QObject *parent) :
    QAction(icon, text, parent),
    _lineType(lineType)
{
}

LayerDrawToolDialog_LineTypeAction::~LayerDrawToolDialog_LineTypeAction()
{
}

int LayerDrawToolDialog_LineTypeAction::getLineType() const
{
    return _lineType;
}

