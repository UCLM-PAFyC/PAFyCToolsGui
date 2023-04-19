#include "PAFyCToolsDialog.h"
#include "ui_PAFyCToolsDialog.h"

PAFyCToolsDialog::PAFyCToolsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PAFyCToolsDialog)
{
    ui->setupUi(this);
}

PAFyCToolsDialog::~PAFyCToolsDialog()
{
    delete ui;
}
