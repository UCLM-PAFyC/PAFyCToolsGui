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
//#include "../../libs/libModelManagementTools/ModelDbManagerDefinitions.h"


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
    if(mPtrParametersManagerModelManagementCommands!=NULL)
    {
        delete(mPtrParametersManagerModelManagementCommands);
        mPtrParametersManagerModelManagementCommands=NULL;
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

    QString parametersModelManagementFileName=mBasePath+"/"+PAFYCTOOLSGUI_PARAMETERS_MODELDBMANAGERDEFINITIONS_PROJECT_TYPE_PAFYCTOOLS_PARAMETERS_FILE_NAME;
    if(!QFile::exists(parametersModelManagementFileName))
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameters file:\n%1").arg(parametersModelManagementFileName);
        return(false);
    }
    mPtrParametersManagerModelManagementCommands=new ParametersManager();
    if(!mPtrParametersManagerModelManagementCommands->loadFromXml(parametersModelManagementFileName,strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nLoading parameters file:\n%1\nerror:\n%2")
                .arg(parametersModelManagementFileName).arg(strAuxError);
        delete(mPtrParametersManagerModelManagementCommands);
        mPtrParametersManagerModelManagementCommands=NULL;
        return(false);
    }

    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_PLPPC);
//    mCommands.push_back();
    QVector<QString> aux1;
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC]=aux1;
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PP);
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PL);
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PF);
    QVector<QString> aux2;
    mModelManagementCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC_PL]=aux2;
    mModelManagementCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC_PL].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PL_MODELMANAGEMET_FIRST_COMMAND);
    QVector<QString> aux3;
    mModelManagementCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC_PF]=aux3;
    mModelManagementCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC_PF].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PF_MODELMANAGEMET_FIRST_COMMAND);
    mModelManagementCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC_PF].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PF_MODELMANAGEMET_SECOND_COMMAND);

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
    QString lasinfoFileName=lastoolsPath+"/"+PAFYCTOOLSGUI_LASINFO_FILE;
    if(!QFile::exists(lasinfoFileName))
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nnot exists file:\n%2")
                .arg(parameterCode).arg(lasinfoFileName);
        strError+=QObject::tr("\nWrong LAStools path:\n%1")
                .arg(lastoolsPath);
        return(false);
    }

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PROCESS_FILE;
    {
        if(QFile::exists(processFileName))
        {
            if(!QFile::remove(processFileName))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileName);
                return(false);
            }
        }
        QFile file(processFileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileName);
            return(false);
        }
        QTextStream strOut(&file);
        strOut<<"echo off"<<"\n";
        strOut<<"set OUTPUT_PATH="<<outputPath<<"\n";
        strOut<<"call \"%OUTPUT_PATH%/"<<PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PROCESS_FILE_LASTOOLS<<"\"\n";
        strOut<<"call \"%OUTPUT_PATH%/"<<PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PROCESS_FILE_GDAL<<"\"\n";
        file.close();
    }

    QString processFileNameLastools=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PROCESS_FILE_LASTOOLS;
    {
        if(QFile::exists(processFileNameLastools))
        {
            if(!QFile::remove(processFileNameLastools))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileNameLastools);
                return(false);
            }
        }
        QFile file(processFileNameLastools);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileNameLastools);
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
        set OUTPUT_DTM_FILE=%PROCESS_PATH%\20220426_Tarazona_vid_hElip_dtm_50cm.tif
        set STEP=0.5
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
        strOut<<"set PATH=%PATH%;\""<<lastoolsPath<<"\"\n";
        strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
        strOut<<"set INPUT_POINT_CLOUD_FILE="<<pointCloudFileName<<"\n";
        strOut<<"set INPUT_ROI_SHAPEFILE="<<roiShapefileName<<"\n";
        //    QString pointCloudClippedFileName=QFileInfo(pointCloudFileName).absolutePath();
        QString pointCloudClippedFileName=outputPath;
        pointCloudClippedFileName+="/";
        QString pointCloudClippedFileNameWithoutPath=QFileInfo(pointCloudFileName).completeBaseName();
        pointCloudClippedFileNameWithoutPath+="_clipped.laz";
        pointCloudClippedFileName+=pointCloudClippedFileNameWithoutPath;
        if(QFile::exists(pointCloudClippedFileName))
        {
            if(!QFile::remove(pointCloudClippedFileName))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(pointCloudClippedFileName);
                file.close();
                return(false);
            }
        }
        strOut<<"set OUTPUT_POINT_CLOUD_CLIPPED_FILE=%PROCESS_PATH%\\"<<pointCloudClippedFileNameWithoutPath<<"\n";
        strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
        strOut<<"set TILE_SIZE=10"<<"\n";
        strOut<<"set TILE_BUFFER=1"<<"\n";
        strOut<<"set GROUND_STEP_SIZE=0.1"<<"\n";
        strOut<<"set CORES=8"<<"\n";
        strOut<<"set LASTHIN_ADAPTATIVE_2D=0.2"<<"\n";
        strOut<<"set LASTHIN_ADAPTATIVE_H=0.2"<<"\n";
        strOut<<"set LASNOISE_STEP_2D=0.05"<<"\n";
        strOut<<"set LASNOISE_STEP_H=0.05"<<"\n";
        strOut<<"set LASNOISE_MINIMUN_NUMBER_OF_POINTS=10"<<"\n";
        strOut<<"set VINE_H_IGNORE_FOOT=0.1"<<"\n";
        strOut<<"set VINE_H_TRUNK_MINIMUM_HEIGHT=0.1"<<"\n";
        strOut<<"set VINE_H_TRUNK_MAXIMUM_HEIGHT=0.4"<<"\n";
        strOut<<"set VINE_H_ARMS_MINIMUM_HEIGHT=0.4"<<"\n";
        strOut<<"set VINE_H_ARMS_MAXIMUM_HEIGHT=1.2"<<"\n";
        strOut<<"set VINE_RASTER_STEP=0.05"<<"\n";
        strOut<<"set VINE_RASTER_FILE_NAME=vines.tif"<<"\n";
        strOut<<"set VINE_RASTER_TRUNK_STEP=0.05"<<"\n";
        strOut<<"set VINE_RASTER_TRUNK_FILE_NAME=vines_trunk.tif"<<"\n";
        strOut<<"set DTM_STEP=0.10"<<"\n";
        QString dtmFileName=QFileInfo(pointCloudFileName).completeBaseName();
        dtmFileName+="_dtm_10cm.tif";
        strOut<<"set OUTPUT_DTM_FILE=\""<<dtmFileName<<"\"\n";
        strOut<<"lasclip64 -v -i \"%INPUT_POINT_CLOUD_FILE%\" ^"<<"\n";
        strOut<<"          -poly \"%INPUT_ROI_SHAPEFILE%\" ^"<<"\n";
        strOut<<"          -o \"%OUTPUT_POINT_CLOUD_CLIPPED_FILE%\""<<"\n";
        strOut<<"rmdir temp_tiles /s /q"<<"\n";
        strOut<<"mkdir temp_tiles"<<"\n";
        strOut<<"lastile -v -i \"%OUTPUT_POINT_CLOUD_CLIPPED_FILE%\" ^"<<"\n";
        strOut<<"        -reversible -tile_size %TILE_SIZE% -buffer %TILE_BUFFER% ^"<<"\n";
        strOut<<"        -o temp_tiles\\tile.laz -olaz"<<"\n";
        strOut<<"rmdir temp_tiles_ground /s /q"<<"\n";
        strOut<<"mkdir temp_tiles_ground"<<"\n";
        strOut<<"lasground_new -v -i temp_tiles\\tile*.laz ^"<<"\n";
        strOut<<"          -step %GROUND_STEP_SIZE% -coarse ^"<<"\n";
        strOut<<"          -odir temp_tiles_ground -olaz ^"<<"\n";
        strOut<<"          -cores %CORES%"<<"\n";
        strOut<<"rmdir temp_tiles /s /q"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned /s /q"<<"\n";
        strOut<<"mkdir temp_tiles_ground_thinned"<<"\n";
        strOut<<"lasthin64 -v -i temp_tiles_ground\\tile*.laz ^"<<"\n";
        strOut<<"          -ignore_class 1 -adaptive %LASTHIN_ADAPTATIVE_2D% %LASTHIN_ADAPTATIVE_H% ^"<<"\n";
        strOut<<"          -odir temp_tiles_ground_thinned -olaz ^"<<"\n";
        strOut<<"          -cores %CORES%"<<"\n";
        strOut<<"rmdir temp_tiles_ground /s /q"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned_denoise /s /q"<<"\n";
        strOut<<"mkdir temp_tiles_ground_thinned_denoise"<<"\n";
        strOut<<"lasnoise64 -v -i temp_tiles_ground_thinned\\tile*.laz ^"<<"\n";
        strOut<<"          -ignore_class 2 -step_xy %LASNOISE_STEP_2D% -step_z %LASNOISE_STEP_H% -remove_noise -isolated %LASNOISE_MINIMUN_NUMBER_OF_POINTS% ^"<<"\n";
        strOut<<"          -odir temp_tiles_ground_thinned_denoise -olaz ^"<<"\n";
        strOut<<"          -cores %CORES%"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned /s /q"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned_denoise_height /s /q"<<"\n";
        strOut<<"mkdir temp_tiles_ground_thinned_denoise_height"<<"\n";
        strOut<<"lasheight64 -v -i temp_tiles_ground_thinned_denoise\\tile*.laz ^"<<"\n";
        strOut<<"          -ignore_class 2 -classify_below %VINE_H_IGNORE_FOOT% 0 ^"<<"\n";
        strOut<<"          -classify_between %VINE_H_TRUNK_MINIMUM_HEIGHT% %VINE_H_TRUNK_MAXIMUM_HEIGHT% 3 ^"<<"\n";
        strOut<<"          -classify_between %VINE_H_ARMS_MINIMUM_HEIGHT% %VINE_H_ARMS_MAXIMUM_HEIGHT% 4 ^"<<"\n";
        strOut<<"          -classify_above %VINE_H_ARMS_MAXIMUM_HEIGHT% 7 ^"<<"\n";
        strOut<<"          -odir temp_tiles_ground_thinned_denoise_height -olaz ^"<<"\n";
        strOut<<"          -cores %CORES%"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned_denoise /s /q"<<"\n";
        strOut<<"lastile -i temp_tiles_ground_thinned_denoise_height\\tile*.laz ^"<<"\n";
        strOut<<"        -reverse_tiling ^"<<"\n";
        strOut<<"        -o \"%OUTPUT_POINT_CLOUD_CLIPPED_FILE%\" -odix _ground -olaz"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned_denoise_height /s /q"<<"\n";
        strOut<<"lasgrid64 -v -i *_ground.laz ^"<<"\n";
        strOut<<"          -keep_class 3 4 -step %VINE_RASTER_STEP% -counter_16bit ^"<<"\n";
        strOut<<"          -o %VINE_RASTER_FILE_NAME%"<<"\n";
        strOut<<"lasgrid64 -v -i *_ground.laz ^"<<"\n";
        strOut<<"          -keep_class 3 -step %VINE_RASTER_TRUNK_STEP% -counter_16bit ^"<<"\n";
        strOut<<"          -o %VINE_RASTER_TRUNK_FILE_NAME%"<<"\n";
        strOut<<"las2dem64 -v -i *_ground.laz ^"<<"\n";
        strOut<<"          -step %STEP% -keep_class 2 ^"<<"\n";
        strOut<<"          -o %OUTPUT_DTM_FILE%"<<"\n";
        file.close();
    }

    QString processFileNameGdal=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PROCESS_FILE_GDAL;
    {
        if(QFile::exists(processFileNameGdal))
        {
            if(!QFile::remove(processFileNameGdal))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileNameGdal);
                return(false);
            }
        }
        QFile file(processFileNameGdal);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileNameGdal);
            return(false);
        }
        QTextStream strOut(&file);
        /*
        echo off
        set OSGEO4W_ROOT=C:\Program Files\QGIS 3.22.3
        set PROCESS_PATH=D:\MetashapeProjects\20220426_Tarazona_Vid_A6000\process
        set INPUT_RASTER_FILE=%PROCESS_PATH%\vines.tif
        set OUTPUT_SHAPEFILE_PIXELS=%PROCESS_PATH%\vines_pixels.shp
        set OUTPUT_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\vines_contours_1.shp
        set OUTPUT_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\vines_centroids_1.shp
        set OUTPUT_SHAPEFILE_CONTOURS=%PROCESS_PATH%\vines_contours.shp
        set OUTPUT_SHAPEFILE_CENTROIDS=%PROCESS_PATH%\vines_centroids.shp
        set DELETE_SHAPEFILE_PIXELS=%PROCESS_PATH%\vines_pixels.*
        set DELETE_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\vines_contours_1.*
        set DELETE_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\vines_centroids_1.*
        set INPUT_TRUNK_RASTER_FILE=%PROCESS_PATH%\vines_trunk.tif
        set OUTPUT_TRUNK_SHAPEFILE_PIXELS=%PROCESS_PATH%\vines_trunk_pixels.shp
        set OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\vines_trunk_contours_1.shp
        set OUTPUT_TRUNK_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\vines_trunk_centroids_1.shp
        set OUTPUT_TRUNK_SHAPEFILE_CONTOURS=%PROCESS_PATH%\vines_trunk_contours.shp
        set OUTPUT_TRUNK_SHAPEFILE_CENTROIDS=%PROCESS_PATH%\vines_trunk_centroids.shp
        set DELETE_TRUNK_SHAPEFILE_PIXELS=%PROCESS_PATH%\vines_trunk_pixels.*
        set DELETE_TRUNK_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\vines_trunk_contours_1.*
        set DELETE_TRUNK_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\vines_trunk_centroids_1.*
        REM set DELETE_TRUNK_SHAPEFILE_AUX=%PROCESS_PATH%\OUTPUT.*
        set FIELD_ID_INITIAL=FID
        set FIELD_ID_FINAL=id
        call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
        python "%OSGEO4W_ROOT%\apps\Python39\Scripts\gdal_polygonize.py" %INPUT_RASTER_FILE% -b 1 -f "ESRI Shapefile" %OUTPUT_SHAPEFILE_PIXELS% OUTPUT DN
        ogr2ogr %OUTPUT_SHAPEFILE_CONTOURS_1% %OUTPUT_SHAPEFILE_PIXELS% -dialect sqlite -sql "SELECT ST_Union(geometry) FROM vines_pixels" -explodecollections
        ogr2ogr %OUTPUT_SHAPEFILE_CENTROIDS_1% %OUTPUT_SHAPEFILE_CONTOURS_1% -dialect sqlite -sql "SELECT ST_Centroid(geometry) from vines_contours_1"
        ogr2ogr %OUTPUT_SHAPEFILE_CONTOURS% %OUTPUT_SHAPEFILE_CONTOURS_1% -sql "SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_contours_1"
        ogrinfo %OUTPUT_SHAPEFILE_CONTOURS% -sql "alter table vines_contours add column enabled integer"
        ogrinfo %OUTPUT_SHAPEFILE_CONTOURS% -dialect SQLite -sql "update vines_contours set enabled=1"
        ogr2ogr %OUTPUT_SHAPEFILE_CENTROIDS% %OUTPUT_SHAPEFILE_CENTROIDS_1% -sql "SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_centroids_1"
        ogrinfo %OUTPUT_SHAPEFILE_CENTROIDS% -sql "alter table vines_centroids add column enabled integer"
        ogrinfo %OUTPUT_SHAPEFILE_CENTROIDS% -dialect SQLite -sql "update vines_centroids set enabled=1"
        del "%DELETE_SHAPEFILE_PIXELS%"
        del "%DELETE_SHAPEFILE_CONTOURS_1%"
        del "%DELETE_SHAPEFILE_CENTROIDS_1%"
        python "%OSGEO4W_ROOT%\apps\Python39\Scripts\gdal_polygonize.py" %INPUT_TRUNK_RASTER_FILE% -b 1 -f "ESRI Shapefile" %OUTPUT_TRUNK_SHAPEFILE_PIXELS% OUTPUT DN
        ogr2ogr %OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1% %OUTPUT_TRUNK_SHAPEFILE_PIXELS% -dialect sqlite -sql "SELECT ST_Union(geometry) FROM vines_trunk_pixels" -explodecollections
        ogr2ogr %OUTPUT_TRUNK_SHAPEFILE_CENTROIDS_1% %OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1% -dialect sqlite -sql "SELECT ST_Centroid(geometry) from vines_trunk_contours_1"
        ogr2ogr %OUTPUT_TRUNK_SHAPEFILE_CONTOURS% %OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1% -sql "SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_trunk_contours_1"
        ogrinfo %OUTPUT_TRUNK_SHAPEFILE_CONTOURS% -sql "alter table vines_trunk_contours add column enabled integer"
        ogrinfo %OUTPUT_TRUNK_SHAPEFILE_CONTOURS% -dialect SQLite -sql "update vines_trunk_contours set enabled=1"
        ogr2ogr %OUTPUT_TRUNK_SHAPEFILE_CENTROIDS% %OUTPUT_TRUNK_SHAPEFILE_CENTROIDS_1% -sql "SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_trunk_centroids_1"
        ogrinfo %OUTPUT_TRUNK_SHAPEFILE_CENTROIDS% -sql "alter table vines_trunk_centroids add column enabled integer"
        ogrinfo %OUTPUT_TRUNK_SHAPEFILE_CENTROIDS% -dialect SQLite -sql "update vines_trunk_centroids set enabled=1"
        del "%DELETE_TRUNK_SHAPEFILE_PIXELS%"
        del "%DELETE_TRUNK_SHAPEFILE_CONTOURS_1%"
        del "%DELETE_TRUNK_SHAPEFILE_CENTROIDS_1%"
        del "%DELETE_TRUNK_SHAPEFILE_AUX%"
        */
        strOut<<"echo off"<<"\n";
        strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
        strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
        strOut<<"set INPUT_RASTER_FILE=%PROCESS_PATH%\\vines.tif"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_PIXELS=%PROCESS_PATH%\\vines_pixels.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\\vines_contours_1.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\\vines_centroids_1.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_CONTOURS=%PROCESS_PATH%\\vines_contours.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_CENTROIDS=%PROCESS_PATH%\\vines_centroids.shp"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_PIXELS=%PROCESS_PATH%\\vines_pixels.*"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\\vines_contours_1.*"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\\vines_centroids_1.*"<<"\n";
        strOut<<"set INPUT_TRUNK_RASTER_FILE=%PROCESS_PATH%\\vines_trunk.tif"<<"\n";
        strOut<<"set OUTPUT_TRUNK_SHAPEFILE_PIXELS=%PROCESS_PATH%\\vines_trunk_pixels.shp"<<"\n";
        strOut<<"set OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\\vines_trunk_contours_1.shp"<<"\n";
        strOut<<"set OUTPUT_TRUNK_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\\vines_trunk_centroids_1.shp"<<"\n";
        strOut<<"set OUTPUT_TRUNK_SHAPEFILE_CONTOURS=%PROCESS_PATH%\\vines_trunk_contours.shp"<<"\n";
        strOut<<"set OUTPUT_TRUNK_SHAPEFILE_CENTROIDS=%PROCESS_PATH%\\vines_trunk_centroids.shp"<<"\n";
        strOut<<"set DELETE_TRUNK_SHAPEFILE_PIXELS=%PROCESS_PATH%\\vines_trunk_pixels.*"<<"\n";
        strOut<<"set DELETE_TRUNK_SHAPEFILE_CONTOURS_1=%PROCESS_PATH%\\vines_trunk_contours_1.*"<<"\n";
        strOut<<"set DELETE_TRUNK_SHAPEFILE_CENTROIDS_1=%PROCESS_PATH%\\vines_trunk_centroids_1.*"<<"\n";
        strOut<<"REM set DELETE_TRUNK_SHAPEFILE_AUX=%PROCESS_PATH%\\OUTPUT.*"<<"\n";
        strOut<<"set FIELD_ID_INITIAL=FID"<<"\n";
        strOut<<"set FIELD_ID_FINAL=id"<<"\n";
        strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
        strOut<<"python \"%OSGEO4W_ROOT%\\apps\\Python39\\Scripts\\gdal_polygonize.py\" \"%INPUT_RASTER_FILE%\" -b 1 -f \"ESRI Shapefile\" \"%OUTPUT_SHAPEFILE_PIXELS%\" OUTPUT DN"<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_SHAPEFILE_CONTOURS_1%\" \"%OUTPUT_SHAPEFILE_PIXELS%\" -dialect sqlite -sql \"SELECT ST_Union(geometry) FROM vines_pixels\" -explodecollections"<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_SHAPEFILE_CENTROIDS_1%\" \"%OUTPUT_SHAPEFILE_CONTOURS_1%\" -dialect sqlite -sql \"SELECT ST_Centroid(geometry) from vines_contours_1\""<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_SHAPEFILE_CONTOURS%\" \"%OUTPUT_SHAPEFILE_CONTOURS_1%\" -sql \"SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_contours_1\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_SHAPEFILE_CONTOURS%\" -sql \"alter table vines_contours add column enabled integer\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_SHAPEFILE_CONTOURS%\" -dialect SQLite -sql \"update vines_contours set enabled=1\""<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_SHAPEFILE_CENTROIDS%\" \"%OUTPUT_SHAPEFILE_CENTROIDS_1%\" -sql \"SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_centroids_1\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_SHAPEFILE_CENTROIDS%\" -sql \"alter table vines_centroids add column enabled integer\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_SHAPEFILE_CENTROIDS%\" -dialect SQLite -sql \"update vines_centroids set enabled=1\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_PIXELS%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_CONTOURS_1%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_CENTROIDS_1%\""<<"\n";
        strOut<<"python \"%OSGEO4W_ROOT%\\apps\\Python39\\Scripts\\gdal_polygonize.py\" \"%INPUT_TRUNK_RASTER_FILE%\" -b 1 -f \"ESRI Shapefile\" \"%OUTPUT_TRUNK_SHAPEFILE_PIXELS%\" OUTPUT DN"<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1%\" \"%OUTPUT_TRUNK_SHAPEFILE_PIXELS%\" -dialect sqlite -sql \"SELECT ST_Union(geometry) FROM vines_trunk_pixels\" -explodecollections"<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_TRUNK_SHAPEFILE_CENTROIDS_1%\" \"%OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1%\" -dialect sqlite -sql \"SELECT ST_Centroid(geometry) from vines_trunk_contours_1\""<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_TRUNK_SHAPEFILE_CONTOURS%\" \"%OUTPUT_TRUNK_SHAPEFILE_CONTOURS_1%\" -sql \"SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_trunk_contours_1\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_TRUNK_SHAPEFILE_CONTOURS%\" -sql \"alter table vines_trunk_contours add column enabled integer\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_TRUNK_SHAPEFILE_CONTOURS%\" -dialect SQLite -sql \"update vines_trunk_contours set enabled=1\""<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_TRUNK_SHAPEFILE_CENTROIDS%\" \"%OUTPUT_TRUNK_SHAPEFILE_CENTROIDS_1%\" -sql \"SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_trunk_centroids_1\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_TRUNK_SHAPEFILE_CENTROIDS%\" -sql \"alter table vines_trunk_centroids add column enabled integer\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_TRUNK_SHAPEFILE_CENTROIDS%\" -dialect SQLite -sql \"update vines_trunk_centroids set enabled=1\""<<"\n";
        strOut<<"del \"%DELETE_TRUNK_SHAPEFILE_PIXELS%\""<<"\n";
        strOut<<"del \"%DELETE_TRUNK_SHAPEFILE_CONTOURS_1%\""<<"\n";
        strOut<<"del \"%DELETE_TRUNK_SHAPEFILE_CENTROIDS_1%\""<<"\n";
        strOut<<"del \"%DELETE_TRUNK_SHAPEFILE_AUX%\""<<"\n";
        file.close();
    }

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
    QString command=PAFYCTOOLSGUI_COMMAND_PLPPC_PL;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,roiShapefileName,inputPath,latsShapefile,parameterCode,lastoolsPath,lidarFilesPath;
    int intValue;
    double dblValue;
    bool okToNumber;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();
    QString programPath=auxDir.absolutePath();
    QString programFile=auxDir.absolutePath()+"/"+PAFYCTOOLSGUI_PROGRAM_CONSOLE_FILE_NAME;
    if(!QFile::exists(programFile))
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists program file:\n%1")
                .arg(programFile);
        return(false);
    }

//    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PL_TAG_ROI_SHAPEFILE;
//    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
//    if(ptrParameter==NULL)
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
//                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
//        return(false);
//    }
//    ptrParameter->getValue(strValue);
//    strValue=strValue.trimmed();
//    if(!QFile::exists(strValue))
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nFor parameter: %1\nnot exists file:\n%2")
//                .arg(parameterCode).arg(strValue);
//        return(false);
//    }
//    roiShapefileName=strValue;

//    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PL_TAG_CRS_EPSG_CODE;
//    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
//    if(ptrParameter==NULL)
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
//                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
//        return(false);
//    }
//    ptrParameter->getValue(strValue);
//    strValue=strValue.trimmed();
//    int crsEpsgCode,verticalCrsEpsgCode=-1;
//    if(strValue.contains("+"))
//    {
//        QStringList strCrsValues=strValue.split("+");
//        if(strCrsValues.size()!=2)
//        {
//            strError=functionName;
//            strError+=QObject::tr("\nFor parameter: %1\nInvalid value for compound crs: %1\nin file:\n%2")
//                    .arg(parameterCode).arg(strValue).arg(mPtrParametersManager->getFileName());
//            return(false);
//        }
//        strValue=strCrsValues[0].trimmed();
//        okToNumber=false;
//        intValue=strValue.toInt(&okToNumber);
//        if(!okToNumber)
//        {
//            strError=functionName;
//            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
//                    .arg(parameterCode).arg(strValue);
//            return(false);
//        }
//        crsEpsgCode=intValue;
//        strValue=strCrsValues[1].trimmed();
//        okToNumber=false;
//        intValue=strValue.toInt(&okToNumber);
//        if(!okToNumber)
//        {
//            strError=functionName;
//            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
//                    .arg(parameterCode).arg(strValue);
//            return(false);
//        }
//        verticalCrsEpsgCode=intValue;
//    }
//    else
//    {
//        okToNumber=false;
//        intValue=strValue.toInt(&okToNumber);
//        if(!okToNumber)
//        {
//            strError=functionName;
//            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
//                    .arg(parameterCode).arg(strValue);
//            return(false);
//        }
//        crsEpsgCode=intValue;
//    }

//    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PL_TAG_PROJECT_ID;
//    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
//    if(ptrParameter==NULL)
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
//                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
//        return(false);
//    }
//    ptrParameter->getValue(strValue);
//    strValue=strValue.trimmed();
//    if(strValue.split(" ").size()>1)
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nExists spaces in value: %1\nfor parameter: %2 in file:\n%3")
//                .arg(strValue).arg(parameterCode).arg(mPtrParametersManager->getFileName());
//        return(false);
//    }
//    QString projectId=strValue;

//    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PL_TAG_INPUT_PATH;
//    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
//    if(ptrParameter==NULL)
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
//                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
//        return(false);
//    }
//    ptrParameter->getValue(strValue);
//    strValue=strValue.trimmed();
//    if(!auxDir.exists(strValue))
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nNot exists path:\n%1\for parameter: %2 in file:\n%3")
//                .arg(strValue).arg(parameterCode).arg(mPtrParametersManager->getFileName());
//        return(false);
//    }
//    QString inputPath=strValue;

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PL_PROCESS_FILE;
    if(QFile::exists(processFileName))
    {
        if(!QFile::remove(processFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nError removing file:\n%1").arg(processFileName);
            return(false);
        }
    }
    QFile file(processFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening file:\n%1").arg(processFileName);
        return(false);
    }
    QTextStream strOut(&file);
    strOut<<"echo off"<<"\n";
    strOut<<"set OUTPUT_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"cd /d \""<<programPath<<"\"\n";
//    strOut<<"SET PATH=%PATH%;\""<<QFileInfo(programFile).absolutePath()<<"\""<<"\n";
//    strOut<<"\""<<programFile<<"\" ";
    strOut<<QFileInfo(programFile).fileName()<<" ";
    strOut<<"\""<<command<<"\" ";
    strOut<<"\""<<qgisPath<<"\" ";
    strOut<<"\""<<outputPath<<"\"\n";
    file.close();

    QStringList parameters;
    mStrExecution=processFileName;
    if(mPtrProgressExternalProcessDialog==NULL)
    {
        mPtrProgressExternalProcessDialog=new ProcessTools::ProgressExternalProcessDialog(true,this);
        mPtrProgressExternalProcessDialog->setAutoCloseWhenFinish(false);
    }
    mPtrProgressExternalProcessDialog->setDialogTitle(command);
    connect(mPtrProgressExternalProcessDialog, SIGNAL(dialog_closed()),this,SLOT(on_ProgressExternalProcessDialog_closed()));

    mInitialDateTime=QDateTime::currentDateTime();
    mProgressExternalProcessTitle=command;
    mPtrProgressExternalProcessDialog->runExternalProcess(mStrExecution,parameters,mBasePath);

    return(true);
}

bool PAFyCToolsDialog::process_plppc_pf(QString &qgisPath,
                                        QString &outputPath,
                                        QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_PLPPC_PF;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,roiShapefileName,inputPath,latsShapefile,parameterCode,lastoolsPath,lidarFilesPath;
    int intValue;
    double dblValue;
    bool okToNumber;
    Parameter* ptrParameter=NULL;
    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PF_TAG_PROJECT_ID;
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
    if(strValue.split(" ").size()>1)
    {
        strError=functionName;
        strError+=QObject::tr("\nExists spaces in project id: %1 in file:\n%2")
                .arg(strValue).arg(mPtrParametersManager->getFileName());
        return(false);
    }
    QString projectId=strValue;
    QString dbFileName=outputPath+"/PAFyCToolsGui_";
    dbFileName+=projectId;
    dbFileName+=".sqlite";
    if(!QFile::exists(dbFileName))
    {
        strError=functionName;
        strError=QObject::tr("\nNot exists project database file:\nror:\n%1")
                .arg(dbFileName);
        return(false);
    }
    QDir auxDir=QDir::currentPath();
    QString programPath=auxDir.absolutePath();
    QString programFile=auxDir.absolutePath()+"/"+PAFYCTOOLSGUI_PROGRAM_CONSOLE_FILE_NAME;
    if(!QFile::exists(programFile))
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists program file:\n%1")
                .arg(programFile);
        return(false);
    }
    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PF_PROCESS_FILE;
    if(QFile::exists(processFileName))
    {
        if(!QFile::remove(processFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nError removing file:\n%1").arg(processFileName);
            return(false);
        }
    }
    QFile file(processFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening file:\n%1").arg(processFileName);
        return(false);
    }
    QTextStream strOut(&file);
    strOut<<"echo off"<<"\n";
    strOut<<"set OUTPUT_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"cd /d \""<<programPath<<"\"\n";
//    strOut<<"SET PATH=%PATH%;\""<<QFileInfo(programFile).absolutePath()<<"\""<<"\n";
//    strOut<<"\""<<programFile<<"\" ";
    strOut<<QFileInfo(programFile).fileName()<<" ";
    strOut<<"\""<<command<<"\" ";
    strOut<<"\""<<qgisPath<<"\" ";
    strOut<<"\""<<outputPath<<"\"\n";
    QString outputFramesShapefile="vines_frames.shp";
//    ogr2ogr -f "ESRI Shapefile" "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames.shp" -sql "select objects.object_id as id, objects_bb3d.the_geom_base from objects, objects_bb3d where objects.id = objects_bb3d.object_id" "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\PAFyCToolsGui_20220426_Tarazona_Vid_A6000.sqlite"
    strOut<<"ogr2ogr -f \"ESRI Shapefile\" \"%OUTPUT_PATH%\\"<<outputFramesShapefile<<"\" -sql ";
    strOut<<"\"select objects.object_id as id, objects_bb3d.the_geom_base from objects, objects_bb3d where objects.id = objects_bb3d.object_id\" ";
    strOut<<"\""<<dbFileName<<"\"";
    file.close();

    QStringList parameters;
    mStrExecution=processFileName;
    if(mPtrProgressExternalProcessDialog==NULL)
    {
        mPtrProgressExternalProcessDialog=new ProcessTools::ProgressExternalProcessDialog(true,this);
        mPtrProgressExternalProcessDialog->setAutoCloseWhenFinish(false);
    }
    mPtrProgressExternalProcessDialog->setDialogTitle(command);
    connect(mPtrProgressExternalProcessDialog, SIGNAL(dialog_closed()),this,SLOT(on_ProgressExternalProcessDialog_closed()));

    mInitialDateTime=QDateTime::currentDateTime();
    mProgressExternalProcessTitle=command;
    mPtrProgressExternalProcessDialog->runExternalProcess(mStrExecution,parameters,mBasePath);
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
    if(mModelManagementCommandsByCommand.contains(command))
    {
        QVector<QString> modelManagementCommands=mModelManagementCommandsByCommand[command];
        for(int nc=0;nc<modelManagementCommands.size();nc++)
        {
            QString modelManagementCommand=modelManagementCommands[nc];
            ParametersManagerDialog parameterDialog(mPtrParametersManagerModelManagementCommands,
                                                    modelManagementCommand);
        }
    }
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
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_PLPPC_PF,Qt::CaseInsensitive)==0)
    {
        if(!process_plppc_pf(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

