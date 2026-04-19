#ifndef DICEROLLFRAME_H
#define DICEROLLFRAME_H

#include <QFrame>
#include <QStringList>
#include "dice.h"

namespace Ui {
class DiceRollFrame;
}

class DiceAnimationWidget;

class DiceRollFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DiceRollFrame(QWidget *parent = nullptr);
    explicit DiceRollFrame(const Dice& dice, QWidget *parent = nullptr);
    ~DiceRollFrame();

public slots:
    void rollDice();

private slots:
    void onAnimationFinished();

private:
    void init();
    void flushPendingResults();

    Ui::DiceRollFrame *ui;
    DiceAnimationWidget *_diceAnim;
    QStringList _pendingResultStrings;
    QString _pendingTotalString;
};

#endif // DICEROLLFRAME_H
