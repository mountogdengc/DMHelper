#ifndef LAYERDRAWTOOLDIALOG_H
#define LAYERDRAWTOOLDIALOG_H

#include <QDialog>

namespace Ui {
class LayerDrawToolDialog;
}

class LayerDrawToolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerDrawToolDialog(QWidget *parent = nullptr);
    ~LayerDrawToolDialog();

private:
    Ui::LayerDrawToolDialog *ui;
};

#endif // LAYERDRAWTOOLDIALOG_H
