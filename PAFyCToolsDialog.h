#ifndef PAFYCTOOLSDIALOG_H
#define PAFYCTOOLSDIALOG_H

#include <QDialog>

namespace Ui {
class PAFyCToolsDialog;
}

class PAFyCToolsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PAFyCToolsDialog(QWidget *parent = nullptr);
    ~PAFyCToolsDialog();

private:
    Ui::PAFyCToolsDialog *ui;
};

#endif // PAFYCTOOLSDIALOG_H
