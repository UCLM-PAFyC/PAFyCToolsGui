/********************************************************************************
** Form generated from reading UI file 'PAFyCToolsDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAFYCTOOLSDIALOG_H
#define UI_PAFYCTOOLSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_PAFyCToolsDialog
{
public:

    void setupUi(QDialog *PAFyCToolsDialog)
    {
        if (PAFyCToolsDialog->objectName().isEmpty())
            PAFyCToolsDialog->setObjectName(QString::fromUtf8("PAFyCToolsDialog"));
        PAFyCToolsDialog->resize(865, 541);

        retranslateUi(PAFyCToolsDialog);

        QMetaObject::connectSlotsByName(PAFyCToolsDialog);
    } // setupUi

    void retranslateUi(QDialog *PAFyCToolsDialog)
    {
        PAFyCToolsDialog->setWindowTitle(QCoreApplication::translate("PAFyCToolsDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PAFyCToolsDialog: public Ui_PAFyCToolsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAFYCTOOLSDIALOG_H
