#include "battlecombatantwidget.h"
#include "ui_battlecombatantwidget.h"
#include "battledialogmodel.h"
#include "conditionseditdialog.h"
#include <QIntValidator>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QSpacerItem>

BattleCombatantWidget::BattleCombatantWidget(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BattleCombatantWidget),
    _combatant(combatant),
    _mouseDown(false),
    _mouseDownPos(),
    _result(0)
{
    ui->setupUi(this);
    QValidator* valHitPoints = new QIntValidator(-10, 9999, this);
    ui->edtHP->setValidator(valHitPoints);
    setCombatantValues();

    connect(ui->edtHP, SIGNAL(textEdited(QString)), this, SLOT(handleHitPointsChanged(QString)));
    connect(ui->btnAdvantage, SIGNAL(clicked(bool)), this, SLOT(handleAdvantageClicked(bool)));
    connect(ui->btnDisadvantage, SIGNAL(clicked(bool)), this, SLOT(handleDisadvantageClicked(bool)));
    connect(ui->chkActive, SIGNAL(toggled(bool)), this, SLOT(handleCombatantActive(bool)));
    connect(ui->btnConditions, &QAbstractButton::clicked, this, &BattleCombatantWidget::handleEditConditions);

    if(_combatant)
        connect(_combatant, &BattleDialogModelCombatant::conditionsChanged, this, &BattleCombatantWidget::handleConditionsChanged);
}

BattleCombatantWidget::~BattleCombatantWidget()
{
    delete ui;
}

bool BattleCombatantWidget::hasAdvantage() const
{
    return ui->btnAdvantage->isChecked();
}

bool BattleCombatantWidget::hasDisadvantage() const
{
    return ui->btnDisadvantage->isChecked();
}

void BattleCombatantWidget::setResult(const QString &text)
{
    ui->edtResult->setText(text);
}

void BattleCombatantWidget::setResult(int result)
{
    _result = result;
}

int BattleCombatantWidget::getResult() const
{
    return _result;
}

void BattleCombatantWidget::applyDamage(int damage)
{
    if(!_combatant)
        return;

    if(_combatant->getHitPoints() > damage)
        _combatant->setHitPoints(_combatant->getHitPoints() - damage);
    else
        _combatant->setHitPoints(0);

    emit combatantChanged(_combatant);
    setCombatantValues();
    emit hitPointsChanged(_combatant, damage);
}

void BattleCombatantWidget::applyConditions(int conditions)
{
    if((!_combatant) || (conditions == 0))
        return;

    _combatant->applyConditions(conditions);
    emit combatantChanged(_combatant);
}

bool BattleCombatantWidget::isActive()
{
    return ui->chkActive->isChecked();
}

void BattleCombatantWidget::mousePressEvent(QMouseEvent *event)
{
    if(!event)
        return;

    _mouseDown = true;
    _mouseDownPos = event->pos();
}

void BattleCombatantWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(!event)
        return;

    if((_mouseDown) && (_mouseDownPos == event->pos()))
    {
        emit selectCombatant(_combatant);
    }

    _mouseDown = false;
}

void BattleCombatantWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(!_combatant)
        return;

    QPixmap combatantImage = _combatant->getIconPixmap(DMHelper::PixmapSize_Full);
    ui->lblIcon->setPixmap(combatantImage.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void BattleCombatantWidget::handleHitPointsChanged(const QString& text)
{
    int newHP = text.toInt();

    if((!_combatant) || (newHP == _combatant->getHitPoints()))
        return;

    _combatant->setHitPoints(newHP);
    emit combatantChanged(_combatant);
}

void BattleCombatantWidget::handleRerollRequest()
{
    emit rerollNeeded(this);
}

void BattleCombatantWidget::handleAdvantageClicked(bool checked)
{
    if(checked && ui->btnDisadvantage->isChecked())
        ui->btnDisadvantage->setChecked(false);

    handleRerollRequest();
}

void BattleCombatantWidget::handleDisadvantageClicked(bool checked)
{
    if(checked && ui->btnAdvantage->isChecked())
        ui->btnAdvantage->setChecked(false);

    handleRerollRequest();
}

void BattleCombatantWidget::handleCombatantActive(bool active)
{
    ui->lblIcon->setEnabled(active);
    ui->edtName->setEnabled(active);
    ui->edtHP->setEnabled(active);
    ui->label->setEnabled(active);
    ui->edtResult->setEnabled(active);
    ui->btnAdvantage->setEnabled(active);
    ui->btnDisadvantage->setEnabled(active);
    ui->btnConditions->setEnabled(active);
    ui->frameConditions->setEnabled(active);
}

void BattleCombatantWidget::handleEditConditions()
{
    if(!_combatant)
        return;

    ConditionsEditDialog dlg(this);
    dlg.setConditions(_combatant->getConditions());
    if(dlg.exec() != QDialog::Accepted)
        return;

    if(dlg.getConditions() == _combatant->getConditions())
        return;

    _combatant->setConditions(dlg.getConditions());
    emit combatantChanged(_combatant);
}

void BattleCombatantWidget::handleConditionsChanged(BattleDialogModelCombatant* combatant)
{
    if(combatant != _combatant)
        return;

    updateConditionIcons();
}

void BattleCombatantWidget::setCombatantValues()
{
    if(!_combatant)
        return;

    ui->edtName->setText(_combatant->getName());
    ui->edtHP->setText(QString::number(_combatant->getHitPoints()));
    updateConditionIcons();
}

void BattleCombatantWidget::updateConditionIcons()
{
    QLayout* layout = ui->frameConditions->layout();
    if(!layout)
        return;

    while(QLayoutItem* item = layout->takeAt(0))
    {
        if(QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }

    const int iconSize = 18;
    const int conditions = _combatant ? _combatant->getConditions() : 0;
    for(int i = 0; i < Combatant::getConditionCount(); ++i)
    {
        const int condition = Combatant::getConditionByIndex(i);
        if((condition == Combatant::Condition_None) || !(conditions & condition))
            continue;

        const QString iconName = Combatant::getConditionIcon(condition);
        if(iconName.isEmpty())
            continue;

        QLabel* iconLabel = new QLabel(ui->frameConditions);
        const QPixmap pixmap(QString(":/img/data/img/") + iconName + QString(".png"));
        iconLabel->setPixmap(pixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setFixedSize(iconSize, iconSize);
        iconLabel->setToolTip(Combatant::getConditionTitle(condition));
        iconLabel->setStyleSheet(QString("background: transparent;"));
        layout->addWidget(iconLabel);
    }

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
}
