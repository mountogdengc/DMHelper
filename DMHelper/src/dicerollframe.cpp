#include "dicerollframe.h"
#include "ui_dicerollframe.h"
#include "dice.h"
#include "diceanimationwidget.h"
#include <QtGlobal>
#include <QIntValidator>
#include <QHBoxLayout>

DiceRollFrame::DiceRollFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DiceRollFrame),
    _diceAnim(nullptr),
    _pendingResultStrings(),
    _pendingTotalString()
{
    ui->setupUi(this);
    init();
}

DiceRollFrame::DiceRollFrame(const Dice& dice, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DiceRollFrame),
    _diceAnim(nullptr),
    _pendingResultStrings(),
    _pendingTotalString()
{
    ui->setupUi(this);
    init();

    ui->editRollCount->setText(QString("1"));
    ui->editDiceCount->setText(QString::number(dice.getCount()));
    ui->editDiceType->setText(QString::number(dice.getType()));
    ui->editBonus->setText(QString::number(dice.getBonus()));

    rollDice();
}

DiceRollFrame::~DiceRollFrame()
{
    delete ui;
}

void DiceRollFrame::rollDice()
{
    ui->editResult->clear();

    const int rcEnd = ui->editRollCount->text().toInt();
    const int dcEnd = ui->editDiceCount->text().toInt();
    const int diceType = ui->editDiceType->text().toInt();
    const int target = ui->editTarget->text().toInt();
    const int bonus = ui->editBonus->text().toInt();
    int total = 0;

    _pendingResultStrings.clear();
    _pendingTotalString.clear();

    QVector<int> firstRollDieValues;

    for(int rc = 0; rc < rcEnd; ++rc)
    {
        int result = 0;
        QString resultStr;

        // Go through and roll the dice, building up the string along the way
        for(int dc = 0; dc < dcEnd; ++dc)
        {
            int roll = Dice::dX(diceType);
            if(dc > 0)
                resultStr.append(QString(" + "));

            resultStr.append(QString::number(roll));
            result += roll;

            if(rc == 0)
                firstRollDieValues.append(roll);
        }

        // Add the bonus number, if it exists
        if(bonus > 0)
        {
            resultStr.append(QString(" + ") + QString::number(bonus));
            result += bonus;
        }

        // If there was somehow more than one number shown, then we should bother showing the overall sum
        if((dcEnd > 1) || (bonus > 0))
            resultStr.append(QString(" = ") + QString::number(result));

        // Set the text color based on whether or not we exceeded the target
        if(result >= target)
            resultStr.prepend(QString("<font color=""#00ff00"">"));
        else
            resultStr.prepend(QString("<font color=""#ff0000"">"));

        resultStr.append(QString("</font>\n"));

        total += result;

        _pendingResultStrings.append(resultStr);
    }

    _pendingTotalString = QString("<b><font color=""#000000"">Total: ") + QString::number(total) + QString("</font></b>\n");

    const bool canAnimate = DiceAnimationWidget::animationEnabled()
                            && _diceAnim
                            && !firstRollDieValues.isEmpty()
                            && diceType > 0;

    if(canAnimate)
    {
        ui->widgetDiceAnim->show();
        _diceAnim->rollDice(diceType, firstRollDieValues);
    }
    else
    {
        flushPendingResults();
    }
}

void DiceRollFrame::onAnimationFinished()
{
    flushPendingResults();
}

void DiceRollFrame::init()
{
    QValidator *valRollCount = new QIntValidator(1, 100, this);
    ui->editRollCount->setValidator(valRollCount);
    QValidator *valDiceCount = new QIntValidator(1, 100, this);
    ui->editDiceCount->setValidator(valDiceCount);
    QValidator *valDiceType = new QIntValidator(1, 100, this);
    ui->editDiceType->setValidator(valDiceType);
    QValidator *valBonus = new QIntValidator(0, 100, this);
    ui->editBonus->setValidator(valBonus);
    QValidator *valTarget = new QIntValidator(0, 100, this);
    ui->editTarget->setValidator(valTarget);

    connect(ui->btnRoll, SIGNAL(clicked()), this, SLOT(rollDice()));

    if(DiceAnimationWidget::animationEnabled())
    {
        _diceAnim = new DiceAnimationWidget(ui->widgetDiceAnim);
        QHBoxLayout *animLayout = new QHBoxLayout(ui->widgetDiceAnim);
        animLayout->setContentsMargins(0, 0, 0, 0);
        animLayout->addWidget(_diceAnim);
        connect(_diceAnim, &DiceAnimationWidget::animationFinished,
                this, &DiceRollFrame::onAnimationFinished);
    }
    else
    {
        ui->widgetDiceAnim->hide();
    }
}

void DiceRollFrame::flushPendingResults()
{
    for(const QString &s : _pendingResultStrings)
        ui->editResult->append(s);

    if(!_pendingTotalString.isEmpty())
        ui->editResult->append(_pendingTotalString);

    _pendingResultStrings.clear();
    _pendingTotalString.clear();
}
