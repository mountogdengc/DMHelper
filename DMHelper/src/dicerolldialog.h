#ifndef DICEROLLDIALOG_H
#define DICEROLLDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QVector>
#include "dice.h"
#include "battledialogmodel.h"

namespace Ui {
class DiceRollDialog;
}

class DiceAnimationWidget;

class DiceRollDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiceRollDialog(QWidget *parent = nullptr);
    explicit DiceRollDialog(const Dice& dice, QWidget *parent = nullptr);
    ~DiceRollDialog();

    void fireAndForget();

public slots:
    void rollDice();

protected:
    virtual void hideEvent(QHideEvent * event);

private slots:
    void onAnimationFinished();

private:
    void init();
    void rollDiceString(int& resultValue, QString& resultString, QVector<int>* captureValues, int* captureType);
    void rollDiceSpecified(int& resultValue, QString& resultString, QVector<int>* captureValues, int* captureType);
    void rollOnce(const Dice& dice, int& resultValue, QString& resultString, QVector<int>* captureValues, int* captureType);
    void flushPendingResults();

    Ui::DiceRollDialog *ui;
    DiceAnimationWidget *_diceAnim;
    QStringList _pendingResultStrings;
    QString _pendingTotalString;
    bool _fireAndForget;

    bool _mouseDown;
    QPoint _mouseDownPos;
};

#endif // DICEROLLDIALOG_H
