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
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_CCFPGP);
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

bool PAFyCToolsDialog::process_ccfpgp(QString &qgisPath,
                                      QString &outputPath,
                                      QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_CCFPGP;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,framesShapefile,dsmFile,dtmFile,outputShapefile,parameterCode,str_date;
    QString strAuxError,dateFormat,dateFromDemsFileStringSeparator;
    int intValue,dateTagPositionInDemFiles;
    double dblValue,cropsMininumHeight;
    bool okToNumber,computeGCC,computeVolume,dateFromDemFiles;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_FRAMES_SHAPEFILE;
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
    framesShapefile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DSM_FILE;
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
    dsmFile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DTM_FILE;
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
    dtmFile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_OUTPUT_SHAPEFILE;
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
    if(!strValue.isEmpty())
    {
//        if(QFile::exists(strValue))
//        {
//            strError=functionName;
//            strError+=QObject::tr("\nFor parameter: %1\nnot exists file:\n%2")
//                    .arg(parameterCode).arg(strValue);
//            return(false);
//        }
        outputShapefile=strValue;
    }
    else
    {
        outputShapefile="none";
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_CROP_MINIMUM_HEIGHT;
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
    okToNumber=false;
    dblValue=strValue.toDouble(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    cropsMininumHeight=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_COMPUTE_GCC;
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
    computeGCC=false;
    if(strValue.compare("true",Qt::CaseInsensitive)==0)
    {
        computeGCC=true;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_COMPUTE_VOLUME;
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
    computeVolume=false;
    if(strValue.compare("true",Qt::CaseInsensitive)==0)
    {
        computeVolume=true;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DATE_FROM_DEM_FILES;
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
    dateFromDemFiles=false;
    if(strValue.compare("true",Qt::CaseInsensitive)==0)
    {
        dateFromDemFiles=true;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DEM_FILES_TAGS_STRING_SEPARATOR;
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
    dateFromDemsFileStringSeparator=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DEM_FILES_TAG_DATE_POSITION;
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
    okToNumber=false;
    dateTagPositionInDemFiles=strValue.toDouble(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DATE_FORMAT;
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
    dateFormat=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CCFPGP_TAG_DATE;
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
    str_date=strValue;
    if(dateFromDemFiles)
    {
        str_date="none";
    }

    mPythonFiles.clear();
    QString pythonFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CCFPGP_PYTHON_FILE;
    if(!writePythonProgramCropsCharacterizationFromPhotogrammetricGeomaticProducts(pythonFileName,
                                                                                   strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nError writting python file:\n%1").arg(strAuxError);
        QFile::remove(pythonFileName);
        return(false);
    }
    mPythonFiles.append(pythonFileName);

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CCFPGP_PROCESS_FILE;
    if(QFile::exists(processFileName))
    {
        if(!QFile::remove(processFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nHa fallado la eliminación del fichero:\n%1").arg(processFileName);
            QFile::remove(pythonFileName);
            return(false);
        }
    }
    QFile file(processFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nHa fallado la apertura del fichero:\n%1").arg(processFileName);
        QFile::remove(pythonFileName);
        return(false);
    }
    QTextStream strOut(&file);

    /*
--input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\test\vines_frames.shp"
 --input_dsm "D:\PAFyCToolsGui\20220603_Tarazona_RGB_Vid\input\DSM-103-RGB_Tarazona_Vid_20220603_25830_2cm.tif"
 --input_dtm "D:\PAFyCToolsGui\20220603_Tarazona_RGB_Vid\input\DEM-103-RGB_Tarazona_Vid_20220603_25830_2cm.tif"
 --crop_minimum_height 0.05 --output_shp "D:\PAFyCToolsGui\2022_Tarazona.shp" --compute_GCC=1 --compute_volume=1
 --date_from_dem_files=1 --dem_files_string_separator="_" --dem_files_date_string_position=4 --date_format="%Y%m%d" --date=none

--input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\test\vines_frames.shp"
 --input_dsm "D:\PAFyCToolsGui\20220603_Tarazona_RGB_Vid\input\DSM-103-RGB_Tarazona_Vid_20220603_25830_2cm.tif"
 --input_dtm "D:\PAFyCToolsGui\20220603_Tarazona_RGB_Vid\input\DEM-103-RGB_Tarazona_Vid_20220603_25830_2cm.tif"
 --crop_minimum_height 0.05 --output_shp none --compute_GCC=1 --compute_volume=1 --date_from_dem_files=1
 --dem_files_string_separator="_" --dem_files_date_string_position=4 --date_format="%Y%m%d" --date=none
    */

    strOut<<"echo off"<<"\n";
    strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"set TOOL="<<pythonFileName<<"\n";
    strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"python %TOOL% ";
    strOut<<"--input_crops_frames_shp \""<<framesShapefile <<"\" ";
    strOut<<"--input_dsm \""<<dsmFile<<"\" ";
    strOut<<"--input_dtm \""<<dtmFile<<"\" ";
    strOut<<"--crop_minimum_height "<<QString::number(cropsMininumHeight,'f',2)<<" ";
    strOut<<"--output_shp \""<<outputShapefile<<"\" ";
    strOut<<"--compute_GCC ";
    if(computeGCC) strOut<<"1"<<" ";
    else strOut<<"0"<<" ";
    strOut<<"--compute_volume ";
    if(computeVolume) strOut<<"1"<<" ";
    else strOut<<"0"<<" ";
    strOut<<"--date_from_dem_files ";
    if(dateFromDemFiles) strOut<<"1"<<" ";
    else strOut<<"0"<<" ";
    strOut<<"--dem_files_string_separator=\""<<dateFromDemsFileStringSeparator<<"\" ";
    strOut<<"--dem_files_date_string_position="<<QString::number(dateTagPositionInDemFiles)<<" ";
    dateFormat=dateFormat.replace("%","%%");
    strOut<<"--date_format=\""<<dateFormat<<"\" ";
    if(str_date.compare("none",Qt::CaseInsensitive)==0)
    {
        strOut<<"--date="<<str_date<<"\n";
    }
    else
    {
        strOut<<"--date=\""<<str_date<<"\"\n";
    }
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
        strOut<<"          -step %DTM_STEP% -keep_class 2 ^"<<"\n";
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
        REM del "%DELETE_TRUNK_SHAPEFILE_AUX%"
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
//        strOut<<"REM set DELETE_TRUNK_SHAPEFILE_AUX=%PROCESS_PATH%\\OUTPUT.*"<<"\n";
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
//        strOut<<"del \"%DELETE_TRUNK_SHAPEFILE_AUX%\""<<"\n";
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

bool PAFyCToolsDialog::writePythonProgramCropsCharacterizationFromPhotogrammetricGeomaticProducts(QString pythonFileName,
                                                                                                  QString& strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_CCFPGP;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    functionName+=QObject::tr("Writing python file");
    if(QFile::exists(pythonFileName))
    {
        if(!QFile::remove(pythonFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nError removing file:\n%1").arg(pythonFileName);
            return(false);
        }
    }
    QFile file(pythonFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening file:\n%1").arg(pythonFileName);
        return(false);
    }
    QTextStream strOut(&file);
    strOut<<"# authors:"<<"\n";
    strOut<<"# David Hernandez Lopez, david.hernandez@uclm.es"<<"\n";
    strOut<<"# Miguel Angel Moreno Hidalgo, miguelangel.moreno@uclm.es"<<"\n";
    strOut<<"# Diego Guerrero Sevilla, diego.guerrero@uclm.es"<<"\n";
    strOut<<""<<"\n";
    strOut<<"import optparse"<<"\n";
    strOut<<"import numpy"<<"\n";
    strOut<<"from osgeo import gdal, osr, ogr"<<"\n";
    strOut<<"import os"<<"\n";
    strOut<<"import json"<<"\n";
    strOut<<"from urllib.parse import unquote"<<"\n";
    strOut<<"import shutil"<<"\n";
    strOut<<"from os.path import exists"<<"\n";
    strOut<<"import datetime"<<"\n";
    strOut<<"import glob"<<"\n";
    strOut<<"from math import floor, ceil, sqrt, isnan, modf, trunc"<<"\n";
    strOut<<"import csv"<<"\n";
    strOut<<"import re"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"class OptionParser(optparse.OptionParser):"<<"\n";
    strOut<<"    def check_required(self, opt):"<<"\n";
    strOut<<"        option = self.get_option(opt)"<<"\n";
    strOut<<"        # Assumes the option's 'default' is set to None!"<<"\n";
    strOut<<"        if getattr(self.values, option.dest) is None:"<<"\n";
    strOut<<"            self.error(\"%s option not supplied\" % option)"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def julian_date(day, month, year):"<<"\n";
    strOut<<"    if month <= 2:  # january & february"<<"\n";
    strOut<<"        year = year - 1.0"<<"\n";
    strOut<<"        month = month + 12.0"<<"\n";
    strOut<<"    jd = floor(365.25 * (year + 4716.0)) + floor(30.6001 * (month + 1.0)) + 2.0"<<"\n";
    strOut<<"    jd = jd - floor(year / 100.0) + floor(floor(year / 100.0) / 4.0)"<<"\n";
    strOut<<"    jd = jd + day - 1524.5"<<"\n";
    strOut<<"    # jd = jd + day - 1524.5 + (utc_time)/24."<<"\n";
    strOut<<"    mjd = jd - 2400000.5"<<"\n";
    strOut<<"    return jd, mjd"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def julian_date_to_date(jd):"<<"\n";
    strOut<<"    jd = jd + 0.5"<<"\n";
    strOut<<"    F, I = modf(jd)"<<"\n";
    strOut<<"    I = int(I)"<<"\n";
    strOut<<"    A = trunc((I - 1867216.25)/36524.25)"<<"\n";
    strOut<<"    if I > 2299160:"<<"\n";
    strOut<<"        B = I + 1 + A - trunc(A / 4.)"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        B = I"<<"\n";
    strOut<<"    C = B + 1524"<<"\n";
    strOut<<"    D = trunc((C - 122.1) / 365.25)"<<"\n";
    strOut<<"    E = trunc(365.25 * D)"<<"\n";
    strOut<<"    G = trunc((C - E) / 30.6001)"<<"\n";
    strOut<<"    day = C - E + F - trunc(30.6001 * G)"<<"\n";
    strOut<<"    if G < 13.5:"<<"\n";
    strOut<<"        month = G - 1"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        month = G - 13"<<"\n";
    strOut<<"    if month > 2.5:"<<"\n";
    strOut<<"        year = D - 4716"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        year = D - 4715"<<"\n";
    strOut<<"    return year, month, day"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def is_number(n):"<<"\n";
    strOut<<"    is_number = True"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        num = float(n)"<<"\n";
    strOut<<"        # check for \"nan\" floats"<<"\n";
    strOut<<"        is_number = num == num  # or use `math.isnan(num)`"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        is_number = False"<<"\n";
    strOut<<"    return is_number"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def process(input_shp,"<<"\n";
    strOut<<"            input_dsm,"<<"\n";
    strOut<<"            input_dtm,"<<"\n";
    strOut<<"            use_input_shp,"<<"\n";
    strOut<<"            output_shp,"<<"\n";
    strOut<<"            crop_minimum_height,"<<"\n";
    strOut<<"            compute_GCC,"<<"\n";
    strOut<<"            compute_volume,"<<"\n";
    strOut<<"            str_date):"<<"\n";
    strOut<<"    str_error = ''"<<"\n";
    strOut<<"    if not exists(input_dsm):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_dsm)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_dtm):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_dtm)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    dsm_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        dsm_ds = gdal.Open(input_dsm)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_dsm)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    dsm_rb = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        dsm_rb = dsm_ds.GetRasterBand(1)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting raster band from file:\\n{}\".format(input_dsm)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    dsm_geotransform = dsm_ds.GetGeoTransform()"<<"\n";
    strOut<<"    dtm_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        dtm_ds = gdal.Open(input_dtm)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_dtm)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    dtm_rb = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        dtm_rb = dtm_ds.GetRasterBand(1)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting raster band from file:\\n{}\".format(input_dtm)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    dtm_geotransform = dtm_ds.GetGeoTransform()"<<"\n";
    strOut<<"    for i in range(len(dsm_geotransform)):"<<"\n";
    strOut<<"        if abs(dsm_geotransform[i]-dtm_geotransform[i]) > 0.0000001:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nDifferent georreferencing in DTM and DSM\""<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"    dsm_crs = osr.SpatialReference()"<<"\n";
    strOut<<"    dsm_crs.ImportFromWkt(dsm_ds.GetProjectionRef())"<<"\n";
    strOut<<"    dtm_crs = osr.SpatialReference()"<<"\n";
    strOut<<"    dtm_crs.ImportFromWkt(dtm_ds.GetProjectionRef())"<<"\n";
    strOut<<"    dsm_crs_wkt = dsm_crs.ExportToWkt()"<<"\n";
    strOut<<"    dtm_crs_wkt = dtm_crs.ExportToWkt()"<<"\n";
    strOut<<"    if dsm_crs_wkt != dtm_crs_wkt:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nDifferent crs in DTM and DSM\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    ulx, xres, xskew, uly, yskew, yres = dsm_ds.GetGeoTransform()"<<"\n";
    strOut<<"    lrx = ulx + (dsm_ds.RasterXSize * xres)"<<"\n";
    strOut<<"    lry = uly + (dsm_ds.RasterYSize * yres)"<<"\n";
    strOut<<"    out_ring = ogr.Geometry(ogr.wkbLinearRing)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    dems_poly = ogr.Geometry(ogr.wkbPolygon)"<<"\n";
    strOut<<"    dems_poly.AddGeometry(out_ring)"<<"\n";
    strOut<<"    rs_pixel_width = dsm_geotransform[1]"<<"\n";
    strOut<<"    rs_pixel_height = dsm_geotransform[5]"<<"\n";
    strOut<<"    dems_pixel_area = abs(rs_pixel_width) * abs(rs_pixel_height)"<<"\n";
    strOut<<"    dems_x_origin = dsm_geotransform[0]"<<"\n";
    strOut<<"    dems_y_origin = dsm_geotransform[3]"<<"\n";
    strOut<<"    dems_pixel_width = dsm_geotransform[1]"<<"\n";
    strOut<<"    dems_pixel_height = dsm_geotransform[5]"<<"\n";
    strOut<<"    driver = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        if not use_input_shp:"<<"\n";
    strOut<<"            in_vec_ds = driver.Open(input_shp, 0)  # 0 means read-only. 1 means writeable."<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            in_vec_ds = driver.Open(input_shp, 1)  # 0 means read-only. 1 means writeable."<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    in_layer = in_vec_ds.GetLayer()"<<"\n";
    strOut<<"    in_crs = in_layer.GetSpatialRef()"<<"\n";
    strOut<<"    in_crs_wkt = in_crs.ExportToWkt()"<<"\n";
    strOut<<"    in_geometry_type = in_layer.GetGeomType()"<<"\n";
    strOut<<"    if in_geometry_type != ogr.wkbPolygon \\"<<"\n";
    strOut<<"            and in_geometry_type != ogr.wkbMultiPolygon \\"<<"\n";
    strOut<<"            and in_geometry_type != ogr.wkbPolygonM and in_geometry_type != ogr.wkbPolygonZM:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot Polygon geometry type in file:\\n{}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    out_vec_ds = None"<<"\n";
    strOut<<"    out_layer = None"<<"\n";
    strOut<<"    field_name_gcc = str_date + \"_gcc\""<<"\n";
    strOut<<"    field_name_vol = str_date + \"_vol\""<<"\n";
    strOut<<"    in_layer_definition = in_layer.GetLayerDefn()"<<"\n";
    strOut<<"    if use_input_shp:"<<"\n";
    strOut<<"        if compute_GCC:"<<"\n";
    strOut<<"            field_id_index = in_layer_definition.GetFieldIndex(field_name_gcc)"<<"\n";
    strOut<<"            if field_id_index == -1:"<<"\n";
    strOut<<"                in_layer.CreateField(ogr.FieldDefn(field_name_gcc, ogr.OFTReal))"<<"\n";
    strOut<<"        if compute_volume:"<<"\n";
    strOut<<"            field_id_index = in_layer_definition.GetFieldIndex(field_name_vol)"<<"\n";
    strOut<<"            if field_id_index == -1:"<<"\n";
    strOut<<"                in_layer.CreateField(ogr.FieldDefn(field_name_vol, ogr.OFTReal))"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        out_ds = driver.CreateDataSource(output_shp)"<<"\n";
    strOut<<"        out_layer = out_ds.CreateLayer(output_shp.split(\".\")[0], in_crs, in_geometry_type)"<<"\n";
    strOut<<"        for i in range(0, in_layer_definition.GetFieldCount()):"<<"\n";
    strOut<<"            fieldDefn = in_layer_definition.GetFieldDefn(i)"<<"\n";
    strOut<<"            out_layer.CreateField(fieldDefn)"<<"\n";
    strOut<<"        if compute_GCC:"<<"\n";
    strOut<<"            out_layer.CreateField(ogr.FieldDefn(field_name_gcc, ogr.OFTReal))"<<"\n";
    strOut<<"        if compute_volume:"<<"\n";
    strOut<<"            out_layer.CreateField(ogr.FieldDefn(field_name_vol, ogr.OFTReal))"<<"\n";
    strOut<<"    number_of_features = in_layer.GetFeatureCount()"<<"\n";
    strOut<<"    cont_feature = 0"<<"\n";
    strOut<<"    for feature in in_layer:"<<"\n";
    strOut<<"        cont_feature = cont_feature + 1"<<"\n";
    strOut<<"        print('Processing plant: {}, of {}'.format(str(cont_feature),"<<"\n";
    strOut<<"                                                   str(number_of_features)))"<<"\n";
    strOut<<"        plot_geometry_full = feature.GetGeometryRef().Clone()"<<"\n";
    strOut<<"        crs_transform = None"<<"\n";
    strOut<<"        if in_crs_wkt != dsm_crs_wkt:"<<"\n";
    strOut<<"            crs_transform = osr.CoordinateTransformation(in_crs, dsm_crs)"<<"\n";
    strOut<<"        if crs_transform:"<<"\n";
    strOut<<"            plot_geometry_full.Transform(crs_transform)"<<"\n";
    strOut<<"        plot_geometry = None"<<"\n";
    strOut<<"        if dems_poly.Overlaps(plot_geometry_full) or dems_poly.Intersects(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = plot_geometry_full.Intersection(dems_poly)"<<"\n";
    strOut<<"        if dems_poly.Contains(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = plot_geometry_full"<<"\n";
    strOut<<"        if dems_poly.Within(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = dems_poly"<<"\n";
    strOut<<"        if not plot_geometry:"<<"\n";
    strOut<<"            continue"<<"\n";
    strOut<<"        plot_geometry = plot_geometry_full.Intersection(dems_poly)"<<"\n";
    strOut<<"        plot_geometry_area = plot_geometry.GetArea()"<<"\n";
    strOut<<"        if plot_geometry_area < (3 * dems_pixel_area):"<<"\n";
    strOut<<"            continue"<<"\n";
    strOut<<"        geom_points_x = []"<<"\n";
    strOut<<"        geom_points_y = []"<<"\n";
    strOut<<"        geom_type_name = plot_geometry.GetGeometryName().lower()"<<"\n";
    strOut<<"        if \"multipolygon\" in geom_type_name:"<<"\n";
    strOut<<"            for i in range(0, plot_geometry.GetGeometryCount()):"<<"\n";
    strOut<<"                ring = plot_geometry.GetGeometryRef(i).GetGeometryRef(0)"<<"\n";
    strOut<<"                numpoints = ring.GetPointCount()"<<"\n";
    strOut<<"                for p in range(numpoints):"<<"\n";
    strOut<<"                    fc, sc, tc = ring.GetPoint(p)"<<"\n";
    strOut<<"                    geom_points_x.append(fc)"<<"\n";
    strOut<<"                    geom_points_y.append(sc)"<<"\n";
    strOut<<"        elif \"polygon\" in geom_type_name:"<<"\n";
    strOut<<"            ring = plot_geometry.GetGeometryRef(0)"<<"\n";
    strOut<<"            numpoints = ring.GetPointCount()"<<"\n";
    strOut<<"            for p in range(numpoints):"<<"\n";
    strOut<<"                fc, sc, tc = ring.GetPoint(p)"<<"\n";
    strOut<<"                geom_points_x.append(fc)"<<"\n";
    strOut<<"                geom_points_y.append(sc)"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            # sys.exit(\"ERROR: Geometry needs to be either Polygon or Multipolygon\")"<<"\n";
    strOut<<"            continue"<<"\n";
    strOut<<"        plot_geom_x_min = min(geom_points_x)"<<"\n";
    strOut<<"        plot_geom_x_max = max(geom_points_x)"<<"\n";
    strOut<<"        plot_geom_y_min = min(geom_points_y)"<<"\n";
    strOut<<"        plot_geom_y_max = max(geom_points_y)"<<"\n";
    strOut<<"        # Specify offset and rows and columns to read"<<"\n";
    strOut<<"        rs_x_off = int((plot_geom_x_min - dems_x_origin) / rs_pixel_width)"<<"\n";
    strOut<<"        rs_y_off = int((dems_y_origin - plot_geom_y_max) / rs_pixel_width)"<<"\n";
    strOut<<"        x_ul = dems_x_origin + rs_x_off * rs_pixel_width"<<"\n";
    strOut<<"        y_ul = dems_y_origin - rs_y_off * rs_pixel_width"<<"\n";
    strOut<<"        rs_x_count = int((plot_geom_x_max - plot_geom_x_min) / rs_pixel_width) + 1"<<"\n";
    strOut<<"        rs_y_count = int((plot_geom_y_max - plot_geom_y_min) / rs_pixel_width) + 1"<<"\n";
    strOut<<"        # Create memory target raster"<<"\n";
    strOut<<"        target_dsm = gdal.GetDriverByName('MEM').Create('', rs_x_count, rs_y_count, 1, gdal.GDT_Byte)"<<"\n";
    strOut<<"        target_dsm.SetGeoTransform(("<<"\n";
    strOut<<"            plot_geom_x_min, rs_pixel_width, 0,"<<"\n";
    strOut<<"            plot_geom_y_max, 0, rs_pixel_height,"<<"\n";
    strOut<<"        ))"<<"\n";
    strOut<<"        target_dtm = gdal.GetDriverByName('MEM').Create('', rs_x_count, rs_y_count, 1, gdal.GDT_Byte)"<<"\n";
    strOut<<"        target_dtm.SetGeoTransform(("<<"\n";
    strOut<<"            plot_geom_x_min, rs_pixel_width, 0,"<<"\n";
    strOut<<"            plot_geom_y_max, 0, rs_pixel_height,"<<"\n";
    strOut<<"        ))"<<"\n";
    strOut<<"        # Create for target raster the same projection as for the value raster"<<"\n";
    strOut<<"        raster_srs = osr.SpatialReference()"<<"\n";
    strOut<<"        raster_srs.ImportFromWkt(dsm_ds.GetProjectionRef())"<<"\n";
    strOut<<"        target_dsm.SetProjection(raster_srs.ExportToWkt())"<<"\n";
    strOut<<"        target_dtm.SetProjection(raster_srs.ExportToWkt())"<<"\n";
    strOut<<"        feature_drv = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"        feature_ds= feature_drv.CreateDataSource(\"/vsimem/memory_name.shp\")"<<"\n";
    strOut<<"        # geometryType = plot_geometry.getGeometryType()"<<"\n";
    strOut<<"        feature_layer = feature_ds.CreateLayer(\"layer\", dsm_crs, geom_type=plot_geometry.GetGeometryType())"<<"\n";
    strOut<<"        featureDefnHeaders = feature_layer.GetLayerDefn()"<<"\n";
    strOut<<"        out_feature = ogr.Feature(featureDefnHeaders)"<<"\n";
    strOut<<"        out_feature.SetGeometry(plot_geometry)"<<"\n";
    strOut<<"        feature_layer.CreateFeature(out_feature)"<<"\n";
    strOut<<"        feature_ds.FlushCache()"<<"\n";
    strOut<<"        # Rasterize zone polygon to raster dsm"<<"\n";
    strOut<<"        gdal.RasterizeLayer(target_dsm, [1], feature_layer, burn_values=[1])"<<"\n";
    strOut<<"        feature_dsm_data = dsm_rb.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count).astype(float)"<<"\n";
    strOut<<"        feature_dsm_band_mask = target_dsm.GetRasterBand(1)"<<"\n";
    strOut<<"        feature_dsm_data_mask = feature_dsm_band_mask.ReadAsArray(0, 0, rs_x_count, rs_y_count).astype(float)"<<"\n";
    strOut<<"        # Mask zone of raster dsm"<<"\n";
    strOut<<"        feature_dsm_raster_array = numpy.ma.masked_array(feature_dsm_data, numpy.logical_not(feature_dsm_data_mask))"<<"\n";
    strOut<<"        dsm_first_indexes, dsm_second_indexes = feature_dsm_raster_array.nonzero()"<<"\n";
    strOut<<"        # Rasterize zone polygon to raster dtm"<<"\n";
    strOut<<"        gdal.RasterizeLayer(target_dtm, [1], feature_layer, burn_values=[1])"<<"\n";
    strOut<<"        feature_dtm_data = dtm_rb.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count).astype(float)"<<"\n";
    strOut<<"        feature_dtm_band_mask = target_dtm.GetRasterBand(1)"<<"\n";
    strOut<<"        feature_dtm_data_mask = feature_dtm_band_mask.ReadAsArray(0, 0, rs_x_count, rs_y_count).astype(float)"<<"\n";
    strOut<<"        # Mask zone of raster dsm"<<"\n";
    strOut<<"        feature_dtm_raster_array = numpy.ma.masked_array(feature_dtm_data, numpy.logical_not(feature_dtm_data_mask))"<<"\n";
    strOut<<"        dtm_first_indexes, dtm_second_indexes = feature_dtm_raster_array.nonzero()"<<"\n";
    strOut<<"        values = []"<<"\n";
    strOut<<"        mean_value = 0."<<"\n";
    strOut<<"        min_value = 10000000."<<"\n";
    strOut<<"        max_value = -10000000."<<"\n";
    strOut<<"        number_of_pixels_in_frame = 0"<<"\n";
    strOut<<"        number_of_plant_pixels_in_frame = 0"<<"\n";
    strOut<<"        volume = 0"<<"\n";
    strOut<<"        for i in range(len(dsm_first_indexes)):"<<"\n";
    strOut<<"            fi = dsm_first_indexes[i]"<<"\n";
    strOut<<"            si = dsm_second_indexes[i]"<<"\n";
    strOut<<"            number_of_pixels_in_frame = number_of_pixels_in_frame + 1"<<"\n";
    strOut<<"            if not fi in dtm_first_indexes:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            if not si in dtm_second_indexes:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            # check mask"<<"\n";
    strOut<<"            dsm_height = feature_dsm_raster_array[dsm_first_indexes[i]][dsm_second_indexes[i]]"<<"\n";
    strOut<<"            dtm_height = feature_dtm_raster_array[dsm_first_indexes[i]][dsm_second_indexes[i]]"<<"\n";
    strOut<<"            value = dsm_height - dtm_height"<<"\n";
    strOut<<"            if value >= crop_minimum_height:"<<"\n";
    strOut<<"                values.append(value)"<<"\n";
    strOut<<"                if value < min_value:"<<"\n";
    strOut<<"                    min_value = value"<<"\n";
    strOut<<"                if value > max_value:"<<"\n";
    strOut<<"                    max_value = value"<<"\n";
    strOut<<"                mean_value = mean_value + value"<<"\n";
    strOut<<"                number_of_plant_pixels_in_frame = number_of_plant_pixels_in_frame + 1"<<"\n";
    strOut<<"                volume = volume + dems_pixel_area * value"<<"\n";
    strOut<<"        gcc = (float(number_of_plant_pixels_in_frame)) / (float(number_of_pixels_in_frame))"<<"\n";
    strOut<<"        if compute_GCC:"<<"\n";
    strOut<<"            feature.SetField(field_name_gcc, gcc)"<<"\n";
    strOut<<"        if compute_volume:"<<"\n";
    strOut<<"            feature.SetField(field_name_vol, volume * 1000.0)"<<"\n";
    strOut<<"        if not use_input_shp:"<<"\n";
    strOut<<"            out_layer.CreateFeature(feature)"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            in_layer.SetFeature(feature)"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    out_vec_ds = None"<<"\n";
    strOut<<"    return str_error"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def main():"<<"\n";
    strOut<<"    # =================="<<"\n";
    strOut<<"    # parse command line"<<"\n";
    strOut<<"    # =================="<<"\n";
    strOut<<"    usage = \"usage: %prog [options] \""<<"\n";
    strOut<<"    parser = OptionParser(usage=usage)"<<"\n";
    strOut<<"    parser.add_option(\"--input_crops_frames_shp\", dest=\"input_crops_frames_shp\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Crops frames shapefile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--input_dsm\", dest=\"input_dsm\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"DSM geotiff\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--input_dtm\", dest=\"input_dtm\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"DTM geotiff\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--crop_minimum_height\", dest=\"crop_minimum_height\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Crop minimum height, in meters\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--output_shp\", dest=\"output_shp\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Output shapefile or none for use input shapefile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--compute_GCC\", dest=\"compute_GCC\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Compute GCC: 1-yes, 0-No\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--compute_volume\", dest=\"compute_volume\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Compute volume: 1-yes, 0-No\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date_from_dem_files\", dest=\"date_from_dem_files\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Read date from DEM files: 1-yes, 0-No\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--dem_files_string_separator\", dest=\"dem_files_string_separator\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"DEM files string separator\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--dem_files_date_string_position\", dest=\"dem_files_date_string_position\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"DEM files date string position\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date_format\", dest=\"date_format\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Date format (%Y%m%d, ...)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date\", dest=\"date\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Date value no from DEM files\", default=None)"<<"\n";
    strOut<<"    (options, args) = parser.parse_args()"<<"\n";
    strOut<<"    if not options.input_crops_frames_shp \\"<<"\n";
    strOut<<"        or not options.input_dsm \\"<<"\n";
    strOut<<"        or not options.input_dtm \\"<<"\n";
    strOut<<"        or not options.crop_minimum_height \\"<<"\n";
    strOut<<"        or not options.output_shp \\"<<"\n";
    strOut<<"        or not options.compute_GCC \\"<<"\n";
    strOut<<"        or not options.compute_volume \\"<<"\n";
    strOut<<"        or not options.date_from_dem_files \\"<<"\n";
    strOut<<"        or not options.dem_files_string_separator \\"<<"\n";
    strOut<<"        or not options.dem_files_date_string_position \\"<<"\n";
    strOut<<"        or not options.date_format \\"<<"\n";
    strOut<<"        or not options.date :"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_crops_shapefile = options.input_crops_frames_shp"<<"\n";
    strOut<<"    if not exists(input_crops_shapefile):"<<"\n";
    strOut<<"        print(\"Error:\\nInput crops shapefile does not exists:\\n{}\".format(input_crops_shapefile))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_dsm = options.input_dsm"<<"\n";
    strOut<<"    if not exists(input_dsm):"<<"\n";
    strOut<<"        print(\"Error:\\nInput DSM does not exists:\\n{}\".format(input_dsm))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_dtm = options.input_dtm"<<"\n";
    strOut<<"    if not exists(input_dtm):"<<"\n";
    strOut<<"        print(\"Error:\\nInput DTM does not exists:\\n{}\".format(input_dtm))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    crop_minimum_height = options.crop_minimum_height"<<"\n";
    strOut<<"    if not is_number(crop_minimum_height):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for crops minimum height: {}\"."<<"\n";
    strOut<<"              format(crop_minimum_height))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    crop_minimum_height = float(crop_minimum_height)"<<"\n";
    strOut<<"    compute_GCC = False"<<"\n";
    strOut<<"    if options.compute_GCC == 1:"<<"\n";
    strOut<<"        compute_GCC = True"<<"\n";
    strOut<<"    compute_volume = False"<<"\n";
    strOut<<"    if options.compute_volume == 1:"<<"\n";
    strOut<<"        compute_volume = True"<<"\n";
    strOut<<"    if not compute_GCC and not compute_volume:"<<"\n";
    strOut<<"        print(\"Error:\\nNothing to do, no GCC or volume computation\")"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    use_input_shp = True"<<"\n";
    strOut<<"    output_shp = options.output_shp"<<"\n";
    strOut<<"    if output_shp != 'none':"<<"\n";
    strOut<<"        use_input_shp = False"<<"\n";
    strOut<<"    date_from_dem_files = False"<<"\n";
    strOut<<"    if options.date_from_dem_files == 1:"<<"\n";
    strOut<<"        date_from_dem_files = True"<<"\n";
    strOut<<"    date_format = options.date_format.strip()"<<"\n";
    strOut<<"    if not date_from_dem_files:"<<"\n";
    strOut<<"        if options.date == 'none':"<<"\n";
    strOut<<"            print(\"Error:\\nDate must be a value if not read from dems file name\")"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        date = datetime.datetime.strptime(options.date, date_format)"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        dem_files_string_separator = options.dem_files_string_separator"<<"\n";
    strOut<<"        dem_files_date_string_position = options.dem_files_date_string_position"<<"\n";
    strOut<<"        dsm_file_name_without_path = os.path.basename(input_dsm)"<<"\n";
    strOut<<"        dsm_file_name_values = dsm_file_name_without_path.split(dem_files_string_separator)"<<"\n";
    strOut<<"        if dem_files_date_string_position < 0 or dem_files_date_string_position > len(dsm_file_name_values):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for dsm files date string position: {}\"."<<"\n";
    strOut<<"                  format(str(dem_files_date_string_position)))"<<"\n";
    strOut<<"        str_date = dsm_file_name_values[dem_files_date_string_position-1]"<<"\n";
    strOut<<"        date_dsm = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"        dtm_file_name_without_path = os.path.basename(input_dtm)"<<"\n";
    strOut<<"        dtm_file_name_values = dtm_file_name_without_path.split(dem_files_string_separator)"<<"\n";
    strOut<<"        if dem_files_date_string_position < 0 or dem_files_date_string_position > len(dtm_file_name_values):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for dtm files date string position: {}\"."<<"\n";
    strOut<<"                  format(str(dem_files_date_string_position)))"<<"\n";
    strOut<<"        str_date = dtm_file_name_values[dem_files_date_string_position-1]"<<"\n";
    strOut<<"        date_dtm = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"        if date_dsm != date_dtm:"<<"\n";
    strOut<<"            print(\"Error:\\nDifferents dates from DSM and DTM\")"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        date = date_dsm"<<"\n";
    strOut<<"    str_date = str(date.strftime('%Y')[2:4]) + str(date.strftime('%m')) + str(date.strftime('%d'))"<<"\n";
    strOut<<"    str_error = process(input_crops_shapefile,"<<"\n";
    strOut<<"                        input_dsm,"<<"\n";
    strOut<<"                        input_dtm,"<<"\n";
    strOut<<"                        use_input_shp,"<<"\n";
    strOut<<"                        output_shp,"<<"\n";
    strOut<<"                        crop_minimum_height,"<<"\n";
    strOut<<"                        compute_GCC,"<<"\n";
    strOut<<"                        compute_volume,"<<"\n";
    strOut<<"                        str_date)"<<"\n";
    strOut<<"    if str_error:"<<"\n";
    strOut<<"        print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    print(\"... Process finished\")"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"if __name__ == '__main__':"<<"\n";
    strOut<<"    main()"<<"\n";

    file.close();
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
    for(int npf=0;npf<mPythonFiles.size();npf++)
    {
        QFile::remove(mPythonFiles[npf]);
    }
    mPythonFiles.clear();
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
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_CCFPGP,Qt::CaseInsensitive)==0)
    {
        if(!process_ccfpgp(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

