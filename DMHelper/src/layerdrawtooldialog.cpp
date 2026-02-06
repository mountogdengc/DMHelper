#include "layerdrawtooldialog.h"
#include "ui_layerdrawtooldialog.h"

LayerDrawToolDialog::LayerDrawToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerDrawToolDialog)
{
    ui->setupUi(this);
}

LayerDrawToolDialog::~LayerDrawToolDialog()
{
    delete ui;
}
