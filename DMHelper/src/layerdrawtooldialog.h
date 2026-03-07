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

    Qt::PenStyle getToolLineType() const;
    QColor getToolLineColor() const;
    int getToolLineWidth() const;

private:
    Ui::LayerDrawToolDialog *ui;
};

class LayerDrawToolDialog_LineTypeAction : public QAction
{
    Q_OBJECT

public:
    explicit LayerDrawToolDialog_LineTypeAction(const QIcon &icon, const QString &text, int lineType, QObject *parent = nullptr);
    virtual ~LayerDrawToolDialog_LineTypeAction() override;

    int getLineType() const;

protected:
    int _lineType;
};


#endif // LAYERDRAWTOOLDIALOG_H
