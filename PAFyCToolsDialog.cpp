#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>

#include <ParametersManager.h>
#include <Parameter.h>
#include <ParametersManagerDialog.h>
#include <ProgressExternalProcessDialog.h>

//#include <quazip.h>
//#include <JlCompress.h>

#include "PAFyCToolsGuiDefinitions.h"
#include "PAFyCToolsDialog.h"
#include "ui_PAFyCToolsDialog.h"

PAFyCToolsDialog::PAFyCToolsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PAFyCToolsDialog)
{
    mPtrParametersManager=NULL;
    mPtrProgressExternalProcessDialog=NULL;
    ui->setupUi(this);
    QString strError;
    if(!initialize(strError))
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Error:\n%1").arg(strError);
        msg+=QObject::tr("\n\nAuthor emails:\n%1\n%2")
                .arg(PAFYCTOOLSGUI_AUTHOR_EMAIL).arg(PAFYCTOOLSGUI_AUTHOR_EMAIL2);
        QMessageBox::information(this,title,msg);
        exit(1);
    }
}

PAFyCToolsDialog::~PAFyCToolsDialog()
{
    delete ui;
}

bool PAFyCToolsDialog::initialize(QString &strError)
{
    QString functionName="initialize";
    mBasePath=QDir::currentPath();

    QString settingsFileName=QDir::currentPath()+"/"+PAFYCTOOLSGUI_SETTINGS_FILE_NAME;
    mPtrSettings=new QSettings(settingsFileName,QSettings::IniFormat,this);
    mLastPath=mPtrSettings->value(PAFYCTOOLSGUI_SETTINGS_TAG_LAST_PATH).toString();
    if(mLastPath.isEmpty())
    {
        mLastPath=mBasePath;
        mPtrSettings->setValue(PAFYCTOOLSGUI_SETTINGS_TAG_LAST_PATH,mLastPath);
        mPtrSettings->sync();
    }
    QString qgisPath=mPtrSettings->value(PAFYCTOOLSGUI_SETTINGS_TAG_QGIS_PATH).toString();
    if(!qgisPath.isEmpty())
    {
        if(!QFile::exists(qgisPath))
        {
            strError=functionName;
            strError+=QObject::tr("\nNot exists QGIS path:\n%1").arg(qgisPath);
            mPtrSettings->setValue(PAFYCTOOLSGUI_SETTINGS_TAG_QGIS_PATH,"");
            mPtrSettings->sync();
            return(false);
        }
        QString qgisSetEnvironmentBatFileName=qgisPath+PAFYCTOOLSGUI_QGIS_BAT_ENVIRONMENT;
        if(!QFile::exists(qgisSetEnvironmentBatFileName))
        {
            strError=functionName;
            strError+=QObject::tr("Not exists QGIS environment file:\n%1").arg(qgisSetEnvironmentBatFileName);
            mPtrSettings->setValue(PAFYCTOOLSGUI_SETTINGS_TAG_QGIS_PATH,"");
            mPtrSettings->sync();
            return(false);
        }
        ui->qgisPathLineEdit->setText(qgisPath);
    }
    QString outputPath=mPtrSettings->value(PAFYCTOOLSGUI_SETTINGS_TAG_OUTPUT_PATH).toString();
    if(!outputPath.isEmpty())
    {
        if(!QFile::exists(outputPath))
        {
            strError=functionName;
            strError+=QObject::tr("\nNot exists output path:\n%1").arg(outputPath);
            mPtrSettings->setValue(PAFYCTOOLSGUI_SETTINGS_TAG_OUTPUT_PATH,"");
            mPtrSettings->sync();
            return(false);
        }
        ui->outputPathLineEdit->setText(outputPath);
    }

    QString parametersFileName=mBasePath+"/"+PAFYCTOOLSGUI_PARAMETERS_FILE_NAME;
    if(!QFile::exists(parametersFileName))
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameters file:\n%1").arg(parametersFileName);
        return(false);
    }
    mPtrParametersManager=new ParametersManager();
    QString strAuxError;
    if(!mPtrParametersManager->loadFromXml(parametersFileName,strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nLoading parameters file:\n%1\nerror:\n%2")
                .arg(parametersFileName).arg(strAuxError);
        delete(mPtrParametersManager);
        mPtrParametersManager=NULL;
        return(false);
    }

    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_PLPPC);
//    mCommands.push_back();
    QVector<QString> aux1;
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC]=aux1;
//    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back();
//    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back();

    ui->commandComboBox->addItem(PAFYCTOOLSGUI_NO_COMBO_SELECT);
    for(int i=0;i<mCommands.size();i++)
    {
        ui->commandComboBox->addItem(mCommands[i]);
    }
    ui->subCommandComboBox->addItem(PAFYCTOOLSGUI_NO_COMBO_SELECT);
    ui->subCommandComboBox->setEnabled(false);

    return(true);
}

void PAFyCToolsDialog::on_qgisPathPushButton_clicked()
{
    QString functionName="on_qgisPathPushButton_clicked";
    QString path=ui->qgisPathLineEdit->text();
    if(path.isEmpty())
    {
        path=mLastPath;
    }
    QString strDir = QFileDialog::getExistingDirectory(this,
                                                     tr("Select QGIS path"),
                                                     path,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if(!strDir.isEmpty())
    {
        QString qgisSetEnvironmentBatFileName=strDir+PAFYCTOOLSGUI_QGIS_BAT_ENVIRONMENT;
        if(!QFile::exists(qgisSetEnvironmentBatFileName))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Not exists QGIS environment file:\n%1").arg(qgisSetEnvironmentBatFileName);
            msg+=QObject::tr("\n\nAuthor emails:\n%1\n%2")
                    .arg(PAFYCTOOLSGUI_AUTHOR_EMAIL).arg(PAFYCTOOLSGUI_AUTHOR_EMAIL2);
            QMessageBox::information(this,title,msg);
            return;
        }
        mLastPath=strDir;
        mPtrSettings->setValue(PAFYCTOOLSGUI_SETTINGS_TAG_QGIS_PATH,mLastPath);
        mPtrSettings->sync();
        ui->qgisPathLineEdit->setText(strDir);
    }
}


void PAFyCToolsDialog::on_outputPathPushButton_clicked()
{
    QString functionName="on_outputPathPushButton_clicked";
    QString path=ui->outputPathLineEdit->text();
    if(path.isEmpty())
    {
        path=mLastPath;
    }
    QString strDir = QFileDialog::getExistingDirectory(this,
                                                     tr("Select output path"),
                                                     path,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if(!strDir.isEmpty())
    {
        mLastPath=strDir;
        mPtrSettings->setValue(PAFYCTOOLSGUI_SETTINGS_TAG_OUTPUT_PATH,mLastPath);
        mPtrSettings->sync();
        ui->outputPathLineEdit->setText(strDir);
    }
}


void PAFyCToolsDialog::on_commandComboBox_currentIndexChanged(int index)
{
    QString functionName="on_commandComboBox_activated";
    QString command=ui->commandComboBox->currentText();
    ui->subCommandComboBox->clear();
    ui->subCommandComboBox->addItem(PAFYCTOOLSGUI_NO_COMBO_SELECT);
    ui->subCommandComboBox->setEnabled(false);
    if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)!=0)
    {
        if(mSubCommandsByCommand.contains(command))
        {
            for(int i=0;i<mSubCommandsByCommand[command].size();i++)
            {
                ui->subCommandComboBox->addItem(mSubCommandsByCommand[command][i]);
            }
            ui->subCommandComboBox->setEnabled(true);
        }
    }
}


void PAFyCToolsDialog::on_helpPushButton_clicked()
{
    QString functionName="on_helpPushButton_clicked";
    QString command=ui->commandComboBox->currentText();
    if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Select command before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString mainCommand=command;
    if(mSubCommandsByCommand.contains(command))
    {
        command=ui->subCommandComboBox->currentText();
        if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
//                &&mainCommand.compare(CAMTIMA_COMMAND_DRIL,Qt::CaseInsensitive)!=0
//                &&mainCommand.compare(CAMTIMA_COMMAND_DRILATS,Qt::CaseInsensitive)!=0)
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Select subcommand before");
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    QString pdfFileName=qApp->applicationDirPath() + PAFYCTOOLSGUI_DOCS_PATH;
//    if(command.compare(CAMTIMA_COMMAND_DPT,Qt::CaseInsensitive)==0)
//    {
//        pdfFileName+=CAMTIMA_COMMAND_DPT_HELP_PDF_FILE;
//    }
//    else if(command.compare(CAMTIMA_COMMAND_DMH,Qt::CaseInsensitive)==0)
//    {
//        pdfFileName+=CAMTIMA_COMMAND_DMH_HELP_PDF_FILE;
//    }
    if(QFile::exists(pdfFileName))
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(pdfFileName));
    }
    else
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Not exists file:\n%1").arg(pdfFileName);
        QMessageBox::information(this,title,msg);
        return;
    }
}


void PAFyCToolsDialog::on_parametersPushButton_clicked()
{
    QString functionName="on_parametersPushButton_clicked";
    QString command=ui->commandComboBox->currentText();
    if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Select command before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if(mSubCommandsByCommand.contains(command))
    {
        command=ui->subCommandComboBox->currentText();
        if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Select subcommand before");
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    ParametersManagerDialog parameterDialog(mPtrParametersManager,
                                            command);
}

void PAFyCToolsDialog::on_ProgressExternalProcessDialog_closed()
{
    QDateTime finalDateTime=QDateTime::currentDateTime();
    mPtrProgressExternalProcessDialog->setAutoCloseWhenFinish(false);
    disconnect(mPtrProgressExternalProcessDialog, SIGNAL(dialog_closed()),this,SLOT(on_ProgressExternalProcessDialog_closed()));
    if(mPtrProgressExternalProcessDialog->getState()==PROCESSDIALOG_STATE_ERROR)
    {
        QString title=mProgressExternalProcessTitle;
        QString msg=QObject::tr("Error ejecutando:\n");
        msg+=mStrExecution;
        QMessageBox::information(this,title,msg);
        return;
    }
    if(mPtrProgressExternalProcessDialog->getState()==PROCESSDIALOG_STATE_CANCELED)
    {
        QString title=mProgressExternalProcessTitle;
        QString msg=QObject::tr("Proceso cancelado:\n");
        msg+=mStrExecution;
        QMessageBox::information(new QWidget(),title,msg);
        return;
    }
    /*
    QString msg="Proceso finalizado";
    int initialSeconds=(int)mInitialDateTime.toTime_t();
    int finalSeconds=(int)finalDateTime.toTime_t();
    int totalDurationSeconds=finalSeconds-initialSeconds;
    double dblTotalDurationSeconds=(double)totalDurationSeconds;
    int durationDays=(int)floor(dblTotalDurationSeconds/60.0/60.0/24.0);
    int durationHours=(int)floor((dblTotalDurationSeconds-durationDays*60.0*60.0*24.0)/60.0/60.0);
    int durationMinutes=(int)floor((dblTotalDurationSeconds-durationDays*60.0*60.0*24.0-durationHours*60.0*60.0)/60.0);
    int durationSeconds=dblTotalDurationSeconds-durationDays*60.0*60.0*24.0-durationHours*60.0*60.0-durationMinutes*60.0;
    {
        QString msgTtime="- Duración del proceso:\n";
        msgTtime+="  - Hora inicial del proceso .......................: ";
        msgTtime+=mInitialDateTime.toString("yyyy/MM/dd - hh/mm/ss.zzz");
        msgTtime+="\n";
        msgTtime+="  - Hora final del proceso .........................: ";
        msgTtime+=finalDateTime.toString("yyyy/MM/dd - hh/mm/ss.zzz");
        msgTtime+="\n";
        msgTtime+="  - Número total de segundos .......................: ";
        msgTtime+=QString::number(dblTotalDurationSeconds,'f',3);
        msgTtime+="\n";
        msgTtime+="    - Número de días ...............................: ";
        msgTtime+=QString::number(durationDays);
        msgTtime+="\n";
        msgTtime+="    - Número de horas ..............................: ";
        msgTtime+=QString::number(durationHours);
        msgTtime+="\n";
        msgTtime+="    - Número de minutos ............................: ";
        msgTtime+=QString::number(durationMinutes);
        msgTtime+="\n";
        msgTtime+="    - Número de segundos ...........................: ";
        msgTtime+=QString::number(durationSeconds,'f',3);
        msgTtime+="\n";
        msg+=msgTtime;
    }
    QString title=mProgressExternalProcessTitle;
    QMessageBox::information(new QWidget(),title,msg);
    */
    /*
    // compresion
    if(mResultFiles.size()>0)
    {
        QDir auxDir=QDir::currentPath();
        QString resultsPath;
        QString resultsZipPath;
        QVector<QString> lostFiles;
        for(int i=0;i<mResultFiles.size();i++)
        {
            QString resultFileName=mResultFiles[i];
            if(!QFile::exists(resultFileName))
            {
                lostFiles.push_back(resultFileName);
                continue;
            }
            if(resultsZipPath.isEmpty())
            {
                resultsZipPath=QFileInfo(resultFileName).absolutePath();
                resultsPath=resultsZipPath;
                resultsZipPath+="/";
                resultsZipPath+=CAMTIMA_RESULTS_PATH;
                if(auxDir.exists(resultsZipPath))
                {
                    if(!removeDir(resultsZipPath))
                    {
                        QString title=mProgressExternalProcessTitle;
                        QString msg=QObject::tr("Error eliminando la carpeta para comprimir los resultados:\n");
                        msg+=resultsZipPath;
                        QMessageBox::information(this,title,msg);
                        return;
                    }
                }
                if(!auxDir.mkpath(resultsZipPath))
                {
                    QString title=mProgressExternalProcessTitle;
                    QString msg=QObject::tr("Error creando la carpeta para comprimir los resultados:\n");
                    msg+=resultsZipPath;
                    QMessageBox::information(this,title,msg);
                    return;
                }
            }
            QString targetResultFileName=resultsZipPath+"/"+QFileInfo(resultFileName).fileName();
            if(!QFile::copy(resultFileName,targetResultFileName))
            {
                QString title=mProgressExternalProcessTitle;
                QString msg=QObject::tr("Error copiando el fichero:\n%1\na la carpeta para comprimir los resultados:\n").arg(resultFileName);
                msg+=resultsZipPath;
                QMessageBox::information(this,title,msg);
                return;
            }
        }
        QString zipFileName=resultsPath+"/"+CAMTIMA_RESULTS_ZIP_FILE_NAME;
        if(!JlCompress::compressDir(zipFileName,
                                    resultsZipPath))
        {
            QString title=mProgressExternalProcessTitle;
            QString msg=QObject::tr("Error creando el fichero comprimido:\n");
            msg+=zipFileName;
            QMessageBox::information(this,title,msg);
            return;
        }
        if(!removeDir(resultsZipPath))
        {
            QString title=mProgressExternalProcessTitle;
            QString msg=QObject::tr("Error eliminando la carpeta comprimida de resultados:\n");
            msg+=resultsZipPath;
            QMessageBox::information(this,title,msg);
            return;
        }
        if(lostFiles.size()>0)
        {
            QString title=mProgressExternalProcessTitle;
            QString msg=QObject::tr("No se han encontrado los siguientes ficheros:");
            for(int i=0;i<lostFiles.size();i++)
            {
                msg+="\n";
                msg+=lostFiles[i];
            }
            QMessageBox::information(this,title,msg);
        }
        mResultFiles.clear();
    }
    */
}


void PAFyCToolsDialog::on_processPushButton_clicked()
{
    QString functionName="on_processPushButton_clicked";
    QString qgisPath=ui->qgisPathLineEdit->text();
    if(qgisPath.isEmpty())
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Select QGIS path before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString qgisSetEnvironmentBatFileName=qgisPath+PAFYCTOOLSGUI_QGIS_BAT_ENVIRONMENT;
    if(!QFile::exists(qgisSetEnvironmentBatFileName))
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Not exists QGIS environmet file:\n%1").arg(qgisSetEnvironmentBatFileName);
        msg+=QObject::tr("\n\nAuthor emails:\n%1\n%2")
                .arg(PAFYCTOOLSGUI_AUTHOR_EMAIL).arg(PAFYCTOOLSGUI_AUTHOR_EMAIL2);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString outputPath=ui->outputPathLineEdit->text();
    if(outputPath.isEmpty())
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Seleccione primero la carpeta de salida");
        QMessageBox::information(this,title,msg);
        return;
    }
    if(QDir(outputPath).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() > 0)
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QMessageBox msgBox(this);
        msgBox.setText(title);
        msgBox.setInformativeText("Exists files in output path.\nDo you want replace it?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Ok:
            return;
            break;
        case QMessageBox::Discard:
            // Don't Save was clicked
            break;
        case QMessageBox::Cancel:
            // Don't Save was clicked
            return;
            break;
        default:
            // should never be reached
            break;
        }
    }

    QDir auxDir=QDir::currentPath();
    if(!auxDir.exists(outputPath))
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Not exists output path:\n%1").arg(outputPath);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString command=ui->commandComboBox->currentText();
    if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("Select command before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if(mSubCommandsByCommand.contains(command))
    {
        command=ui->subCommandComboBox->currentText();
        if(command.compare(PAFYCTOOLSGUI_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Select subcommand before");
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    QString strAuxError;
//    if(command.compare(CAMTIMA_COMMAND_DPT,Qt::CaseInsensitive)==0)
//    {
//        if(!process_dpt(qgisPath,outputPath,strAuxError))
//        {
//            QString title=PAFYCTOOLSGUI_TITLE;
//            QString msg=QObject::tr("En el proceso:\n%1\nse ha producido el error:\n%2")
//                    .arg(command).arg(strAuxError);
//            QMessageBox::information(this,title,msg);
//            return;
//        }
//    }
//    else if(command.compare(CAMTIMA_COMMAND_DMH,Qt::CaseInsensitive)==0)
//    {
//        if(!process_dmh(qgisPath,outputPath,strAuxError))
//        {
//            QString title=CAMTIMA_TITLE;
//            QString msg=QObject::tr("En el proceso:\n%1\nse ha producido el error:\n%2")
//                    .arg(command).arg(strAuxError);
//            QMessageBox::information(this,title,msg);
//            return;
//        }
//    }
}

