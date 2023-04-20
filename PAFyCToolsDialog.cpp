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
    if(mPtrParametersManager!=NULL)
    {
        delete(mPtrParametersManager);
        mPtrParametersManager=NULL;
    }
//    if(mPtrProgressExternalProcessDialog!=NULL)
//    {
//        delete(mPtrProgressExternalProcessDialog);
//        mPtrProgressExternalProcessDialog=NULL;
//    }
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
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PP);
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PL);

    ui->commandComboBox->addItem(PAFYCTOOLSGUI_NO_COMBO_SELECT);
    for(int i=0;i<mCommands.size();i++)
    {
        ui->commandComboBox->addItem(mCommands[i]);
    }
    ui->subCommandComboBox->addItem(PAFYCTOOLSGUI_NO_COMBO_SELECT);
    ui->subCommandComboBox->setEnabled(false);

    return(true);
}

bool PAFyCToolsDialog::process_plppc_pp(QString &qgisPath,
                                        QString &outputPath,
                                        QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_PLPPC_PP;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,pointCloudFileName,roiShapefileName,inputPath,latsShapefile,parameterCode,lastoolsPath,lidarFilesPath;
    int epsgCode;
    int intValue;
    double dblValue;
    bool okToNumber;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_TAG_POINT_CLOUD_FILE;
    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    strValue=strValue.trimmed();
    if(!QFile::exists(strValue))
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nnot exists file:\n%2")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    pointCloudFileName=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_TAG_ROI_SHAPEFILE;
    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    strValue=strValue.trimmed();
    if(!QFile::exists(strValue))
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nnot exists file:\n%2")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    roiShapefileName=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_TAG_CRS_EPSG_CODE;
    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    strValue=strValue.trimmed();
    int crsEpsgCode,verticalCrsEpsgCode=-1;
    if(strValue.contains("+"))
    {
        QStringList strCrsValues=strValue.split("+");
        if(strCrsValues.size()!=2)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nInvalid value for compound crs: %1\nin file:\n%2")
                    .arg(parameterCode).arg(strValue).arg(mPtrParametersManager->getFileName());
            return(false);
        }
        strValue=strCrsValues[0].trimmed();
        okToNumber=false;
        intValue=strValue.toInt(&okToNumber);
        if(!okToNumber)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        crsEpsgCode=intValue;
        strValue=strCrsValues[1].trimmed();
        okToNumber=false;
        intValue=strValue.toInt(&okToNumber);
        if(!okToNumber)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        verticalCrsEpsgCode=intValue;
    }
    else
    {
        okToNumber=false;
        intValue=strValue.toInt(&okToNumber);
        if(!okToNumber)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        crsEpsgCode=intValue;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_TAG_LASTOOLS_PATH;
    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    strValue=strValue.trimmed();
    lastoolsPath=strValue;
    if(!auxDir.exists(lastoolsPath))
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nnot exists path:\n%2")
                .arg(parameterCode).arg(lastoolsPath);
        return(false);
    }

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PROCESS_FILE;
    if(QFile::exists(processFileName))
    {
        if(!QFile::remove(processFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nHa fallado la eliminación del fichero:\n%1").arg(processFileName);
            return(false);
        }
    }
    QFile file(processFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nHa fallado la apertura del fichero:\n%1").arg(processFileName);
        return(false);
    }
    QTextStream strOut(&file);
/*
echo off
set PATH=%PATH%;C:\lastools2022\bin;
set PROCESS_PATH=D:\MetashapeProjects\20220426_Tarazona_Vid_A6000\process
set INPUT_POINT_CLOUD_FILE=D:\MetashapeProjects\20220426_Tarazona_Vid_A6000\export\20220426_Tarazona_vid_hElip.laz
set INPUT_ROI_SHAPEFILE=D:\MetashapeProjects\20220426_Tarazona_Vid_A6000\roi\roi_25830.shp
set OUTPUT_POINT_CLOUD_CLIPPED_FILE=%PROCESS_PATH%\20220426_Tarazona_vid_hElip_clipped.laz
set TILE_SIZE=10
set TILE_BUFFER=1
set GROUND_STEP_SIZE=0.1
set CORES=8
set LASTHIN_ADAPTATIVE_2D=0.2
set LASTHIN_ADAPTATIVE_H=0.2
set LASNOISE_STEP_2D=0.05
set LASNOISE_STEP_H=0.05
set LASNOISE_MINIMUN_NUMBER_OF_POINTS=10
set VINE_H_IGNORE_FOOT=0.1
set VINE_H_TRUNK_MINIMUM_HEIGHT=0.1
set VINE_H_TRUNK_MAXIMUM_HEIGHT=0.4
set VINE_H_ARMS_MINIMUM_HEIGHT=0.4
set VINE_H_ARMS_MAXIMUM_HEIGHT=1.2
set VINE_RASTER_STEP=0.05
set VINE_RASTER_FILE_NAME=vines.tif
set VINE_RASTER_TRUNK_STEP=0.05
set VINE_RASTER_TRUNK_FILE_NAME=vines_trunk.tif
REM lasclip64 -v -i %INPUT_POINT_CLOUD_FILE% ^
          REM -poly %INPUT_ROI_SHAPEFILE% ^
          REM -o %OUTPUT_POINT_CLOUD_CLIPPED_FILE%
:: create temporary tile directory
rmdir temp_tiles /s /q
mkdir temp_tiles
:: create a temporary and reversible tiling with tile size 500 and buffer 50
lastile -v -i %OUTPUT_POINT_CLOUD_CLIPPED_FILE% ^
        -reversible -tile_size %TILE_SIZE% -buffer %TILE_BUFFER% ^
        -o temp_tiles\tile.laz -olaz
:: create another temporary tile directory
rmdir temp_tiles_ground /s /q
mkdir temp_tiles_ground
:: run lasground on all temporary tiles on 3 cores
lasground_new -v -i temp_tiles\tile*.laz ^
          -step %GROUND_STEP_SIZE% -coarse ^
          -odir temp_tiles_ground -olaz ^
          -cores %CORES%
:: delete temporary tile directory
rmdir temp_tiles /s /q
rmdir temp_tiles_ground_thinned /s /q
mkdir temp_tiles_ground_thinned
:: run lasthin on all temporary tiles on 3 cores
lasthin64 -v -i temp_tiles_ground\tile*.laz ^
          -ignore_class 1 -adaptive %LASTHIN_ADAPTATIVE_2D% %LASTHIN_ADAPTATIVE_H% ^
          -odir temp_tiles_ground_thinned -olaz ^
          -cores %CORES%
:: delete temporary tile directory
rmdir temp_tiles_ground /s /q
rmdir temp_tiles_ground_thinned_denoise /s /q
mkdir temp_tiles_ground_thinned_denoise
:: run lasthin on all temporary tiles on 3 cores
lasnoise64 -v -i temp_tiles_ground_thinned\tile*.laz ^
          -ignore_class 2 -step_xy %LASNOISE_STEP_2D% -step_z %LASNOISE_STEP_H% -remove_noise -isolated %LASNOISE_MINIMUN_NUMBER_OF_POINTS% ^
          -odir temp_tiles_ground_thinned_denoise -olaz ^
          -cores %CORES%
:: delete temporary tile directory
rmdir temp_tiles_ground_thinned /s /q
rmdir temp_tiles_ground_thinned_denoise_height /s /q
mkdir temp_tiles_ground_thinned_denoise_height
:: run lasthin on all temporary tiles on 3 cores
lasheight64 -v -i temp_tiles_ground_thinned_denoise\tile*.laz ^
          -ignore_class 2 -classify_below %VINE_H_IGNORE_FOOT% 0 ^
          -classify_between %VINE_H_TRUNK_MINIMUM_HEIGHT% %VINE_H_TRUNK_MAXIMUM_HEIGHT% 3 ^
          -classify_between %VINE_H_ARMS_MINIMUM_HEIGHT% %VINE_H_ARMS_MAXIMUM_HEIGHT% 4 ^
          -classify_above %VINE_H_ARMS_MAXIMUM_HEIGHT% 7 ^
          -odir temp_tiles_ground_thinned_denoise_height -olaz ^
          -cores %CORES%
:: recreate ground classified huge LAS / LAZ file
rmdir temp_tiles_ground_thinned_denoise /s /q
lastile -i temp_tiles_ground_thinned_denoise_height\tile*.laz ^
        -reverse_tiling ^
        -o %OUTPUT_POINT_CLOUD_CLIPPED_FILE% -odix _ground -olaz
:: delete other temporary tile directory
rmdir temp_tiles_ground_thinned_denoise_height /s /q
lasgrid64 -v -i *_ground.laz ^
          -keep_class 3 4 -step %VINE_RASTER_STEP% -counter_16bit ^
          -o %VINE_RASTER_FILE_NAME%
lasgrid64 -v -i *_ground.laz ^
          -keep_class 3 -step %VINE_RASTER_TRUNK_STEP% -counter_16bit ^
          -o %VINE_RASTER_TRUNK_FILE_NAME%
*/
    strOut<<"echo off"<<"\n";
    strOut<<"set LASTOOLS_PATH="<<lastoolsPath<<"\n";
    strOut<<"set PNOA_LIDAR_FILES_PATH="<<lidarFilesPath<<"\n";


    file.close();
    QStringList parameters;
    mStrExecution=processFileName;
    if(mPtrProgressExternalProcessDialog==NULL)
    {
        mPtrProgressExternalProcessDialog=new ProcessTools::ProgressExternalProcessDialog(true,this);
        mPtrProgressExternalProcessDialog->setAutoCloseWhenFinish(false);
    }
    mPtrProgressExternalProcessDialog->setDialogTitle(command);
//    connect(mPtrProgressExternalProcessDialog, SIGNAL(dialog_closed()),this,SLOT(on_ProgressExternalProcessDialog_closed()));

    mInitialDateTime=QDateTime::currentDateTime();
    mProgressExternalProcessTitle=command;
    mPtrProgressExternalProcessDialog->runExternalProcess(mStrExecution,parameters,mBasePath);
    return(true);
}

bool PAFyCToolsDialog::process_plppc_pl(QString &qgisPath,
                                        QString &outputPath,
                                        QString &strError)
{

    return(true);
}

bool PAFyCToolsDialog::removeDir(QString dirName, bool onlyContent)
{
    bool result = true;
    QDir dir(dirName);
    if (dir.exists(dirName))
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir())
            {
                result = removeDir(info.absoluteFilePath());
            }
            else
            {
                result = QFile::remove(info.absoluteFilePath());
            }
            if (!result)
            {
                return result;
            }
        }
        if(!onlyContent)
        {
            result = dir.rmdir(dirName);
        }
    }
    return result;
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
    if(command.compare(PAFYCTOOLSGUI_COMMAND_PLPPC_PP,Qt::CaseInsensitive)==0)
    {
        pdfFileName+=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_HELP_PDF_FILE;
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_PLPPC_PL,Qt::CaseInsensitive)==0)
    {
        pdfFileName+=PAFYCTOOLSGUI_COMMAND_PLPPC_PL_HELP_PDF_FILE;
    }
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
    if(command.compare(PAFYCTOOLSGUI_COMMAND_PLPPC_PP,Qt::CaseInsensitive)==0)
    {
        if(!process_plppc_pp(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_PLPPC_PL,Qt::CaseInsensitive)==0)
    {
        if(!process_plppc_pl(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

