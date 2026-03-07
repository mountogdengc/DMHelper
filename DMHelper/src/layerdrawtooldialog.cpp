#include "layerdrawtooldialog.h"
#include "ui_layerdrawtooldialog.h"
#include <QMenu>

LayerDrawToolDialog::LayerDrawToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerDrawToolDialog)
{
    ui->setupUi(this);

    QMenu* lineTypeMenu = new QMenu(this);
    LayerDrawToolDialog_LineTypeAction* solidAction = new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pensolid.png"), QString("Solid Line"), Qt::SolidLine);
    lineTypeMenu->addAction(solidAction);
    ui->btnLineType->setIcon(solidAction->icon());
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendash.png"), QString("Dashed Line"), Qt::DashLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendot.png"), QString("Dotted Line"), Qt::DotLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendashdot.png"), QString("Dash Dot Line"), Qt::DashDotLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendashdotdot.png"), QString("Dash Dot Dot Line"), Qt::DashDotDotLine));
    ui->btnLineType->setMenu(lineTypeMenu);

    ui->btnColor->setRotationVisible(false);
}

LayerDrawToolDialog::~LayerDrawToolDialog()
{
    delete ui;
}

Qt::PenStyle LayerDrawToolDialog::getToolLineType() const
{
    return Qt::DashLine;
}

QColor LayerDrawToolDialog::getToolLineColor() const
{
    return ui->btnColor->getColor();
}

int LayerDrawToolDialog::getToolLineWidth() const
{
    return ui->spinSize->value();
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

