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
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_PAFyCToolsDialog
{
public:
    QGridLayout *gridLayout_3;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QPushButton *qgisPathPushButton;
    QLineEdit *qgisPathLineEdit;
    QPushButton *outputPathPushButton;
    QLineEdit *outputPathLineEdit;
    QLabel *label;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QComboBox *commandComboBox;
    QLabel *label_3;
    QComboBox *subCommandComboBox;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *helpPushButton;
    QPushButton *parametersPushButton;
    QPushButton *processPushButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *PAFyCToolsDialog)
    {
        if (PAFyCToolsDialog->objectName().isEmpty())
            PAFyCToolsDialog->setObjectName(QString::fromUtf8("PAFyCToolsDialog"));
        PAFyCToolsDialog->resize(984, 434);
        QIcon icon;
        icon.addFile(QString::fromUtf8("logos/pafyc.ico"), QSize(), QIcon::Normal, QIcon::Off);
        PAFyCToolsDialog->setWindowIcon(icon);
        gridLayout_3 = new QGridLayout(PAFyCToolsDialog);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        qgisPathPushButton = new QPushButton(PAFyCToolsDialog);
        qgisPathPushButton->setObjectName(QString::fromUtf8("qgisPathPushButton"));

        gridLayout->addWidget(qgisPathPushButton, 0, 0, 1, 1);

        qgisPathLineEdit = new QLineEdit(PAFyCToolsDialog);
        qgisPathLineEdit->setObjectName(QString::fromUtf8("qgisPathLineEdit"));
        qgisPathLineEdit->setReadOnly(true);

        gridLayout->addWidget(qgisPathLineEdit, 0, 1, 1, 1);

        outputPathPushButton = new QPushButton(PAFyCToolsDialog);
        outputPathPushButton->setObjectName(QString::fromUtf8("outputPathPushButton"));
        outputPathPushButton->setMinimumSize(QSize(200, 0));

        gridLayout->addWidget(outputPathPushButton, 1, 0, 1, 1);

        outputPathLineEdit = new QLineEdit(PAFyCToolsDialog);
        outputPathLineEdit->setObjectName(QString::fromUtf8("outputPathLineEdit"));
        outputPathLineEdit->setReadOnly(true);

        gridLayout->addWidget(outputPathLineEdit, 1, 1, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 2);

        label = new QLabel(PAFyCToolsDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(400, 340));
        label->setPixmap(QPixmap(QString::fromUtf8(":/logos/logos/PAFyCToolsGui_400.png")));

        gridLayout_2->addWidget(label, 1, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_2 = new QLabel(PAFyCToolsDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout->addWidget(label_2);

        commandComboBox = new QComboBox(PAFyCToolsDialog);
        commandComboBox->setObjectName(QString::fromUtf8("commandComboBox"));
        commandComboBox->setMinimumSize(QSize(550, 0));

        verticalLayout->addWidget(commandComboBox);

        label_3 = new QLabel(PAFyCToolsDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout->addWidget(label_3);

        subCommandComboBox = new QComboBox(PAFyCToolsDialog);
        subCommandComboBox->setObjectName(QString::fromUtf8("subCommandComboBox"));

        verticalLayout->addWidget(subCommandComboBox);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(208, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        helpPushButton = new QPushButton(PAFyCToolsDialog);
        helpPushButton->setObjectName(QString::fromUtf8("helpPushButton"));
        helpPushButton->setMaximumSize(QSize(60, 16777215));

        horizontalLayout->addWidget(helpPushButton);

        parametersPushButton = new QPushButton(PAFyCToolsDialog);
        parametersPushButton->setObjectName(QString::fromUtf8("parametersPushButton"));

        horizontalLayout->addWidget(parametersPushButton);

        processPushButton = new QPushButton(PAFyCToolsDialog);
        processPushButton->setObjectName(QString::fromUtf8("processPushButton"));

        horizontalLayout->addWidget(processPushButton);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 118, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        gridLayout_2->addLayout(verticalLayout, 1, 1, 1, 1);


        gridLayout_3->addLayout(gridLayout_2, 0, 0, 1, 1);


        retranslateUi(PAFyCToolsDialog);

        QMetaObject::connectSlotsByName(PAFyCToolsDialog);
    } // setupUi

    void retranslateUi(QDialog *PAFyCToolsDialog)
    {
        PAFyCToolsDialog->setWindowTitle(QCoreApplication::translate("PAFyCToolsDialog", "PAFyC Tools GUI", nullptr));
        qgisPathPushButton->setText(QCoreApplication::translate("PAFyCToolsDialog", "QGIS path:", nullptr));
        outputPathPushButton->setText(QCoreApplication::translate("PAFyCToolsDialog", "Output path:", nullptr));
        label->setText(QString());
        label_2->setText(QCoreApplication::translate("PAFyCToolsDialog", "Select command:", nullptr));
        label_3->setText(QCoreApplication::translate("PAFyCToolsDialog", "Select subcommand:", nullptr));
        helpPushButton->setText(QCoreApplication::translate("PAFyCToolsDialog", "Help", nullptr));
        parametersPushButton->setText(QCoreApplication::translate("PAFyCToolsDialog", "Select parameters", nullptr));
        processPushButton->setText(QCoreApplication::translate("PAFyCToolsDialog", "Process", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PAFyCToolsDialog: public Ui_PAFyCToolsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAFYCTOOLSDIALOG_H
