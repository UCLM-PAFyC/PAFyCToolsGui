#include "PAFyCToolsDialog.h".h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>

#define IPYPROJECT_AUTHOR_MAIL               "david.hernandez@uclm.es"

#define IPYPROJECT_DATE_FORMAT               "yyyy:MM:dd"

#define IPYPROJECT_LICENSE_INITIAL_DATE      "2016:01:01"
#define IPYPROJECT_LICENSE_FINAL_DATE        "2024:01:31"

#include <ILicenseManager.h>
void initialize(bool &isInitialized,
                QString pythonModulePath);
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    bool isInitialized=false;
    QString pythonModulePath=QDir::currentPath(); // carpeta libCpp del plugin
    initialize(isInitialized,pythonModulePath);
    if(!isInitialized)
    {
        QString title="PAFyCToolsConsole";
        QString msg=QObject::tr("License error");
        msg+=QObject::tr("\nContact the author:\n");
        msg+=IPYPROJECT_AUTHOR_MAIL;
        QMessageBox::information(new QWidget(),title,msg);
//        PyErr_SetString(cppExceptionTypeObj, msg.toStdString().c_str());
        exit(0);
    }
    PAFyCToolsDialog w;
    w.show();
    return a.exec();
}

void initialize(bool &isInitialized,
                QString pythonModulePath)
{
    QDate initialLicenseDate=QDate::fromString(IPYPROJECT_LICENSE_INITIAL_DATE,IPYPROJECT_DATE_FORMAT);
    QDate finalLicenseDate=QDate::fromString(IPYPROJECT_LICENSE_FINAL_DATE,IPYPROJECT_DATE_FORMAT);
    bool checkKey=false;
    bool checkDate=false;
    isInitialized=true;
    if(checkKey||checkDate)
    {
        QString contactMail=IPYPROJECT_AUTHOR_MAIL;
        int waittingTimeMiliseconds=5000;
        ILicenseManager ilm(checkKey,
                            checkDate,
                            initialLicenseDate,
                            finalLicenseDate,
                            waittingTimeMiliseconds,
                            contactMail,
                            pythonModulePath);
        bool validKey=ilm.getValidKey();
        bool validDate=ilm.getValidDate();
        bool licensed=ilm.getLicensed();
        if(checkDate&&!validDate)
        {
            isInitialized=false;
//            QString title="MainWindowICOM3DUAV::MainWindowICOM3DUAV";
//            QString msg=tr("No se ha reconocido la validez de la licencia temporal");
//            msg+=tr("\nCompruebe la conexion a internet");
//            msg+=tr("\nPara obtener una licencia contacte con:\n%1").arg(MAINWINDOW_CONTACT_MAIL);
//            QMessageBox::information(this,title,msg);
//            exit(1);
        }
        if(checkKey&&!validKey)
        {
//            QString title="MainWindowICOM3DUAV::MainWindowICOM3DUAV";
//            QString msg=tr("No se ha reconocido la validez de la licencia");
//            msg+=tr("\nPara obtener una licencia contacte con:\n%1").arg(MAINWINDOW_CONTACT_MAIL);
//            QMessageBox::information(this,title,msg);
//            exit(1);
            isInitialized=false;
        }
    }
}
