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
        QString qgisPythonGdalPolygonizeFileName=qgisPath+PAFYCTOOLSGUI_QGIS_PYTHON_GDAL_POLYGONIZE;
        if(!QFile::exists(qgisPythonGdalPolygonizeFileName))
        {
            strError=functionName;
            strError+=QObject::tr("Not exists QGIS python GDAL polygonize file:\n%1").arg(qgisPythonGdalPolygonizeFileName);
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
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_BCVRM);
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_CMNDVI);
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_CMGCCVOL);
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_CWSITHO);
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_SZBR);
    mCommands.push_back(PAFYCTOOLSGUI_COMMAND_MFHA);
//    mCommands.push_back();
    QVector<QString> aux1;
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC]=aux1;
    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL);
//    mSubCommandsByCommand[PAFYCTOOLSGUI_COMMAND_PLPPC].push_back(PAFYCTOOLSGUI_COMMAND_PLPPC_PP);
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
    mFilesToRemove.clear();
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
//        QString strDateTime=QDateTime::toString("yyyyddMM_hhmmss");
//        QFileInfo fileInfo(framesShapefile);
//        QString auxPath=fileInfo.absolutePath();
//        QString auxCompleteBaseName=fileInfo.completeBaseName();
//        QString auxSuffix=fileInfo.completeSuffix();
//        QString newShapefile=auxPath+"/"+auxCompleteBaseName;
//        newShapefile+="_";
//        newShapefile+=strDateTime;
//        newShapefile+=".";
//        newShapefile+=auxSuffix;
//        copyShapefile(framesShapefile,newShapefile);
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

 --input_crops_frames_shp "D:/PAFyCToolsGui/20231019_mamh/Datos/Marcos_export.shp"
--input_dsm "D:/PAFyCToolsGui/20231019_mamh/Datos/DSM_20220615.tif"
--input_dtm "D:/PAFyCToolsGui/20231019_mamh/Datos/DEM_20220615.tif"
--crop_minimum_height 0.05 --output_shp "D:/PAFyCToolsGui/20231019_mamh/output_dhl/marcos_export.shp" --compute_GCC 1 --compute_volume 1
--date_from_dem_files 1 --dem_files_string_separator="_" --dem_files_date_string_position=2 --date_format=%Y%m%d --date=none
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
//    dateFormat=dateFormat.replace("%","%%");
//    strOut<<"--date_format=\""<<dateFormat<<"\" ";
    strOut<<"--date_format="<<dateFormat<<" ";
    if(str_date.compare("none",Qt::CaseInsensitive)==0)
    {
        strOut<<"--date="<<str_date<<"\n";
    }
    else
    {
        strOut<<"--date=\""<<str_date<<"\"\n";
    }
    file.close();

//    mFilesToRemove.push_back(processFileName);

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
        set LASNOISE_STEP_H=0.05 # lo he cambiado a 1.2
        set LASNOISE_MINIMUN_NUMBER_OF_POINTS=10 # lo he cambiado a 100
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
        strOut<<"set LASNOISE_STEP_H=1.20"<<"\n";
        strOut<<"set LASNOISE_MINIMUN_NUMBER_OF_POINTS=100"<<"\n";
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

bool PAFyCToolsDialog::process_plppc_pp_pwol(QString &qgisPath,
                                             QString &outputPath,
                                             QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,pointCloudFileName,roiShapefileName,inputPath,latsShapefile,parameterCode,lastoolsPath,lidarFilesPath;
    int epsgCode;
    int intValue;
    double dblValue;
    bool okToNumber;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_POINT_CLOUD_FILE;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_ROI_SHAPEFILE;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_CRS_EPSG_CODE;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_LASTOOLS_PATH;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_GROUND_CLASSIFICATION_STEP;
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
    double groundClassicationStep=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_ADAPTATIVE_THINNING_2D;
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
    double adaptativeThinning2d=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_ADAPTATIVE_THINNING_H;
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
    double adaptativeThinningH=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_BASE_MININUM_HEIGHT;
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
    double trunkBaseMinimumHeight=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_BASE_MAXINUM_HEIGHT;
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
    double trunkBaseMaximumHeight=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_HIGHER_MINIMUM_HEIGHT;
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
    double trunkHigherMinimumHeight=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_HIGHER_MAXIMUM_HEIGHT;
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
    double trunkHigherMaximumHeight=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_HIGHER_RASTER_FILTER_GSD;
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
    double trunkHigherRasterFilterGsd=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_BASE_RASTER_FILTER_GSD;
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
    double trunkBaseRasterFilterGsd=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_DTM_GSD;
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
    double dtmGsd=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_TAG_TRUNK_BASE_RASTER_FILTER_MIN_PIXELS;
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
    intValue=strValue.toInt(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an interger")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    double trunkBaseMinimumArea=pow(trunkBaseRasterFilterGsd,2.0)*((double)intValue);

    QString processFileNameLastools_1=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_LASTOOLS_1;
    {
        if(QFile::exists(processFileNameLastools_1))
        {
            if(!QFile::remove(processFileNameLastools_1))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileNameLastools_1);
                return(false);
            }
        }
        QFile file(processFileNameLastools_1);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileNameLastools_1);
            return(false);
        }
        QTextStream strOut(&file);
        /*
        echo off
        set PATH=%PATH%;"C:/LAStools/bin"
        set PROCESS_PATH=D:/PAFyCToolsGui/230329_FuenteAlamo_Vid_A6000/20230529
        set INPUT_POINT_CLOUD_FILE=C:/DHL/PAFyCToolsGui/230329_FuenteAlamo_Vid_A6000/input_data/230329_FuenteAlamo_Vid_A6000_PC_25830_gcp.laz
        set INPUT_ROI_SHAPEFILE=C:/DHL/PAFyCToolsGui/230329_FuenteAlamo_Vid_A6000/input_data/FuenteAlamo_25830.shp
        set OUTPUT_POINT_CLOUD_CLIPPED_FILE=%PROCESS_PATH%\230329_FuenteAlamo_Vid_A6000_PC_25830_gcp_clipped.laz
        cd /d "%PROCESS_PATH%"
        set TILE_SIZE=10
        set TILE_BUFFER=1
        set GROUND_STEP_SIZE=0.5
        set CORES=8
        set LASTHIN_ADAPTATIVE_2D=0.20
        set LASTHIN_ADAPTATIVE_H=0.04
        set VINE_TRUNK_BASE_MINIMUM_HEIGHT=0.01
        set VINE_TRUNK_BASE_MAXIMUM_HEIGHT=0.2
        set VINE_TRUNK_HIGHER_MINIMUM_HEIGHT=0.3
        set VINE_TRUNK_HIGHER_MAXIMUM_HEIGHT=1.2
        set VINE_FILTER_RASTER_STEP=0.4
        set VINE_FILTER_RASTER_FILE_NAME=vines.tif
        set VINE_RASTER_TRUNK_STEP=0.02
        set DTM_STEP=0.10
        set OUTPUT_DTM_FILE="230329_FuenteAlamo_Vid_A6000_PC_25830_gcp_dtm_10cm.tif"
        lasclip64 -v -i "%INPUT_POINT_CLOUD_FILE%" ^
                  -poly "%INPUT_ROI_SHAPEFILE%" ^
                  -o "%OUTPUT_POINT_CLOUD_CLIPPED_FILE%"
        rmdir temp_tiles /s /q
        mkdir temp_tiles
        lastile -v -i "%OUTPUT_POINT_CLOUD_CLIPPED_FILE%" ^
                -reversible -tile_size %TILE_SIZE% -buffer %TILE_BUFFER% ^
                -o temp_tiles\tile.laz -olaz
        rmdir temp_tiles_ground /s /q
        mkdir temp_tiles_ground
        lasground_new -v -i temp_tiles\tile*.laz ^
                  -step %GROUND_STEP_SIZE% -coarse ^
                  -odir temp_tiles_ground -olaz ^
                  -cores %CORES%
        rmdir temp_tiles /s /q
        rmdir temp_tiles_ground_thinned /s /q
        mkdir temp_tiles_ground_thinned
        lasthin64 -v -i temp_tiles_ground\tile*.laz ^
                  -ignore_class 1 -adaptive %LASTHIN_ADAPTATIVE_H% %LASTHIN_ADAPTATIVE_2D% ^
                  -odir temp_tiles_ground_thinned -olaz ^
                  -cores %CORES%
        rmdir temp_tiles_ground /s /q
        rmdir temp_tiles_ground_thinned_height /s /q
        mkdir temp_tiles_ground_thinned_height
        lasheight64 -v -i temp_tiles_ground_thinned\tile*.laz ^
                  -ignore_class 2 -classify_below %VINE_TRUNK_BASE_MINIMUM_HEIGHT% 0 ^
                  -classify_between %VINE_TRUNK_BASE_MINIMUM_HEIGHT% %VINE_TRUNK_BASE_MAXIMUM_HEIGHT% 3 ^
                  -classify_between %VINE_TRUNK_HIGHER_MINIMUM_HEIGHT% %VINE_TRUNK_HIGHER_MAXIMUM_HEIGHT% 4 ^
                  -classify_above %VINE_H_ARMS_MAXIMUM_HEIGHT% 7 ^
                  -odir temp_tiles_ground_thinned_height -olaz ^
                  -cores %CORES%
        rmdir temp_tiles_ground_thinned /s /q
        lastile -i temp_tiles_ground_thinned_height\tile*.laz ^
                -reverse_tiling ^
                -o "%OUTPUT_POINT_CLOUD_CLIPPED_FILE%" -odix _ground -olaz
        rmdir temp_tiles_ground_thinned_height /s /q
        lasgrid64 -v -i *_ground.laz ^
                  -keep_class 4 -step %VINE_FILTER_RASTER_STEP% -counter_16bit ^
                  -o %VINE_FILTER_RASTER_FILE_NAME%
        las2dem64 -v -i *_ground.laz ^
                  -step %DTM_STEP% -keep_class 2 ^
                  -o %OUTPUT_DTM_FILE%
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
        strOut<<"set GROUND_STEP_SIZE="<<QString::number(groundClassicationStep,'f',3)<<"\n";
        strOut<<"set CORES=8"<<"\n";
        strOut<<"set LASTHIN_ADAPTATIVE_2D="<<QString::number(adaptativeThinning2d,'f',3)<<"\n";
        strOut<<"set LASTHIN_ADAPTATIVE_H="<<QString::number(adaptativeThinningH,'f',3)<<"\n";
        strOut<<"set VINE_TRUNK_BASE_MINIMUM_HEIGHT="<<QString::number(trunkBaseMinimumHeight,'f',3)<<"\n";
        strOut<<"set VINE_TRUNK_BASE_MAXIMUM_HEIGHT="<<QString::number(trunkBaseMaximumHeight,'f',3)<<"\n";
        strOut<<"set VINE_TRUNK_HIGHER_MINIMUM_HEIGHT="<<QString::number(trunkHigherMinimumHeight,'f',3)<<"\n";
        strOut<<"set VINE_TRUNK_HIGHER_MAXIMUM_HEIGHT="<<QString::number(trunkHigherMaximumHeight,'f',3)<<"\n";
        strOut<<"set VINE_FILTER_RASTER_STEP="<<QString::number(trunkHigherRasterFilterGsd,'f',3)<<"\n";
        strOut<<"set VINE_FILTER_RASTER_FILE_NAME=vines.tif"<<"\n";
        strOut<<"set VINE_RASTER_TRUNK_STEP="<<QString::number(trunkBaseRasterFilterGsd,'f',3)<<"\n";
        strOut<<"set DTM_STEP="<<QString::number(dtmGsd,'f',2)<<"\n";
        QString dtmFileName=QFileInfo(pointCloudFileName).completeBaseName();
        dtmFileName+="_dtm_";
        dtmFileName+=QString::number(qRound(dtmGsd*100.));
        dtmFileName+="cm.tif";
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
        strOut<<"          -ignore_class 1 -adaptive %LASTHIN_ADAPTATIVE_H% %LASTHIN_ADAPTATIVE_2D% ^"<<"\n";
        strOut<<"          -odir temp_tiles_ground_thinned -olaz ^"<<"\n";
        strOut<<"          -cores %CORES%"<<"\n";
        strOut<<"rmdir temp_tiles_ground /s /q"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned_height /s /q"<<"\n";
        strOut<<"mkdir temp_tiles_ground_thinned_height"<<"\n";
        strOut<<"lasheight64 -v -i temp_tiles_ground_thinned\\tile*.laz ^"<<"\n";
        strOut<<"          -ignore_class 2 -classify_below %VINE_TRUNK_BASE_MINIMUM_HEIGHT% 0 ^"<<"\n";
        strOut<<"          -classify_between %VINE_TRUNK_BASE_MINIMUM_HEIGHT% %VINE_TRUNK_BASE_MAXIMUM_HEIGHT% 3 ^"<<"\n";
        strOut<<"          -classify_between %VINE_TRUNK_HIGHER_MINIMUM_HEIGHT% %VINE_TRUNK_HIGHER_MAXIMUM_HEIGHT% 4 ^"<<"\n";
        strOut<<"          -classify_above %VINE_H_ARMS_MAXIMUM_HEIGHT% 7 ^"<<"\n";
        strOut<<"          -odir temp_tiles_ground_thinned_height -olaz ^"<<"\n";
        strOut<<"          -cores %CORES%"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned /s /q"<<"\n";
        strOut<<"lastile -i temp_tiles_ground_thinned_height\\tile*.laz ^"<<"\n";
        strOut<<"        -reverse_tiling ^"<<"\n";
        strOut<<"        -o \"%OUTPUT_POINT_CLOUD_CLIPPED_FILE%\" -odix _ground -olaz"<<"\n";
        strOut<<"rmdir temp_tiles_ground_thinned_height /s /q"<<"\n";
        strOut<<"lasgrid64 -v -i *_ground.laz ^"<<"\n";
        strOut<<"          -keep_class 4 -step %VINE_FILTER_RASTER_STEP% -counter_16bit ^"<<"\n";
        strOut<<"          -o %VINE_FILTER_RASTER_FILE_NAME%"<<"\n";
        strOut<<"las2dem64 -v -i *_ground.laz ^"<<"\n";
        strOut<<"          -step %DTM_STEP% -keep_class 2 ^"<<"\n";
        strOut<<"          -o %OUTPUT_DTM_FILE%"<<"\n";
        file.close();
    }
    QString processFileNameGdal_1=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_GDAL_1;
    {
        if(QFile::exists(processFileNameGdal_1))
        {
            if(!QFile::remove(processFileNameGdal_1))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileNameGdal_1);
                return(false);
            }
        }
        QFile file(processFileNameGdal_1);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileNameGdal_1);
            return(false);
        }
        QTextStream strOut(&file);
        /*
        echo off
        set OSGEO4W_ROOT=C:/Program Files/QGIS 3.22.12
        set PROCESS_PATH=D:/PAFyCToolsGui/230329_FuenteAlamo_Vid_A6000/20230529
        set INPUT_RASTER_FILE=%PROCESS_PATH%\vines.tif
        set OUTPUT_SHAPEFILE_PIXELS=%PROCESS_PATH%\vines_pixels.shp
        set OUTPUT_SHAPEFILE_PIXELS_UNION=%PROCESS_PATH%\vines_pixels_union.shp
        set DELETE_RASTER_FILE=%PROCESS_PATH%\vines.*
        set DELETE_SHAPEFILE_PIXELS=%PROCESS_PATH%\vines_pixels.*
        call "%OSGEO4W_ROOT%\\bin\o4w_env.bat"
        python "%OSGEO4W_ROOT%\\apps\\Python39\\Scripts\\gdal_polygonize.py" "%INPUT_RASTER_FILE%" -b 1 -f "ESRI Shapefile" "%OUTPUT_SHAPEFILE_PIXELS%" OUTPUT DN
        ogr2ogr -f "ESRI Shapefile" "%OUTPUT_SHAPEFILE_PIXELS_UNION%" "%OUTPUT_SHAPEFILE_PIXELS%" -dialect sqlite -sql "SELECT ST_Union(geometry) FROM vines_pixels" -explodecollections
        del "%DELETE_RASTER_FILE%"
        del "%DELETE_SHAPEFILE_PIXELS%"
        */
        strOut<<"echo off"<<"\n";
        strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
        strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
        strOut<<"set INPUT_RASTER_FILE=%PROCESS_PATH%\\vines.tif"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_PIXELS=%PROCESS_PATH%\\vines_pixels.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_PIXELS_UNION=%PROCESS_PATH%\\vines_pixels_union.shp"<<"\n";
        strOut<<"set DELETE_RASTER_FILE=%PROCESS_PATH%\\vines.*"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_PIXELS=%PROCESS_PATH%\\vines_pixels.*"<<"\n";
        strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
        strOut<<"python \"%OSGEO4W_ROOT%\\apps\\Python39\\Scripts\\gdal_polygonize.py\" \"%INPUT_RASTER_FILE%\" -b 1 -f \"ESRI Shapefile\" \"%OUTPUT_SHAPEFILE_PIXELS%\" OUTPUT DN"<<"\n";
        strOut<<"ogr2ogr -f \"ESRI Shapefile\" \"%OUTPUT_SHAPEFILE_PIXELS_UNION%\" \"%OUTPUT_SHAPEFILE_PIXELS%\" -dialect sqlite -sql \"SELECT ST_Union(geometry) FROM vines_pixels\" -explodecollections"<<"\n";
        strOut<<"del \"%DELETE_RASTER_FILE%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_PIXELS%\""<<"\n";
        file.close();
    }

    QString processFileNameLastools_2=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_LASTOOLS_2;
    {
        if(QFile::exists(processFileNameLastools_2))
        {
            if(!QFile::remove(processFileNameLastools_2))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileNameLastools_2);
                return(false);
            }
        }
        QFile file(processFileNameLastools_2);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileNameLastools_2);
            return(false);
        }
        QTextStream strOut(&file);
        /*
        echo off
        set PATH=%PATH%;"C:/LAStools/bin"
        set PROCESS_PATH=D:/PAFyCToolsGui/230329_FuenteAlamo_Vid_A6000/20230529
        set OUTPUT_SHAPEFILE_PIXELS_UNION=%PROCESS_PATH%\vines_pixels_union.shp
        set OUTPUT_FILTER_LAZ=%PROCESS_PATH%\vine_trunk_filter.laz
        set VINE_TRUNK_FILTER_RASTER_STEP=0.01
        set VINE_RASTER_TRUNK_FILE_NAME=%PROCESS_PATH%\vines_trunk.tif
        set DELETE_SHAPEFILE_UNION=%PROCESS_PATH%\vines_pixels_union.*
        cd /d "%PROCESS_PATH%"
        lasclip64 -i *_ground.laz -poly "%OUTPUT_SHAPEFILE_PIXELS_UNION%" -o "%OUTPUT_FILTER_LAZ%"
        lasgrid64 -i "%OUTPUT_FILTER_LAZ%" -keep_class 3 -step %VINE_TRUNK_FILTER_RASTER_STEP% -counter_16bit -o "%VINE_RASTER_TRUNK_FILE_NAME%"
        del "%DELETE_SHAPEFILE_UNION%"
        del "%OUTPUT_FILTER_LAZ%"
        */
        strOut<<"echo off"<<"\n";
        strOut<<"set PATH=%PATH%;\""<<lastoolsPath<<"\"\n";
        strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_PIXELS_UNION=%PROCESS_PATH%\\vines_pixels_union.shp"<<"\n";
        strOut<<"set OUTPUT_FILTER_LAZ=%PROCESS_PATH%\\vine_trunk_filter.laz"<<"\n";
        strOut<<"set VINE_TRUNK_FILTER_RASTER_STEP="<<QString::number(trunkBaseRasterFilterGsd,'f',3)<<"\n";
        strOut<<"set VINE_RASTER_TRUNK_FILE_NAME=%PROCESS_PATH%\\vines_trunk.tif"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_UNION=%PROCESS_PATH%\\vines_pixels_union.*"<<"\n";
        strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
        strOut<<"lasclip64 -i *_ground.laz -poly \"%OUTPUT_SHAPEFILE_PIXELS_UNION%\" -o \"%OUTPUT_FILTER_LAZ%\""<<"\n";
        strOut<<"lasgrid64 -i \"%OUTPUT_FILTER_LAZ%\" -keep_class 3 -step %VINE_TRUNK_FILTER_RASTER_STEP% -counter_16bit -o \"%VINE_RASTER_TRUNK_FILE_NAME%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_UNION%\""<<"\n";
        strOut<<"del \"%OUTPUT_FILTER_LAZ%\""<<"\n";
        file.close();
    }
    QString processFileNameGdal_2=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_GDAL_2;
    {
        if(QFile::exists(processFileNameGdal_2))
        {
            if(!QFile::remove(processFileNameGdal_2))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing file:\n%1").arg(processFileNameGdal_2);
                return(false);
            }
        }
        QFile file(processFileNameGdal_2);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening file:\n%1").arg(processFileNameGdal_1);
            return(false);
        }
        QTextStream strOut(&file);
        /*
        echo off
        set OSGEO4W_ROOT=C:/Program Files/QGIS 3.22.12
        set PROCESS_PATH=D:/PAFyCToolsGui/230329_FuenteAlamo_Vid_A6000/20230529
        set INPUT_RASTER_FILE=%PROCESS_PATH%\vines_trunk.tif
        set OUTPUT_SHAPEFILE_AUX=%PROCESS_PATH%\vines_taux.shp
        set OUTPUT_SHAPEFILE_POLYGON=%PROCESS_PATH%\vines_tpol.shp
        set OUTPUT_SHAPEFILE_FID=%PROCESS_PATH%\vines_trunk_ctrsfid.shp
        set OUTPUT_SHAPEFILE=%PROCESS_PATH%\vines_trunk_contours.shp
        set MINIMUM_AREA=0.0003
        set DELETE_RASTER_TRUNK=%PROCESS_PATH%\vines_trunk.*
        set DELETE_SHAPEFILE_AUX=%PROCESS_PATH%\vines_taux.*
        set DELETE_SHAPEFILE_POLYGON=%PROCESS_PATH%\vines_tpol.*
        set DELETE_SHAPEFILE_FID=%PROCESS_PATH%\vines_trunk_ctrsfid.*
        set FIELD_ID_INITIAL=FID
        set FIELD_ID_FINAL=id
        call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
        python "%OSGEO4W_ROOT%\apps\Python39\Scripts\gdal_polygonize.py" "%INPUT_RASTER_FILE%" -b 1 -f "ESRI Shapefile" "%OUTPUT_SHAPEFILE_AUX%" OUTPUT DN
        ogr2ogr -f "ESRI Shapefile" "%OUTPUT_SHAPEFILE_POLYGON%" "%OUTPUT_SHAPEFILE_AUX%" -explodecollections -dialect sqlite -sql "select ST_union(geometry) from vines_taux"
        ogr2ogr -f "ESRI Shapefile" "%OUTPUT_SHAPEFILE%" "%OUTPUT_SHAPEFILE_POLYGON%" -dialect sqlite -sql "select * from vines_tpol where ST_area(geometry)>%MINIMUM_AREA%"
        ogr2ogr "%OUTPUT_SHAPEFILE%" "%OUTPUT_SHAPEFILE_FID%" -sql "SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_trunk_ctrsfid"
        ogrinfo "%OUTPUT_SHAPEFILE%" -sql "alter table vines_trunk_contours add column enabled integer"
        ogrinfo "%OUTPUT_SHAPEFILE%" -dialect SQLite -sql "update vines_trunk_contours set enabled=1"
        del "%DELETE_RASTER_TRUNK%"
        del "%DELETE_SHAPEFILE_AUX%"
        del "%DELETE_SHAPEFILE_POLYGON%"
        */
        strOut<<"echo off"<<"\n";
        strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
        strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
        strOut<<"set INPUT_RASTER_FILE=%PROCESS_PATH%\\vines_trunk.tif"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_AUX=%PROCESS_PATH%\\vines_taux.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_POLYGON=%PROCESS_PATH%\\vines_tpol.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE_FID=%PROCESS_PATH%\\vines_trunk_ctrsfid.shp"<<"\n";
        strOut<<"set OUTPUT_SHAPEFILE=%PROCESS_PATH%\\vines_trunk_contours.shp"<<"\n";
        strOut<<"set MINIMUM_AREA="<<QString::number(trunkBaseMinimumArea,'f',6)<<"\n";
        strOut<<"set DELETE_RASTER_TRUNK=%PROCESS_PATH%\\vines_trunk.*"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_AUX=%PROCESS_PATH%\\vines_taux.*"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_POLYGON=%PROCESS_PATH%\\vines_tpol.*"<<"\n";
        strOut<<"set DELETE_SHAPEFILE_FID=%PROCESS_PATH%\\vines_trunk_ctrsfid.*"<<"\n";
        strOut<<"set FIELD_ID_INITIAL=FID"<<"\n";
        strOut<<"set FIELD_ID_FINAL=id"<<"\n";
        strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
        strOut<<"python \"%OSGEO4W_ROOT%\\apps\\Python39\\Scripts\\gdal_polygonize.py\" \"%INPUT_RASTER_FILE%\" -b 1 -f \"ESRI Shapefile\" \"%OUTPUT_SHAPEFILE_AUX%\" OUTPUT DN"<<"\n";
        strOut<<"ogr2ogr -f \"ESRI Shapefile\" \"%OUTPUT_SHAPEFILE_POLYGON%\" \"%OUTPUT_SHAPEFILE_AUX%\" -explodecollections -dialect sqlite -sql \"select ST_union(geometry) from vines_taux\""<<"\n";
        strOut<<"ogr2ogr -f \"ESRI Shapefile\" \"%OUTPUT_SHAPEFILE_FID%\" \"%OUTPUT_SHAPEFILE_POLYGON%\" -dialect sqlite -sql \"select * from vines_tpol where ST_area(geometry)>%MINIMUM_AREA%\""<<"\n";
        strOut<<"ogr2ogr \"%OUTPUT_SHAPEFILE%\" \"%OUTPUT_SHAPEFILE_FID%\" -sql \"SELECT %FIELD_ID_INITIAL% AS %FIELD_ID_FINAL% from vines_trunk_ctrsfid\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_SHAPEFILE%\" -sql \"alter table vines_trunk_contours add column enabled integer\""<<"\n";
        strOut<<"ogrinfo \"%OUTPUT_SHAPEFILE%\" -dialect SQLite -sql \"update vines_trunk_contours set enabled=1\""<<"\n";
        strOut<<"del \"%DELETE_RASTER_TRUNK%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_AUX%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_POLYGON%\""<<"\n";
        strOut<<"del \"%DELETE_SHAPEFILE_FID%\""<<"\n";
        file.close();
    }
    //groundClassicationStep,adaptativeThinning2d,adaptativeThinningH
    //trunkBaseMinimumHeight,trunkBaseMaximumHeight
    //trunkHigherMinimumHeight,trunkHigherMaximumHeight
    //trunkHigherRasterFilterGsd,trunkBaseRasterFilterGsd
    //dtmGsd,trunkBaseMinimumArea    
    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE;
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
        strOut<<"call \"%OUTPUT_PATH%/"<<PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_LASTOOLS_1<<"\"\n";
        strOut<<"call \"%OUTPUT_PATH%/"<<PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_GDAL_1<<"\"\n";
        strOut<<"call \"%OUTPUT_PATH%/"<<PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_LASTOOLS_2<<"\"\n";
        strOut<<"call \"%OUTPUT_PATH%/"<<PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL_PROCESS_FILE_GDAL_2<<"\"\n";
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

bool PAFyCToolsDialog::process_bcvrm(QString &qgisPath,
                                     QString &outputPath,
                                     QString &strError)
{
    mFilesToRemove.clear();
    mFoldersToRemove.clear();
    mPythonFiles.clear();

    QString command=PAFYCTOOLSGUI_COMMAND_BCVRM;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,parameterCode;
    int intValue;
    double dblValue;
    bool okToNumber;
    Parameter* ptrParameter=NULL;

    QString qgisProgramProcess=qgisPath+PAFYCTOOLSGUI_COMMAND_BCVRM_QGIS_PROGRAM;
    if(!QFile::exists(qgisProgramProcess))
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists QGIS processing program:\n%1")
                .arg(qgisProgramProcess);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_INPUT_SHAPEFILE;
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
        strError+=QObject::tr("\nFor parameter: %1 not exists file:\n%2")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    QString inputShapefileName=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_VARIABLE;
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
    QString fieldName=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_ERROR_MEASURE;
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
    QString errorMeasure=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_GSD;
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
    bool okToDbl=false;
    dblValue=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1 value is not a double: %2")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    double gsd=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_CROSS_VALIDATION;
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
    QStringList strValues=strValue.split(":");
    QString strCrossValidation=strValues.at(0);
    int crossValidation=strCrossValidation.toInt();

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_CROSS_VALIDATION_SUBSAMPLES;
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
    bool okToInt=false;
    intValue=strValue.toInt(&okToInt);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1 value is not an integer: %2")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    int crossValidationSubsamples=intValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_BCVRM_TAG_OUTPUT_GEOTIFF;
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
    QString outputGeoTIFF=strValue;
    if(outputGeoTIFF.isEmpty())
    {
        outputGeoTIFF=outputPath+"/";
        outputGeoTIFF+=QFileInfo(inputShapefileName).completeBaseName();
        outputGeoTIFF+="_";
        QString auxFieldName=fieldName.replace(" ","_");
        outputGeoTIFF+=auxFieldName;
        outputGeoTIFF+=".tif";
    }
    if(QFile::exists(outputGeoTIFF))
    {
        if(!QFile::remove(outputGeoTIFF))
        {
            strError=functionName;
            strError+=QObject::tr("\nError removing existing output GeoTiff:\n%1")
                    .arg(outputGeoTIFF);
            return(false);
        }
    }

    QString temporalPath=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_BCVRM_TEMPORAL_PATH;
    QDir currentDir=QDir::current();
    if(currentDir.exists(temporalPath))
    {
        if(!removeDir(temporalPath))
        {
            strError=functionName;
            strError+=QObject::tr("\nError removing existing temporal path:\n%1")
                    .arg(outputGeoTIFF);
            return(false);
        }
    }
    if(!currentDir.mkpath(temporalPath))
    {
        strError=functionName;
        strError+=QObject::tr("\nError creating temporal path:\n%1")
                .arg(outputGeoTIFF);
        return(false);
    }
    mFoldersToRemove.push_back(temporalPath);
    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_BCVRM_PROCESS_FILE;
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
    QString centroidsShapefileName=temporalPath+"/centroids.shp";
    QString regionShapefileName=temporalPath+"/region.shp";
    QString predictionFileName=temporalPath+"/prediction.sdat";
    QString varianceFileName=temporalPath+"/variance.sdat";
    QString cvsFileName=temporalPath+"/cvs.shp";
    QString cvrFileName=temporalPath+"/cvr.shp";
    QTextStream strOut(&file);
    strOut<<"echo off"<<"\n";
    strOut<<"set OUTPUT_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"cd /d \""<<outputPath<<"\"\n";
    strOut<<"ogr2ogr -f \"ESRI Shapefile\" \"";
    strOut<<centroidsShapefileName<<"\" \"";
    strOut<<inputShapefileName<<"\"  -dialect sqlite -sql \"SELECT ST_Centroid(geometry), * FROM ";
    strOut<<QFileInfo(inputShapefileName).completeBaseName()<<"\"\n";
    strOut<<"ogr2ogr -f \"ESRI Shapefile\" \"";
    strOut<<regionShapefileName<<"\" \"";
    strOut<<inputShapefileName<<"\"  -dialect sqlite -sql \"SELECT ST_ConvexHULL(ST_Union(geometry)), * FROM ";
    strOut<<QFileInfo(inputShapefileName).completeBaseName()<<"\"\n";
    strOut<<"call \""<<qgisProgramProcess<<"\" run saga:ordinarykriging";
    strOut<<" --TARGET_DEFINITION=0";
    strOut<<" --POINTS=\""<<centroidsShapefileName<<"\"";
    strOut<<" --FIELD=\""<<fieldName<<"\"";
    strOut<<" --TARGET_USER_SIZE="<<QString::number(gsd,'f',2);
    strOut<<" --TQUALITY=0";
    strOut<<" --VAR_MAXDIST=0.0";
    strOut<<" --VAR_NCLASSES=100";
    strOut<<" --VAR_NSKIP=1";
    strOut<<" --VAR_MODEL=\"a + b * x\"";
    strOut<<" --LOG=false";
    strOut<<" --BLOCK=false";
    strOut<<" --DBLOCK=100.0";
    strOut<<" --CV_METHOD="<<QString::number(crossValidation);
    strOut<<" --CV_SAMPLES="<<QString::number(crossValidationSubsamples);
    strOut<<" --SEARCH_RANGE=1";
    strOut<<" --SEARCH_RADIUS=1000.0";
    strOut<<" --SEARCH_POINTS_ALL=1";
    strOut<<" --SEARCH_POINTS_MIN=16";
    strOut<<" --SEARCH_POINTS_MAX=20";
    strOut<<" --PREDICTION=\""<<predictionFileName<<"\"";
    strOut<<" --VARIANCE=\""<<varianceFileName<<"\"";
    strOut<<" --CV_SUMMARY=\""<<cvsFileName<<"\"";
    strOut<<" --CV_RESIDUALS=\""<<cvrFileName<<"\"";
    strOut<<"\n";
    strOut<<"gdalwarp -cutline \""<<regionShapefileName<<"\" \""<<predictionFileName<<"\" ";
    strOut<<"\""<<outputGeoTIFF<<"\""<<"\n";
    file.close();

    mFilesToRemove.push_back(processFileName);

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

bool PAFyCToolsDialog::process_cmgccvol(QString &qgisPath,
                                        QString &outputPath,
                                        QString &strError)
{
    mFilesToRemove.clear();
    QString command=PAFYCTOOLSGUI_COMMAND_CMGCCVOL;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,framesShapefile,outputShapefile,parameterCode;
    QString strAuxError,dateFormat,segmentationMethod,methodData,dataFieldName;
    int intValue,numberOfClustersForKmeans;
    double dblValue,cropsMininumValue,minimumValueForPercentile;
    bool okToNumber;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_FRAMES_SHAPEFILE;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_OUTPUT_SHAPEFILE;
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
//        QString strDateTime=QDateTime::toString("yyyyddMM_hhmmss");
//        QFileInfo fileInfo(framesShapefile);
//        QString auxPath=fileInfo.absolutePath();
//        QString auxCompleteBaseName=fileInfo.completeBaseName();
//        QString auxSuffix=fileInfo.completeSuffix();
//        QString newShapefile=auxPath+"/"+auxCompleteBaseName;
//        newShapefile+="_";
//        newShapefile+=strDateTime;
//        newShapefile+=".";
//        newShapefile+=auxSuffix;
//        copyShapefile(framesShapefile,newShapefile);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_CROPS_MINIMUM_VALUE;
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
    cropsMininumValue=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_SEGMENTATION_METHOD;
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
    segmentationMethod=strValue;
    if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_SEGMENTATION_METHOD_KMEANS,Qt::CaseInsensitive)==0)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_NUMBER_OF_CLUSTERS_FOR_KMEANS;
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
        intValue=strValue.toInt(&okToNumber);
        if(!okToNumber)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        numberOfClustersForKmeans=intValue;
    }
    else if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_SEGMENTATION_METHOD_PERCENTILE,Qt::CaseInsensitive)==0)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_MINIMUM_VALUE_FOR_PERCENTILE;
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
        minimumValueForPercentile=dblValue;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_METHOD_DATA;
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
    methodData=strValue;
    if(methodData.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_METHOD_DATA_GCC,Qt::CaseInsensitive)!=0
            &&methodData.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_METHOD_DATA_VOL,Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nInvalid value for parameter: %1 in file:\n%2")
                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
        strError+=QObject::tr("\nMust be gcc or vol");
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_DATE_FORMAT;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_DATA_FIELD_NAME;
    ptrParameter=mPtrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists parameter: %1 in file:\n%2")
                .arg(parameterCode).arg(mPtrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    dataFieldName=strValue.trimmed();

    mPythonFiles.clear();
    QString pythonFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CMGCCVOL_PYTHON_FILE;
    if(!writePythonProgramCropMonitoringFromPhotogrammetricGeomaticProducts(pythonFileName,
                                                                            strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nError writting python file:\n%1").arg(strAuxError);
        QFile::remove(pythonFileName);
        return(false);
    }
    mPythonFiles.append(pythonFileName);

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CMGCCVOL_PROCESS_FILE;
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
    gcc_or_vol, gcc, kmeans
    --input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates.shp"
 --method_data gcc --crop_minimum_value 0.02 --method_segmentation  kmeans --kmeans_clusters 6 --input_field "220905_gcc"
 --output_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates_gcc_kmeans.shp"
 --date_format="%Y%m%d"

    gcc_or_vol, gcc, percentile
    --input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates.shp"
 --method_data gcc --crop_minimum_value 0.02 --method_segmentation  percentile --percentile_minimum_threshold 0.05
 --input_field "220905_gcc" --output_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates_gcc_percentile.shp"
 --date_format="%Y%m%d"

    gcc_or_vol, vol, kmeans
    --input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates.shp"
 --method_data vol --crop_minimum_value 0.1 --method_segmentation  kmeans --kmeans_clusters 6 --input_field "220812_vol"
 --output_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates_vol_kmeans.shp"
 --date_format="%Y%m%d"

    gcc_or_vol, vol, percentile
    --input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates.shp"
 --method_data vol --crop_minimum_value 0.1 --method_segmentation  percentile --percentile_minimum_threshold 0.05
 --input_field "220812_vol" --output_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_gcc_vol_allDates_vol_kmeans.shp"
 --date_format="%Y%m%d"
    */

    strOut<<"echo off"<<"\n";
    strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"set TOOL="<<pythonFileName<<"\n";
    strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"python %TOOL% ";
    strOut<<"--input_crops_frames_shp \""<<framesShapefile <<"\" ";
    strOut<<"--output_shp \""<<outputShapefile<<"\" ";
    strOut<<"--date_format="<<dateFormat<<" ";
    strOut<<"--crop_minimum_value "<<QString::number(cropsMininumValue,'f',2)<<" ";
    strOut<<"--method_data "<<methodData<<" ";
    strOut<<"--method_segmentation "<<segmentationMethod<<" ";
    if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_SEGMENTATION_METHOD_KMEANS,Qt::CaseInsensitive)==0)
    {
        strOut<<"--kmeans_clusters "<<QString::number(numberOfClustersForKmeans)<<" ";
    }
    else if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL_TAG_SEGMENTATION_METHOD_PERCENTILE,Qt::CaseInsensitive)==0)
    {
        strOut<<"--percentile_minimum_threshold "<<QString::number(minimumValueForPercentile,'f',2)<<" ";
    }
    strOut<<"--input_field="<<dataFieldName<<"\n";
    file.close();

    //    mFilesToRemove.push_back(processFileName);

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

bool PAFyCToolsDialog::process_cmndvi(QString &qgisPath,
                                      QString &outputPath,
                                      QString &strError)
{
    mFilesToRemove.clear();
    QString command=PAFYCTOOLSGUI_COMMAND_CMNDVI;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,framesShapefile,orthomosaicFile,outputShapefile,parameterCode,str_date;
    QString strAuxError,dateFormat,dateFromOrthomosaicFileStringSeparator,segmentationMethod;
    int intValue,dateTagPositionInOrthomosaicFile,blueBandPosition,greenBandPosition;
    int redBandPosition,nirBandPosition,numberOfClustersForKmeans;
    double dblValue,cropsMininumNDVI,soilRGBMaximumReflectance,factorToReflectance,minimumValueForPercentile;
    bool okToNumber,computeGCC,computeVolume,dateFromOrthomosaciFile;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_FRAMES_SHAPEFILE;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_MULTISPECTRAL_ORTHOMOSAIC_FILE;
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
    orthomosaicFile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_OUTPUT_SHAPEFILE;
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
//        QString strDateTime=QDateTime::toString("yyyyddMM_hhmmss");
//        QFileInfo fileInfo(framesShapefile);
//        QString auxPath=fileInfo.absolutePath();
//        QString auxCompleteBaseName=fileInfo.completeBaseName();
//        QString auxSuffix=fileInfo.completeSuffix();
//        QString newShapefile=auxPath+"/"+auxCompleteBaseName;
//        newShapefile+="_";
//        newShapefile+=strDateTime;
//        newShapefile+=".";
//        newShapefile+=auxSuffix;
//        copyShapefile(framesShapefile,newShapefile);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_CROPS_MINIMUM_NDVI;
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
    cropsMininumNDVI=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_FACTOR_TO_REFLECTANCE;
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
    factorToReflectance=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_SOIL_RGB_MAXIMUM_REFLECTANCE;
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
    soilRGBMaximumReflectance=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_BLUE_BAND_POSITION;
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
    intValue=strValue.toInt(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    blueBandPosition=intValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_GREEN_BAND_POSITION;
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
    intValue=strValue.toInt(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    greenBandPosition=intValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_RED_BAND_POSITION;
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
    intValue=strValue.toInt(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    redBandPosition=intValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_NIR_BAND_POSITION;
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
    intValue=strValue.toInt(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    nirBandPosition=intValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_SEGMENTATION_METHOD;
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
    segmentationMethod=strValue;
    if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_SEGMENTATION_METHOD_KMEANS,Qt::CaseInsensitive)==0)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_NUMBER_OF_CLUSTERS_FOR_KMEANS;
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
        intValue=strValue.toInt(&okToNumber);
        if(!okToNumber)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        numberOfClustersForKmeans=intValue;
    }
    else if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_SEGMENTATION_METHOD_PERCENTILE,Qt::CaseInsensitive)==0)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_MINIMUM_VALUE_FOR_PERCENTILE;
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
        minimumValueForPercentile=dblValue;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_DATE_FROM_MULTISPECTRAL_ORTHOMOSAIC_FILE;
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
    dateFromOrthomosaciFile=false;
    if(strValue.compare("true",Qt::CaseInsensitive)==0)
    {
        dateFromOrthomosaciFile=true;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_MULTISPECTRAL_ORTHOMOSAIC_FILE_TAGS_STRING_SEPARATOR;
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
    dateFromOrthomosaicFileStringSeparator=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_MULTISPECTRAL_ORTHOMOSAIC_FILE_TAG_DATE_POSITION;
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
    dateTagPositionInOrthomosaicFile=strValue.toDouble(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_DATE_FORMAT;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_DATE;
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
    if(dateFromOrthomosaciFile)
    {
        str_date="none";
    }

    mPythonFiles.clear();
    QString pythonFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CMNDVI_PYTHON_FILE;
    if(!writePythonProgramCropMonitoringFromPhotogrammetricGeomaticProducts(pythonFileName,
                                                                            strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nError writting python file:\n%1").arg(strAuxError);
        QFile::remove(pythonFileName);
        return(false);
    }
    mPythonFiles.append(pythonFileName);

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CMNDVI_PROCESS_FILE;
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
--input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_initial.shp"
--method_data ndvi --method_segmentation  kmeans --kmeans_clusters 6
--input_orthomosaic "D:\PAFyCToolsGui\20230630_Tarazona_MULTI_Vid\ORT-303-RFL-Tarazona_Vid_20230630_25830_4cm.tif"
--blue_band_position 1 --green_band_position 2 --red_band_position 3 --nir_band_position 5
--soil_maximum_rgb_reflectance 0.08  --crop_minimum_value 0.5 --factor_to_reflectance 1.0
--output_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_initial_ndvi_kmeans.shp"
--date_from_orthomosaic_file=1 --orthomosaic_file_string_separator="_"
--orthomosaic_file_date_string_position=3 --date_format="%Y%m%d" --date=none

--input_crops_frames_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_initial.shp"
--method_data ndvi --method_segmentation  percentile --percentile_minimum_threshold 0.05
--input_orthomosaic "D:\PAFyCToolsGui\20230630_Tarazona_MULTI_Vid\ORT-303-RFL-Tarazona_Vid_20230630_25830_4cm.tif"
--blue_band_position 1 --green_band_position 2 --red_band_position 3 --nir_band_position 5 --soil_maximum_rgb_reflectance 0.08
--crop_minimum_value 0.5 --factor_to_reflectance 1.0
--output_shp "D:\PAFyCToolsGui\20220426_Tarazona_Vid_A6000\output\vines_frames_initial_ndvi_percentile.shp"
 --date_from_orthomosaic_file=1 --orthomosaic_file_string_separator="_" --orthomosaic_file_date_string_position=3
--date_format="%Y%m%d" --date=none
    */

    strOut<<"echo off"<<"\n";
    strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"set TOOL="<<pythonFileName<<"\n";
    strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"python %TOOL% ";
    strOut<<"--input_crops_frames_shp \""<<framesShapefile <<"\" ";
    strOut<<"--input_orthomosaic \""<<orthomosaicFile <<"\" ";
    strOut<<"--output_shp \""<<outputShapefile<<"\" ";
    strOut<<"--date_from_orthomosaic_file ";
    if(dateFromOrthomosaciFile) strOut<<"1"<<" ";
    else strOut<<"0"<<" ";
    strOut<<"--orthomosaic_file_string_separator=\""<<dateFromOrthomosaicFileStringSeparator<<"\" ";
    strOut<<"--orthomosaic_file_date_string_position="<<QString::number(dateTagPositionInOrthomosaicFile)<<" ";
    //    dateFormat=dateFormat.replace("%","%%");
    //    strOut<<"--date_format=\""<<dateFormat<<"\" ";
    strOut<<"--date_format="<<dateFormat<<" ";
    strOut<<"--factor_to_reflectance "<<QString::number(factorToReflectance,'f',6)<<" ";
    strOut<<"--crop_minimum_value "<<QString::number(cropsMininumNDVI,'f',2)<<" ";
    strOut<<"--soil_maximum_rgb_reflectance "<<QString::number(soilRGBMaximumReflectance,'f',2)<<" ";
    strOut<<"--blue_band_position "<<QString::number(blueBandPosition)<<" ";
    strOut<<"--green_band_position "<<QString::number(greenBandPosition)<<" ";
    strOut<<"--red_band_position "<<QString::number(redBandPosition)<<" ";
    strOut<<"--nir_band_position "<<QString::number(nirBandPosition)<<" ";
    strOut<<"--method_data ndvi --method_segmentation "<<segmentationMethod<<" ";
    if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_SEGMENTATION_METHOD_KMEANS,Qt::CaseInsensitive)==0)
    {
        strOut<<"--kmeans_clusters "<<QString::number(numberOfClustersForKmeans)<<" ";
    }
    else if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CMNDVI_TAG_SEGMENTATION_METHOD_PERCENTILE,Qt::CaseInsensitive)==0)
    {
        strOut<<"--percentile_minimum_threshold "<<QString::number(minimumValueForPercentile,'f',2)<<" ";
    }
    if(str_date.compare("none",Qt::CaseInsensitive)==0)
    {
        strOut<<"--date="<<str_date<<"\n";
    }
    else
    {
        strOut<<"--date=\""<<str_date<<"\"\n";
    }
    file.close();

//    mFilesToRemove.push_back(processFileName);

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

bool PAFyCToolsDialog::process_cwsitho(QString &qgisPath,
                                       QString &outputPath,
                                       QString &strError)
{
    mFilesToRemove.clear();
    QString command=PAFYCTOOLSGUI_COMMAND_CWSITHO;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,framesShapefile,orthomosaicFile,outputShapefile,parameterCode,str_date;
    QString strAuxError,dateFormat,dateFromOrthomosaicFileStringSeparator,segmentationMethod;
    int intValue,dateTagPositionInOrthomosaicFile,numberOfClustersForKmeans;
    double upperLineCoefA,upperLineCoefB,lowerLineCoefA,lowerLineCoefB;
    double dblValue,factorToTemperature,maximumValueForPercentile;
    double temperature,relativeHumidity;
    bool okToNumber,dateFromOrthomosaciFile;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_FRAMES_SHAPEFILE;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_THERMAL_ORTHOMOSAIC_FILE;
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
    orthomosaicFile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_OUTPUT_SHAPEFILE;
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
//        QString strDateTime=QDateTime::toString("yyyyddMM_hhmmss");
//        QFileInfo fileInfo(framesShapefile);
//        QString auxPath=fileInfo.absolutePath();
//        QString auxCompleteBaseName=fileInfo.completeBaseName();
//        QString auxSuffix=fileInfo.completeSuffix();
//        QString newShapefile=auxPath+"/"+auxCompleteBaseName;
//        newShapefile+="_";
//        newShapefile+=strDateTime;
//        newShapefile+=".";
//        newShapefile+=auxSuffix;
//        copyShapefile(framesShapefile,newShapefile);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_AIR_TEMPERATURE;
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
    temperature=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_RELATIVE_HUMIDITY;
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
    relativeHumidity=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_FRAMES_UPPER_BASE_LINE_COEFFICIENT_A;
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
    upperLineCoefA=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_FRAMES_UPPER_BASE_LINE_COEFFICIENT_B;
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
    upperLineCoefB=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_FRAMES_LOWER_BASE_LINE_COEFFICIENT_A;
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
    lowerLineCoefA=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_FRAMES_LOWER_BASE_LINE_COEFFICIENT_B;
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
    lowerLineCoefB=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_FACTOR_TO_TEMPERATURE;
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
    factorToTemperature=dblValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_SEGMENTATION_METHOD;
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
    segmentationMethod=strValue;
    if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_SEGMENTATION_METHOD_KMEANS,Qt::CaseInsensitive)==0)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_NUMBER_OF_CLUSTERS_FOR_KMEANS;
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
        intValue=strValue.toInt(&okToNumber);
        if(!okToNumber)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        numberOfClustersForKmeans=intValue;
    }
    else if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_SEGMENTATION_METHOD_PERCENTILE,Qt::CaseInsensitive)==0)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_MAXIMUM_VALUE_FOR_PERCENTILE;
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
        maximumValueForPercentile=dblValue;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_DATE_FROM_THERMAL_ORTHOMOSAIC_FILE;
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
    dateFromOrthomosaciFile=false;
    if(strValue.compare("true",Qt::CaseInsensitive)==0)
    {
        dateFromOrthomosaciFile=true;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_THERMAL_ORTHOMOSAIC_FILE_TAGS_STRING_SEPARATOR;
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
    dateFromOrthomosaicFileStringSeparator=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_THERMAL_ORTHOMOSAIC_FILE_TAG_DATE_POSITION;
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
    dateTagPositionInOrthomosaicFile=strValue.toDouble(&okToNumber);
    if(!okToNumber)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_DATE_FORMAT;
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

    parameterCode=PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_DATE;
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
    if(dateFromOrthomosaciFile)
    {
        str_date="none";
    }

    mPythonFiles.clear();
    QString pythonFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CWSITHO_PYTHON_FILE;
    if(!writePythonProgramCropWaterStressIndexUsingThermalOrthomosaic(pythonFileName,
                                                                      strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nError writting python file:\n%1").arg(strAuxError);
        QFile::remove(pythonFileName);
        return(false);
    }
    mPythonFiles.append(pythonFileName);

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_CWSITHO_PROCESS_FILE;
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
    --input_crops_frames_shp "D:\PAFyCToolsGui\20230825_Tarazona_Thermal_CWSI\input\Shape_cepas.shp"
    --method_segmentation kmeans --kmeans_clusters 3
    --input_orthomosaic "D:\PAFyCToolsGui\20230825_Tarazona_Thermal_CWSI\input\ORT_FTA_Tarazona_P001_20230825_4258_5782_66_mm.tif"
    --temperature 25.6 --relative_humidity 30.0 --upper_line_coef_a 0.221000 --upper_line_coef_b 6.671
    --lower_line_coef_a -2.0168 --lower_line_coef_b 2.70228 --factor_to_temperature 0.01
    --output_shp "D:\PAFyCToolsGui\20230825_Tarazona_Thermal_CWSI\output\Shape_cepas_cwsi.shp"
    --date_from_orthomosaic_file=1 --orthomosaic_file_string_separator="_"
    --orthomosaic_file_date_string_position=5 --date_format="%Y%m%d" --date=none
    */
    strOut<<"echo off"<<"\n";
    strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"set TOOL="<<pythonFileName<<"\n";
    strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"python %TOOL% ";
    strOut<<"--input_crops_frames_shp \""<<framesShapefile <<"\" ";
    strOut<<"--method_segmentation "<<segmentationMethod<<" ";
    if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_SEGMENTATION_METHOD_KMEANS,Qt::CaseInsensitive)==0)
    {
        strOut<<"--kmeans_clusters "<<QString::number(numberOfClustersForKmeans)<<" ";
    }
    else if(segmentationMethod.compare(PAFYCTOOLSGUI_COMMAND_CWSITHO_TAG_SEGMENTATION_METHOD_PERCENTILE,Qt::CaseInsensitive)==0)
    {
        strOut<<"--percentile_minimum_threshold "<<QString::number(maximumValueForPercentile,'f',2)<<" ";
    }
    strOut<<"--input_orthomosaic \""<<orthomosaicFile <<"\" ";
    strOut<<"--temperature "<<QString::number(temperature,'f',2)<<" ";
    strOut<<"--relative_humidity "<<QString::number(relativeHumidity,'f',1)<<" ";
    strOut<<"--upper_line_coef_a "<<QString::number(upperLineCoefA,'f',6)<<" ";
    strOut<<"--upper_line_coef_b "<<QString::number(upperLineCoefB,'f',3)<<" ";
    strOut<<"--lower_line_coef_a "<<QString::number(lowerLineCoefA,'f',6)<<" ";
    strOut<<"--lower_line_coef_b "<<QString::number(lowerLineCoefB,'f',3)<<" ";
    strOut<<"--factor_to_temperature "<<QString::number(factorToTemperature,'f',6)<<" ";
    strOut<<"--output_shp \""<<outputShapefile<<"\" ";
    strOut<<"--date_from_orthomosaic_file ";
    if(dateFromOrthomosaciFile) strOut<<"1"<<" ";
    else strOut<<"0"<<" ";
    strOut<<"--orthomosaic_file_string_separator=\""<<dateFromOrthomosaicFileStringSeparator<<"\" ";
    strOut<<"--orthomosaic_file_date_string_position="<<QString::number(dateTagPositionInOrthomosaicFile)<<" ";
    //    dateFormat=dateFormat.replace("%","%%");
    //    strOut<<"--date_format=\""<<dateFormat<<"\" ";
    strOut<<"--date_format="<<dateFormat<<" ";
    if(str_date.compare("none",Qt::CaseInsensitive)==0)
    {
        strOut<<"--date="<<str_date<<"\n";
    }
    else
    {
        strOut<<"--date=\""<<str_date<<"\"\n";
    }
    file.close();

    //    mFilesToRemove.push_back(processFileName);

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

bool PAFyCToolsDialog::process_szbr(QString &qgisPath,
                                    QString &outputPath,
                                    QString &strError)
{
    mFilesToRemove.clear();
    QString command=PAFYCTOOLSGUI_COMMAND_SZBR;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,parameterCode,inputOrthomosaic,inputShapefile;
    QString strFactorToReflectance,strBandsToUse,strUseOnlyOnePrincipalComponent;
    QString strAuxError,dateFormat,dateFromOrthomosaicFileStringSeparator,segmentationMethod;
    int intValue,noDataValue,redBandPosition,nirBandPosition,maximumNumberOfClustersForKmeans;
    double upperLineCoefA,upperLineCoefB,lowerLineCoefA,lowerLineCoefB;
    double dblValue,minimumNdvi,maximumNdvi,minimumExplainedVariance,factorToReflectance;
    double minimumClassificationArea;
    bool okToNumber,dateFromOrthomosaciFile;
    bool okToInt,okToDbl;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_INPUT_ORTHOMOSAIC;
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
    inputOrthomosaic=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_ORTHOMOSAIC_NO_DATA_VALUE;
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
    okToInt=false;
    noDataValue=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_FACTOR_TO_REFLECTANCE;
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
    okToDbl=false;
    factorToReflectance=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    strFactorToReflectance=strValue;


    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_INPUT_SHAPEFILE;
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
    inputShapefile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_BANDS_TO_USE;
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
    QStringList strValues=strValue.split(";");
    if(strValues.size()<1)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a list of integers separated by ;")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    for(int i=0;i<strValues.size();i++)
    {
        strValue=strValues.at(i);
        okToInt=false;
        int intValue=strValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a list of integers separated by ;")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        if(i>0)
            strBandsToUse+=" ";
        strBandsToUse+=QString::number(intValue);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_RED_BAND_POSITION;
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
    okToInt=false;
    redBandPosition=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_NIR_BAND_POSITION;
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
    okToInt=false;
    nirBandPosition=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_SOIL_MINIMUM_NDVI;
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
    okToDbl=false;
    minimumNdvi=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_SOIL_MAXIMUM_NDVI;
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
    okToDbl=false;
    maximumNdvi=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_MINIMUM_EXPLAINED_VARIANCE;
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
    okToDbl=false;
    minimumExplainedVariance=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    minimumExplainedVariance=minimumExplainedVariance/100.;

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_USE_ONLY_ONE_PRINCIPAL_COMPONENT;
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

    if(strValue.compare("True",Qt::CaseInsensitive)==0)
        strUseOnlyOnePrincipalComponent="1";
    else
        strUseOnlyOnePrincipalComponent="0";

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_MAXIMUM_NUMBER_OF_CLUSTERS_KMEANS;
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
    okToInt=false;
    maximumNumberOfClustersForKmeans=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_MINIMUM_CLASSIFIED_AREA;
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
    okToDbl=false;
    minimumClassificationArea=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }


    parameterCode=PAFYCTOOLSGUI_COMMAND_SZBR_OUTPUT_PATH;
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
        if(!auxDir.mkpath(strValue))
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nnot exists path:\n%2")
                .arg(parameterCode).arg(strValue);
        strError+=QObject::tr("\nand is not possible make it");
        return(false);
    }
    QString resultsPath=strValue;

    mPythonFiles.clear();
    QString pythonFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_SZBR_PYTHON_FILE;
    if(!writePythonProgramSoilZoningBasedInReflectivity(pythonFileName,
                                                        strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nError writting python file:\n%1").arg(strAuxError);
        QFile::remove(pythonFileName);
        return(false);
    }
    mPythonFiles.append(pythonFileName);

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_SZBR_PROCESS_FILE;
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
    --input_orthomosaic "D:\PAFyCToolsGui\20240709_SoilZoningBaseReflectivity\OrtoSeleccion_recortada.tif"
    --no_data_value 65535 --input_rois_shp "D:\PAFyCToolsGui\20240709_SoilZoningBaseReflectivity\Limites_parcela_Hotel.shp"
    --factor_to_reflectance 3.051757812500000e-05 --bands_to_use 1 2 4 5 6 --red_band_number 4
    --nir_band_number 6 --minimum_ndvi -1.0 --maximum_ndvi 0.15 --minimum_explained_variance 0.8
    --only_one_principal_component 1 --max_number_of_kmeans_clusters 5 --minimum_classification_area 5.0
    --output_path ""
    */
    strOut<<"echo off"<<"\n";
    strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"set TOOL="<<pythonFileName<<"\n";
    strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"python %TOOL% ";
    strOut<<"--input_orthomosaic \""<<inputOrthomosaic<<"\" ";
    strOut<<"--no_data_value "<<QString::number(noDataValue)<<" ";
    strOut<<"--input_rois_shp \""<<inputShapefile<<"\" ";
    strOut<<"--factor_to_reflectance  "<<strFactorToReflectance<<" ";
    strOut<<"--bands_to_use "<<strBandsToUse<<" ";
    strOut<<"--red_band_number "<<QString::number(redBandPosition)<<" ";
    strOut<<"--nir_band_number "<<QString::number(nirBandPosition)<<" ";
    strOut<<"--minimum_ndvi "<<QString::number(minimumNdvi,'f',2)<<" ";
    strOut<<"--maximum_ndvi "<<QString::number(maximumNdvi,'f',2)<<" ";
    strOut<<"--minimum_explained_variance "<<QString::number(minimumExplainedVariance,'f',1)<<" ";
    strOut<<"--only_one_principal_component "<<strUseOnlyOnePrincipalComponent<<" ";
    strOut<<"--max_number_of_kmeans_clusters "<<QString::number(maximumNumberOfClustersForKmeans)<<" ";
    strOut<<"--minimum_classification_area "<<QString::number(minimumClassificationArea,'f',2)<<" ";
    strOut<<"--output_path \"";
    if(!resultsPath.isEmpty())
        strOut<<resultsPath;
    strOut<<"\"";
    file.close();

    //    mFilesToRemove.push_back(processFileName);

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

bool PAFyCToolsDialog::process_mfha(QString &qgisPath, QString &outputPath, QString &strError)
{
    mFilesToRemove.clear();
    QString command=PAFYCTOOLSGUI_COMMAND_MFHA;
    QString functionName=QObject::tr("Proccessing command:\n%1").arg(command);
    QString strValue,parameterCode,inputOrthomosaic,inputShapefile;
    QString strFactorToReflectance,strBandsToUse,strUseOnlyOnePrincipalComponent;
    QString strAuxError,dateFormat,dateFromOrthomosaicFileStringSeparator,strGridSpacing;
    int intValue,noDataValue,redBandPosition,nirBandPosition,numberOfClusters;
    double dblValue,minimumNdvi,minimumExplainedVariance,factorToReflectance;
    double gridSpacing,minimumNirReflectance;
    QString strWeightFactorByCluster;
    bool okToNumber,dateFromOrthomosaciFile;
    bool okToInt,okToDbl;
    Parameter* ptrParameter=NULL;
    QDir auxDir=QDir::currentPath();

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_INPUT_ORTHOMOSAIC;
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
    inputOrthomosaic=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_ORTHOMOSAIC_NO_DATA_VALUE;
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
    okToInt=false;
    noDataValue=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_GRID_SPACING;
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
    okToDbl=false;
    gridSpacing=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    strGridSpacing=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_FACTOR_TO_REFLECTANCE;
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
    okToDbl=false;
    factorToReflectance=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    strFactorToReflectance=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_INPUT_SHAPEFILE;
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
    inputShapefile=strValue;

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_BANDS_TO_USE;
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
    QStringList strValues=strValue.split(";");
    if(strValues.size()<1)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a list of integers separated by ;")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    for(int i=0;i<strValues.size();i++)
    {
        strValue=strValues.at(i);
        okToInt=false;
        int intValue=strValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a list of integers separated by ;")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        if(i>0)
            strBandsToUse+=" ";
        strBandsToUse+=QString::number(intValue);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_QUALITY_FACTOR_BY_CLUSTER;
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
    strValues=strValue.split(";");
    if(strValues.size()<1)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a list of doubles separated by ;")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    numberOfClusters=0;
    strWeightFactorByCluster="";
    for(int i=0;i<strValues.size();i++)
    {
        strValue=strValues.at(i);
        okToInt=false;
        double dblValue=strValue.toDouble(&okToDbl);
        if(!okToDbl)
        {
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a list of doubles separated by ;")
                    .arg(parameterCode).arg(strValue);
            return(false);
        }
        if(i>0)
            strWeightFactorByCluster+=" ";
        strWeightFactorByCluster+=QString::number(dblValue);
        numberOfClusters++;
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_RED_BAND_POSITION;
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
    okToInt=false;
    redBandPosition=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_NIR_BAND_POSITION;
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
    okToInt=false;
    nirBandPosition=strValue.toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not an integer")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_VEGETATION_MINIMUM_NDVI;
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
    okToDbl=false;
    minimumNdvi=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_VEGETATION_MINIMUM_NIR_REFLECTANCE;
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
    okToDbl=false;
    minimumNirReflectance=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_MINIMUM_EXPLAINED_VARIANCE;
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
    okToDbl=false;
    minimumExplainedVariance=strValue.toDouble(&okToDbl);
    if(!okToDbl)
    {
        strError=functionName;
        strError+=QObject::tr("\nFor parameter: %1\nvalue: %2 is not a double")
                .arg(parameterCode).arg(strValue);
        return(false);
    }
    minimumExplainedVariance=minimumExplainedVariance/100.;

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_USE_ONLY_ONE_PRINCIPAL_COMPONENT;
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

    if(strValue.compare("True",Qt::CaseInsensitive)==0)
        strUseOnlyOnePrincipalComponent="1";
    else
        strUseOnlyOnePrincipalComponent="0";

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_TAG_CROP_MINIMUM_HEIGHT;
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
    double cropsMininumHeight=dblValue;
    QString dsmFile,dtmFile;

    if(cropsMininumHeight>0.)
    {
        parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_TAG_DSM_FILE;
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

        parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_TAG_DTM_FILE;
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
    }

    parameterCode=PAFYCTOOLSGUI_COMMAND_MFHA_OUTPUT_PATH;
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
        if(!auxDir.exists(strValue))
        {
            if(!auxDir.mkpath(strValue))
            strError=functionName;
            strError+=QObject::tr("\nFor parameter: %1\nnot exists path:\n%2")
                    .arg(parameterCode).arg(strValue);
            strError+=QObject::tr("\nand is not possible make it");
            return(false);
        }
    }
    QString resultsPath=strValue;

    mPythonFiles.clear();
    QString pythonFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_MFHA_PYTHON_FILE;
    if(!writePythonProgramMonitoringFloraAtHighAltitude(pythonFileName,
                                                        strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nError writting python file:\n%1").arg(strAuxError);
        QFile::remove(pythonFileName);
        return(false);
    }
    mPythonFiles.append(pythonFileName);

    QString processFileName=outputPath+"/"+PAFYCTOOLSGUI_COMMAND_MFHA_PROCESS_FILE;
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
    --input_orthomosaic "D:/PAFyCToolsGui/20241218_Cebreros/20240814_Cebreros_25830.tif"
    --no_data_value -32767 --input_rois_shp "D:/PAFyCToolsGui/20241218_Cebreros/cebreros_mfha_roi.shp"
    --factor_to_reflectance 3.051757812500000e-05 --bands_to_use 1 2 4 5 6 --red_band_number 4
    --nir_band_number 6 --minimum_ndvi 0.2 --minimum_nir_reflectance 0.3 --minimum_explained_variance 0.8
    --only_one_principal_component 1 --weight_factor_by_cluster 1.0 2.0 3.0 4.0 --grid_spacing 1.0
    --input_dsm "D:/PAFyCToolsGui/20241218_Cebreros/20240814_Cebreros_25830_DSM.tif"
    --input_dtm "D:/PAFyCToolsGui/20241218_Cebreros/20240814_Cebreros_25830_DTM.tif"
    --crop_minimum_height 0.05
    --output_path "D:/PAFyCToolsGui/20241218_Cebreros/output"
    */
    strOut<<"echo off"<<"\n";
    strOut<<"set PROCESS_PATH="<<outputPath<<"\n";
    strOut<<"set OSGEO4W_ROOT="<<qgisPath<<"\n";
    strOut<<"set TOOL="<<pythonFileName<<"\n";
    strOut<<"cd /d \"%PROCESS_PATH%\""<<"\n";
    strOut<<"call \"%OSGEO4W_ROOT%\\bin\\o4w_env.bat\""<<"\n";
    strOut<<"python %TOOL% ";
    strOut<<"--input_orthomosaic \""<<inputOrthomosaic<<"\" ";
    strOut<<"--no_data_value "<<QString::number(noDataValue)<<" ";
    strOut<<"--input_rois_shp \""<<inputShapefile<<"\" ";
    strOut<<"--factor_to_reflectance  "<<strFactorToReflectance<<" ";
    strOut<<"--bands_to_use "<<strBandsToUse<<" ";
    strOut<<"--red_band_number "<<QString::number(redBandPosition)<<" ";
    strOut<<"--nir_band_number "<<QString::number(nirBandPosition)<<" ";
    strOut<<"--minimum_ndvi "<<QString::number(minimumNdvi,'f',2)<<" ";
    strOut<<"--minimum_nir_reflectance "<<QString::number(minimumNirReflectance,'f',2)<<" ";
    strOut<<"--minimum_explained_variance "<<QString::number(minimumExplainedVariance,'f',1)<<" ";
    strOut<<"--only_one_principal_component "<<strUseOnlyOnePrincipalComponent<<" ";
    strOut<<"--weight_factor_by_cluster "<<strWeightFactorByCluster<<" ";
    strOut<<"--grid_spacing "<<strGridSpacing<<" ";
    strOut<<"--crop_minimum_height "<<QString::number(cropsMininumHeight,'f',2)<<" ";
    strOut<<"--input_dsm \""<<dsmFile<<"\" ";
    strOut<<"--input_dtm \""<<dtmFile<<"\" ";
    strOut<<"--output_path \"";
    if(!resultsPath.isEmpty())
        strOut<<resultsPath;
    strOut<<"\"";
    file.close();

    //    mFilesToRemove.push_back(processFileName);

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
    strOut<<"def copy_shapefile(input_shp, output_shp):"<<"\n";
    strOut<<"    str_error = ''"<<"\n";
    strOut<<"    input_base_name = os.path.splitext(os.path.basename(input_shp))[0]"<<"\n";
    strOut<<"    input_base_path = os.path.dirname(input_shp)"<<"\n";
    strOut<<"    output_base_path = os.path.dirname(output_shp)"<<"\n";
    strOut<<"    output_base_name = os.path.splitext(os.path.basename(output_shp))[0]"<<"\n";
    strOut<<"    for file in os.listdir(input_base_path):"<<"\n";
    strOut<<"        file_base_name = os.path.splitext(os.path.basename(file))[0]"<<"\n";
    strOut<<"        if file_base_name == input_base_name:"<<"\n";
    strOut<<"            file_extension = os.path.splitext(os.path.basename(file))[1]"<<"\n";
    strOut<<"            output_file = output_base_path + \"/\" + output_base_name  + file_extension"<<"\n";
    strOut<<"            output_file = os.path.normcase(output_file)"<<"\n";
    strOut<<"            input_file = input_base_path + \"/\" + file"<<"\n";
    strOut<<"            input_file = os.path.normcase(input_file)"<<"\n";
    strOut<<"            try:"<<"\n";
    strOut<<"                shutil.copyfile(input_file, output_file)"<<"\n";
    strOut<<"            except EnvironmentError as e:"<<"\n";
    strOut<<"                str_error = \"Unable to copy file. %s\" % e"<<"\n";
    strOut<<"                return str_error"<<"\n";
    strOut<<"    return str_error"<<"\n";
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
    strOut<<"        in_vec_ds = driver.Open(input_shp, 1)  # 0 means read-only. 1 means writeable."<<"\n";
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
    strOut<<"    field_name_gcc = str_date + \"_gcc\""<<"\n";
    strOut<<"    field_name_vol = str_date + \"_vol\""<<"\n";
    strOut<<"    in_layer_definition = in_layer.GetLayerDefn()"<<"\n";
    strOut<<"    if compute_GCC:"<<"\n";
    strOut<<"        field_id_index = in_layer_definition.GetFieldIndex(field_name_gcc)"<<"\n";
    strOut<<"        if field_id_index == -1:"<<"\n";
    strOut<<"            in_layer.CreateField(ogr.FieldDefn(field_name_gcc, ogr.OFTReal))"<<"\n";
    strOut<<"    if compute_volume:"<<"\n";
    strOut<<"        field_id_index = in_layer_definition.GetFieldIndex(field_name_vol)"<<"\n";
    strOut<<"        if field_id_index == -1:"<<"\n";
    strOut<<"            in_layer.CreateField(ogr.FieldDefn(field_name_vol, ogr.OFTReal))"<<"\n";
    strOut<<"    in_layer_definition = in_layer.GetLayerDefn()"<<"\n";
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
    strOut<<"        in_layer.SetFeature(feature)"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
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
    strOut<<"    if not options.input_crops_frames_shp:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.input_dsm:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.input_dtm:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if options.crop_minimum_height == None:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.output_shp:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if options.compute_GCC == None:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if options.compute_volume == None:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if options.date_from_dem_files == None:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.dem_files_string_separator:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.dem_files_date_string_position:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.date_format:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.date:"<<"\n";
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
    strOut<<"        dsm_file_name_without_path = os.path.splitext(os.path.basename(input_dsm))[0]"<<"\n";
    strOut<<"        dsm_file_name_values = dsm_file_name_without_path.split(dem_files_string_separator)"<<"\n";
    strOut<<"        if dem_files_date_string_position < 0 or dem_files_date_string_position > len(dsm_file_name_values):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for dsm files date string position: {}\"."<<"\n";
    strOut<<"                  format(str(dem_files_date_string_position)))"<<"\n";
    strOut<<"        str_date = dsm_file_name_values[dem_files_date_string_position-1]"<<"\n";
    strOut<<"        date_dsm = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"        dtm_file_name_without_path = os.path.splitext(os.path.basename(input_dtm))[0]"<<"\n";
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
    strOut<<"    input_shp = input_crops_shapefile"<<"\n";
    strOut<<"    if not use_input_shp:"<<"\n";
    strOut<<"        str_error = copy_shapefile(input_crops_shapefile, output_shp)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_shp = output_shp"<<"\n";
    strOut<<"    str_error = process(input_shp,"<<"\n";
    strOut<<"                        input_dsm,"<<"\n";
    strOut<<"                        input_dtm,"<<"\n";
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

bool PAFyCToolsDialog::writePythonProgramCropMonitoringFromPhotogrammetricGeomaticProducts(QString pythonFileName,
                                                                                           QString &strError)
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
    strOut<<"import cv2 as cv"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def copy_shapefile(input_shp, output_shp):"<<"\n";
    strOut<<"    str_error = ''"<<"\n";
    strOut<<"    input_base_name = os.path.splitext(os.path.basename(input_shp))[0]"<<"\n";
    strOut<<"    input_base_path = os.path.dirname(input_shp)"<<"\n";
    strOut<<"    output_base_path = os.path.dirname(output_shp)"<<"\n";
    strOut<<"    output_base_name = os.path.splitext(os.path.basename(output_shp))[0]"<<"\n";
    strOut<<"    for file in os.listdir(input_base_path):"<<"\n";
    strOut<<"        file_base_name = os.path.splitext(os.path.basename(file))[0]"<<"\n";
    strOut<<"        if file_base_name == input_base_name:"<<"\n";
    strOut<<"            file_extension = os.path.splitext(os.path.basename(file))[1]"<<"\n";
    strOut<<"            output_file = output_base_path + \"/\" + output_base_name  + file_extension"<<"\n";
    strOut<<"            output_file = os.path.normcase(output_file)"<<"\n";
    strOut<<"            input_file = input_base_path + \"/\" + file"<<"\n";
    strOut<<"            input_file = os.path.normcase(input_file)"<<"\n";
    strOut<<"            try:"<<"\n";
    strOut<<"                shutil.copyfile(input_file, output_file)"<<"\n";
    strOut<<"            except EnvironmentError as e:"<<"\n";
    strOut<<"                str_error = \"Unable to copy file. %s\" % e"<<"\n";
    strOut<<"                return str_error"<<"\n";
    strOut<<"    return str_error"<<"\n";
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
    strOut<<"def sortFunction(e):"<<"\n";
    strOut<<"    return e['value']"<<"\n";
    strOut<<""<<"\n";
    strOut<<"def process_gcc_or_vol(input_shp,"<<"\n";
    strOut<<"                       crop_minimum_value,"<<"\n";
    strOut<<"                       kmeans_clusters,"<<"\n";
    strOut<<"                       percentile_minimum_threshold,"<<"\n";
    strOut<<"                       gcc,"<<"\n";
    strOut<<"                       vol,"<<"\n";
    strOut<<"                       input_field_name,"<<"\n";
    strOut<<"                       date_format):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    if kmeans_clusters < 0 and percentile_minimum_threshold < 0:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nkmeans_clusters or percentile_minimum_threshold must be greather than 0\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    elif kmeans_clusters > 0 and percentile_minimum_threshold > 0:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nkmeans_clusters or percentile_minimum_threshold must be greather than 0\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    driver = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        in_vec_ds = driver.Open(input_shp, 1)  # 0 means read-only. 1 means writeable."<<"\n";
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
    strOut<<"    in_layer_definition = in_layer.GetLayerDefn()"<<"\n";
    strOut<<"    input_field_id_index = in_layer_definition.GetFieldIndex(input_field_name)"<<"\n";
    strOut<<"    if input_field_id_index == -1:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists field: {} in file:\\n{}\".format(input_field_name, input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    output_field_name = None"<<"\n";
    strOut<<"    if gcc:"<<"\n";
    strOut<<"        output_field_name = '_dg'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name = '_dv'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name = output_field_name + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name = output_field_name + 'p'"<<"\n";
    strOut<<"    str_date = None"<<"\n";
    strOut<<"    if '_' in input_field_name:"<<"\n";
    strOut<<"        str_values = input_field_name.split('_')"<<"\n";
    strOut<<"        for i in range(len(str_values)):"<<"\n";
    strOut<<"            str_value = str_values[i]"<<"\n";
    strOut<<"            is_date = True"<<"\n";
    strOut<<"            if len(str_value) == 6:"<<"\n";
    strOut<<"                str_value = '20' + str_value"<<"\n";
    strOut<<"            try:"<<"\n";
    strOut<<"                date = datetime.datetime.strptime(str_value, date_format)"<<"\n";
    strOut<<"            except ValueError as error:"<<"\n";
    strOut<<"                is_date = False"<<"\n";
    strOut<<"            if is_date:"<<"\n";
    strOut<<"                str_date = str(date.strftime('%Y')[2:4]) + str(date.strftime('%m')) + str(date.strftime('%d'))"<<"\n";
    strOut<<"                break"<<"\n";
    strOut<<"    if str_date:"<<"\n";
    strOut<<"        output_field_name = str_date + output_field_name"<<"\n";
    strOut<<"    output_field_id_index = in_layer_definition.GetFieldIndex(output_field_name)"<<"\n";
    strOut<<"    if output_field_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name, ogr.OFTInteger))#ogr.OFTReal))"<<"\n";
    strOut<<"    cont_feature = 0"<<"\n";
    strOut<<"    input_values = []"<<"\n";
    strOut<<"    position_in_input_values_by_feature_position = {}"<<"\n";
    strOut<<"    for feature in in_layer:"<<"\n";
    strOut<<"        value = feature.GetFieldAsDouble(input_field_id_index)"<<"\n";
    strOut<<"        if value >= crop_minimum_value:"<<"\n";
    strOut<<"            input_value = {}"<<"\n";
    strOut<<"            input_value['position'] = cont_feature"<<"\n";
    strOut<<"            input_value['value'] = value"<<"\n";
    strOut<<"            input_values.append(input_value)"<<"\n";
    strOut<<"            position_in_input_values_by_feature_position[cont_feature] = len(input_values) - 1"<<"\n";
    strOut<<"        cont_feature = cont_feature + 1"<<"\n";
    strOut<<"    number_of_features = in_layer.GetFeatureCount()"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        input_values_cv = numpy.zeros([len(input_values), 1], dtype=numpy.float32)"<<"\n";
    strOut<<"        cont_feature_crop = 0"<<"\n";
    strOut<<"        for input_value in input_values:"<<"\n";
    strOut<<"            input_values_cv[cont_feature_crop][0] = input_value['value']"<<"\n";
    strOut<<"            cont_feature_crop = cont_feature_crop + 1"<<"\n";
    strOut<<"        # Define criteria = ( type, max_iter = 10 , epsilon = 1.0 )"<<"\n";
    strOut<<"        # criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 10, 1.0)"<<"\n";
    strOut<<"        criteria = (cv.TERM_CRITERIA_MAX_ITER, 100, 1.0)"<<"\n";
    strOut<<"        flags = cv.KMEANS_RANDOM_CENTERS"<<"\n";
    strOut<<"        compactness, labels, centers = cv.kmeans(input_values_cv, kmeans_clusters,"<<"\n";
    strOut<<"                                                 None, criteria, 10, flags)"<<"\n";
    strOut<<"        pos_center_min_value = -1"<<"\n";
    strOut<<"        center_min_value = 100000000."<<"\n";
    strOut<<"        for i in range(kmeans_clusters):"<<"\n";
    strOut<<"            if centers[i] < center_min_value:"<<"\n";
    strOut<<"                center_min_value = centers[i]"<<"\n";
    strOut<<"                pos_center_min_value = i"<<"\n";
    strOut<<"        cont_feature = 0"<<"\n";
    strOut<<"        for feature in in_layer:"<<"\n";
    strOut<<"            damaged = 0"<<"\n";
    strOut<<"            if not cont_feature in position_in_input_values_by_feature_position:"<<"\n";
    strOut<<"                damaged = -1"<<"\n";
    strOut<<"            else:"<<"\n";
    strOut<<"                pos_in_input_values = position_in_input_values_by_feature_position[cont_feature]"<<"\n";
    strOut<<"                pos_center = labels[position_in_input_values_by_feature_position[cont_feature]][0]"<<"\n";
    strOut<<"                if pos_center == pos_center_min_value:"<<"\n";
    strOut<<"                    damaged = 1"<<"\n";
    strOut<<"            cont_feature = cont_feature + 1"<<"\n";
    strOut<<"            feature.SetField(output_field_name, damaged)"<<"\n";
    strOut<<"            in_layer.SetFeature(feature)"<<"\n";
    strOut<<"    elif percentile_minimum_threshold > 0.:"<<"\n";
    strOut<<"        input_values.sort(key=sortFunction)"<<"\n";
    strOut<<"        damage_positions = []"<<"\n";
    strOut<<"        number_of_damages = 0"<<"\n";
    strOut<<"        threshold_value = -1"<<"\n";
    strOut<<"        for i in range(0, len(input_values)):"<<"\n";
    strOut<<"            damage_positions.append(input_values[i]['position'])"<<"\n";
    strOut<<"            if number_of_damages / number_of_features > percentile_minimum_threshold:"<<"\n";
    strOut<<"                threshold_value = input_values[i]['value']"<<"\n";
    strOut<<"                break"<<"\n";
    strOut<<"            number_of_damages = number_of_damages + 1"<<"\n";
    strOut<<"        cont_feature = 0"<<"\n";
    strOut<<"        for feature in in_layer:"<<"\n";
    strOut<<"            damaged = 0"<<"\n";
    strOut<<"            if not cont_feature in position_in_input_values_by_feature_position:"<<"\n";
    strOut<<"                damaged = -1"<<"\n";
    strOut<<"            else:"<<"\n";
    strOut<<"                if cont_feature in damage_positions:"<<"\n";
    strOut<<"                    damaged = 1"<<"\n";
    strOut<<"            cont_feature = cont_feature + 1"<<"\n";
    strOut<<"            feature.SetField(output_field_name, damaged)"<<"\n";
    strOut<<"            in_layer.SetFeature(feature)"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    return str_error"<<"\n";
    strOut<<""<<"\n";
    strOut<<"def process_ndvi(input_shp,"<<"\n";
    strOut<<"                 kmeans_clusters,"<<"\n";
    strOut<<"                 percentile_minimum_threshold,"<<"\n";
    strOut<<"                 input_orthomosaic,"<<"\n";
    strOut<<"                 blue_band_position,"<<"\n";
    strOut<<"                 green_band_position,"<<"\n";
    strOut<<"                 red_band_position,"<<"\n";
    strOut<<"                 nir_band_position,"<<"\n";
    strOut<<"                 factor_to_reflectance,"<<"\n";
    strOut<<"                 soil_maximum_rgb_reflectance,"<<"\n";
    strOut<<"                 crop_minimum_value,"<<"\n";
    strOut<<"                 str_date):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    if kmeans_clusters < 0 and percentile_minimum_threshold < 0:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nkmeans_clusters or percentile_minimum_threshold must be greather than 0\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    elif kmeans_clusters > 0 and percentile_minimum_threshold > 0:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nkmeans_clusters or percentile_minimum_threshold must be greather than 0\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds = gdal.Open(input_orthomosaic)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    # orthomosaic_number_of_bands = orthomosaic_ds.GetRasterCount()"<<"\n";
    strOut<<"    # if blue_band_position > orthomosaic_number_of_bands:"<<"\n";
    strOut<<"    #     str_error = \"Function process\""<<"\n";
    strOut<<"    #     str_error += \"\\nBlue band position is greather than orthomosaic number of bands\""<<"\n";
    strOut<<"    #     return str_error"<<"\n";
    strOut<<"    orthomosaic_ds_rb_blue = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds_rb_blue = orthomosaic_ds.GetRasterBand(blue_band_position)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting BLUE raster band from file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_ds_rb_green = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds_rb_green = orthomosaic_ds.GetRasterBand(green_band_position)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting GREEN raster band from file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_ds_rb_red = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds_rb_red = orthomosaic_ds.GetRasterBand(red_band_position)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting RED raster band from file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_ds_rb_nir = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds_rb_nir = orthomosaic_ds.GetRasterBand(nir_band_position)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting NIR raster band from file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_geotransform = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    orthomosaic_crs = osr.SpatialReference()"<<"\n";
    strOut<<"    orthomosaic_crs.ImportFromWkt(orthomosaic_ds.GetProjectionRef())"<<"\n";
    strOut<<"    orthomosaic_crs_wkt = orthomosaic_crs.ExportToWkt()"<<"\n";
    strOut<<"    ulx, xres, xskew, uly, yskew, yres = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    lrx = ulx + (orthomosaic_ds.RasterXSize * xres)"<<"\n";
    strOut<<"    lry = uly + (orthomosaic_ds.RasterYSize * yres)"<<"\n";
    strOut<<"    out_ring = ogr.Geometry(ogr.wkbLinearRing)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    orthomosaic_poly = ogr.Geometry(ogr.wkbPolygon)"<<"\n";
    strOut<<"    orthomosaic_poly.AddGeometry(out_ring)"<<"\n";
    strOut<<"    rs_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    rs_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    orthomosaic_pixel_area = abs(rs_pixel_width) * abs(rs_pixel_height)"<<"\n";
    strOut<<"    orthomosaic_x_origin = orthomosaic_geotransform[0]"<<"\n";
    strOut<<"    orthomosaic_y_origin = orthomosaic_geotransform[3]"<<"\n";
    strOut<<"    orthomosaic_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    orthomosaic_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    driver = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        in_vec_ds = driver.Open(input_shp, 1)  # 0 means read-only. 1 means writeable."<<"\n";
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
    strOut<<"    in_layer_definition = in_layer.GetLayerDefn()"<<"\n";
    strOut<<"    number_of_features = in_layer.GetFeatureCount()"<<"\n";
    strOut<<"    cont_feature = 0"<<"\n";
    strOut<<"    input_values = []"<<"\n";
    strOut<<"    position_in_input_values_by_feature_position = {}"<<"\n";

    strOut<<"    output_field_name = str_date"<<"\n";
    strOut<<"    output_field_name = output_field_name + '_' + 'dn'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name = output_field_name + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name = output_field_name + 'p'"<<"\n";
    strOut<<"    output_field_id_index = in_layer_definition.GetFieldIndex(output_field_name)"<<"\n";
    strOut<<"    if output_field_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name, ogr.OFTInteger))#ogr.OFTReal))"<<"\n";
    strOut<<"    output_field_ndvi_name = str_date"<<"\n";
    strOut<<"    output_field_ndvi_name = output_field_ndvi_name + '_' + 'ni'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_ndvi_name = output_field_ndvi_name + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_ndvi_name = output_field_ndvi_name + 'p'"<<"\n";
    strOut<<"    output_field_ndvi_id_index = in_layer_definition.GetFieldIndex(output_field_ndvi_name)"<<"\n";
    strOut<<"    if output_field_ndvi_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_ndvi_name, ogr.OFTReal))"<<"\n";
    strOut<<"    for feature in in_layer:"<<"\n";
    strOut<<"        print('Processing plant: {}, of {}'.format(str(cont_feature + 1),"<<"\n";
    strOut<<"                                                   str(number_of_features)))"<<"\n";
    strOut<<"        plot_geometry_full = feature.GetGeometryRef().Clone()"<<"\n";
    strOut<<"        crs_transform = None"<<"\n";
    strOut<<"        if in_crs_wkt != orthomosaic_crs_wkt:"<<"\n";
    strOut<<"            crs_transform = osr.CoordinateTransformation(in_crs, orthomosaic_crs)"<<"\n";
    strOut<<"        if crs_transform:"<<"\n";
    strOut<<"            plot_geometry_full.Transform(crs_transform)"<<"\n";
    strOut<<"        plot_geometry = None"<<"\n";
    strOut<<"        if orthomosaic_poly.Overlaps(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = plot_geometry_full.Intersection(orthomosaic_poly)"<<"\n";
    strOut<<"        if orthomosaic_poly.Contains(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = plot_geometry_full"<<"\n";
    strOut<<"        if orthomosaic_poly.Within(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = orthomosaic_poly"<<"\n";
    strOut<<"        if not plot_geometry:"<<"\n";
    strOut<<"            continue"<<"\n";
    strOut<<"        plot_geometry = plot_geometry_full.Intersection(orthomosaic_poly)"<<"\n";
    strOut<<"        plot_geometry_area = plot_geometry.GetArea()"<<"\n";
    strOut<<"        if plot_geometry_area < (3 * orthomosaic_pixel_area):"<<"\n";
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
    strOut<<"        rs_x_off = int((plot_geom_x_min - orthomosaic_x_origin) / rs_pixel_width)"<<"\n";
    strOut<<"        rs_y_off = int((orthomosaic_y_origin - plot_geom_y_max) / rs_pixel_width)"<<"\n";
    strOut<<"        x_ul = orthomosaic_x_origin + rs_x_off * rs_pixel_width"<<"\n";
    strOut<<"        y_ul = orthomosaic_y_origin - rs_y_off * rs_pixel_width"<<"\n";
    strOut<<"        rs_x_count = int((plot_geom_x_max - plot_geom_x_min) / rs_pixel_width) + 1"<<"\n";
    strOut<<"        rs_y_count = int((plot_geom_y_max - plot_geom_y_min) / rs_pixel_width) + 1"<<"\n";
    strOut<<"        # Create memory target raster"<<"\n";
    strOut<<"        target_orthomosaic = gdal.GetDriverByName('MEM').Create('', rs_x_count, rs_y_count, 1, gdal.GDT_Byte)"<<"\n";
    strOut<<"        target_orthomosaic.SetGeoTransform(("<<"\n";
    strOut<<"            plot_geom_x_min, rs_pixel_width, 0,"<<"\n";
    strOut<<"            plot_geom_y_max, 0, rs_pixel_height,"<<"\n";
    strOut<<"        ))"<<"\n";
    strOut<<"        # Create for target raster the same projection as for the value raster"<<"\n";
    strOut<<"        raster_srs = osr.SpatialReference()"<<"\n";
    strOut<<"        raster_srs.ImportFromWkt(orthomosaic_ds.GetProjectionRef())"<<"\n";
    strOut<<"        target_orthomosaic.SetProjection(raster_srs.ExportToWkt())"<<"\n";
    strOut<<"        target_orthomosaic.SetProjection(raster_srs.ExportToWkt())"<<"\n";
    strOut<<"        feature_drv = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"        feature_ds= feature_drv.CreateDataSource(\"/vsimem/memory_name.shp\")"<<"\n";
    strOut<<"        # geometryType = plot_geometry.getGeometryType()"<<"\n";
    strOut<<"        feature_layer = feature_ds.CreateLayer(\"layer\", orthomosaic_crs, geom_type=plot_geometry.GetGeometryType())"<<"\n";
    strOut<<"        featureDefnHeaders = feature_layer.GetLayerDefn()"<<"\n";
    strOut<<"        out_feature = ogr.Feature(featureDefnHeaders)"<<"\n";
    strOut<<"        out_feature.SetGeometry(plot_geometry)"<<"\n";
    strOut<<"        feature_layer.CreateFeature(out_feature)"<<"\n";
    strOut<<"        feature_ds.FlushCache()"<<"\n";
    strOut<<"        # Rasterize zone polygon to raster blue"<<"\n";
    strOut<<"        gdal.RasterizeLayer(target_orthomosaic, [1], feature_layer, burn_values=[1])"<<"\n";
    strOut<<"        feature_orthomosaic_band_mask = target_orthomosaic.GetRasterBand(1)"<<"\n";
    strOut<<"        feature_orthomosaic_data_mask = (feature_orthomosaic_band_mask.ReadAsArray(0, 0,"<<"\n";
    strOut<<"                                                                                  rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                         .astype(float))"<<"\n";
    strOut<<"        # Mask zone of raster blue"<<"\n";
    strOut<<"        feature_orthomosaic_data_blue = (orthomosaic_ds_rb_blue.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                    .astype(float))"<<"\n";
    strOut<<"        feature_dsm_raster_array_blue = numpy.ma.masked_array(feature_orthomosaic_data_blue,"<<"\n";
    strOut<<"                                                         numpy.logical_not(feature_orthomosaic_data_mask))"<<"\n";
    strOut<<"        orthomosaic_first_indexes_blue, orthomosaic_second_indexes_blue = feature_dsm_raster_array_blue.nonzero()"<<"\n";
    strOut<<"        # Mask zone of raster green"<<"\n";
    strOut<<"        feature_orthomosaic_data_green = (orthomosaic_ds_rb_green.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                    .astype(float))"<<"\n";
    strOut<<"        feature_dsm_raster_array_green = numpy.ma.masked_array(feature_orthomosaic_data_green,"<<"\n";
    strOut<<"                                                         numpy.logical_not(feature_orthomosaic_data_mask))"<<"\n";
    strOut<<"        orthomosaic_first_indexes_green, orthomosaic_second_indexes_green = feature_dsm_raster_array_green.nonzero()"<<"\n";
    strOut<<"        # Mask zone of raster red"<<"\n";
    strOut<<"        feature_orthomosaic_data_red = (orthomosaic_ds_rb_red.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                    .astype(float))"<<"\n";
    strOut<<"        feature_dsm_raster_array_red = numpy.ma.masked_array(feature_orthomosaic_data_red,"<<"\n";
    strOut<<"                                                         numpy.logical_not(feature_orthomosaic_data_mask))"<<"\n";
    strOut<<"        orthomosaic_first_indexes_red, orthomosaic_second_indexes_red = feature_dsm_raster_array_red.nonzero()"<<"\n";
    strOut<<"        # Mask zone of raster nir"<<"\n";
    strOut<<"        feature_orthomosaic_data_nir = (orthomosaic_ds_rb_nir.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                    .astype(float))"<<"\n";
    strOut<<"        feature_dsm_raster_array_nir = numpy.ma.masked_array(feature_orthomosaic_data_nir,"<<"\n";
    strOut<<"                                                         numpy.logical_not(feature_orthomosaic_data_mask))"<<"\n";
    strOut<<"        orthomosaic_first_indexes_nir, orthomosaic_second_indexes_nir = feature_dsm_raster_array_nir.nonzero()"<<"\n";
    strOut<<"        ndvi_mean = 0."<<"\n";
    strOut<<"        ndvi_min = 2."<<"\n";
    strOut<<"        ndvi_max = -2."<<"\n";
    strOut<<"        ndvi_number_of_values = 0"<<"\n";
    strOut<<"        for i in range(len(orthomosaic_first_indexes_blue)):"<<"\n";
    strOut<<"            fi = orthomosaic_first_indexes_blue[i]"<<"\n";
    strOut<<"            si = orthomosaic_second_indexes_blue[i]"<<"\n";
    strOut<<"            if (not fi in orthomosaic_first_indexes_green"<<"\n";
    strOut<<"                    or not fi in orthomosaic_first_indexes_red"<<"\n";
    strOut<<"                    or not fi in orthomosaic_first_indexes_nir):"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            if (not si in orthomosaic_second_indexes_green"<<"\n";
    strOut<<"                    or not si in orthomosaic_second_indexes_red"<<"\n";
    strOut<<"                    or not si in orthomosaic_second_indexes_nir):"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            blue = feature_dsm_raster_array_blue[fi][si] * factor_to_reflectance"<<"\n";
    strOut<<"            green = feature_dsm_raster_array_green[fi][si] * factor_to_reflectance"<<"\n";
    strOut<<"            red = feature_dsm_raster_array_red[fi][si] * factor_to_reflectance"<<"\n";
    strOut<<"            nir = feature_dsm_raster_array_nir[fi][si] * factor_to_reflectance"<<"\n";
    strOut<<"            if (blue < soil_maximum_rgb_reflectance"<<"\n";
    strOut<<"                    and green < soil_maximum_rgb_reflectance"<<"\n";
    strOut<<"                    and red < soil_maximum_rgb_reflectance):"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            ndvi = (nir - red) / (red + nir)"<<"\n";
    strOut<<"            if ndvi < crop_minimum_value:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            ndvi_mean = ndvi_mean + ndvi"<<"\n";
    strOut<<"            if ndvi < ndvi_min:"<<"\n";
    strOut<<"                ndvi_min = ndvi"<<"\n";
    strOut<<"            if ndvi > ndvi_max:"<<"\n";
    strOut<<"                ndvi_max = ndvi"<<"\n";
    strOut<<"            ndvi_number_of_values = ndvi_number_of_values + 1"<<"\n";
    strOut<<"        if ndvi_number_of_values > 0:"<<"\n";
    strOut<<"            ndvi_mean = ndvi_mean / ndvi_number_of_values"<<"\n";
    strOut<<"            input_value = {}"<<"\n";
    strOut<<"            input_value['position'] = cont_feature"<<"\n";
    strOut<<"            input_value['value'] = ndvi_mean"<<"\n";
    strOut<<"            input_values.append(input_value)"<<"\n";
    strOut<<"            position_in_input_values_by_feature_position[cont_feature] = len(input_values) - 1"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            ndvi_mean = -1"<<"\n";
    strOut<<"        cont_feature = cont_feature + 1"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        input_values_cv = numpy.zeros([len(input_values), 1], dtype=numpy.float32)"<<"\n";
    strOut<<"        cont_feature_crop = 0"<<"\n";
    strOut<<"        for input_value in input_values:"<<"\n";
    strOut<<"            input_values_cv[cont_feature_crop][0] = input_value['value']"<<"\n";
    strOut<<"            cont_feature_crop = cont_feature_crop + 1"<<"\n";
    strOut<<"        # Define criteria = ( type, max_iter = 10 , epsilon = 1.0 )"<<"\n";
    strOut<<"        # criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 10, 1.0)"<<"\n";
    strOut<<"        criteria = (cv.TERM_CRITERIA_MAX_ITER, 100, 1.0)"<<"\n";
    strOut<<"        flags = cv.KMEANS_RANDOM_CENTERS"<<"\n";
    strOut<<"        compactness, labels, centers = cv.kmeans(input_values_cv, kmeans_clusters,"<<"\n";
    strOut<<"                                                 None, criteria, 10, flags)"<<"\n";
    strOut<<"        pos_center_min_value = -1"<<"\n";
    strOut<<"        center_min_value = 100000000."<<"\n";
    strOut<<"        for i in range(kmeans_clusters):"<<"\n";
    strOut<<"            if centers[i] < center_min_value:"<<"\n";
    strOut<<"                center_min_value = centers[i]"<<"\n";
    strOut<<"                pos_center_min_value = i"<<"\n";
    strOut<<"        cont_feature = 0"<<"\n";
    strOut<<"        for feature in in_layer:"<<"\n";
    strOut<<"            damaged = 0"<<"\n";
    strOut<<"            ndvi = -1"<<"\n";
    strOut<<"            if not cont_feature in position_in_input_values_by_feature_position:"<<"\n";
    strOut<<"                damaged = -1"<<"\n";
    strOut<<"            else:"<<"\n";
    strOut<<"                pos_in_input_values = position_in_input_values_by_feature_position[cont_feature]"<<"\n";
    strOut<<"                pos_center = labels[position_in_input_values_by_feature_position[cont_feature]][0]"<<"\n";
    strOut<<"                if pos_center == pos_center_min_value:"<<"\n";
    strOut<<"                    damaged = 1"<<"\n";
    strOut<<"                ndvi = input_values[position_in_input_values_by_feature_position[cont_feature]]['value']"<<"\n";
    strOut<<"            cont_feature = cont_feature + 1"<<"\n";
    strOut<<"            feature.SetField(output_field_name, damaged)"<<"\n";
    strOut<<"            feature.SetField(output_field_ndvi_name, ndvi)"<<"\n";
    strOut<<"            in_layer.SetFeature(feature)"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        input_values.sort(key=sortFunction)"<<"\n";
    strOut<<"        damage_positions = []"<<"\n";
    strOut<<"        number_of_damages = 0"<<"\n";
    strOut<<"        threshold_value = -1"<<"\n";
    strOut<<"        for i in range(0, len(input_values)):"<<"\n";
    strOut<<"            damage_positions.append(input_values[i]['position'])"<<"\n";
    strOut<<"            if number_of_damages / number_of_features > percentile_minimum_threshold:"<<"\n";
    strOut<<"                threshold_value = input_values[i]['value']"<<"\n";
    strOut<<"                break"<<"\n";
    strOut<<"            number_of_damages = number_of_damages + 1"<<"\n";
    strOut<<"        cont_feature = 0"<<"\n";
    strOut<<"        for feature in in_layer:"<<"\n";
    strOut<<"            damaged = 0"<<"\n";
    strOut<<"            ndvi = -1"<<"\n";
    strOut<<"            if not cont_feature in position_in_input_values_by_feature_position:"<<"\n";
    strOut<<"                damaged = -1"<<"\n";
    strOut<<"            else:"<<"\n";
    strOut<<"                if cont_feature in damage_positions:"<<"\n";
    strOut<<"                    damaged = 1"<<"\n";
    strOut<<"                ndvi = input_values[position_in_input_values_by_feature_position[cont_feature]]['value']"<<"\n";
    strOut<<"            cont_feature = cont_feature + 1"<<"\n";
    strOut<<"            feature.SetField(output_field_name, damaged)"<<"\n";
    strOut<<"            in_layer.SetFeature(feature)"<<"\n";
    strOut<<"            feature.SetField(output_field_ndvi_name, ndvi)"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    return str_error"<<"\n";
    strOut<<""<<"\n";
    strOut<<"def main():"<<"\n";
    strOut<<"    # =================="<<"\n";
    strOut<<"    # parse command line"<<"\n";
    strOut<<"    # =================="<<"\n";
    strOut<<"    usage = \"usage: %prog [options] \""<<"\n";
    strOut<<"    parser = OptionParser(usage=usage)"<<"\n";
    strOut<<"    parser.add_option(\"--input_crops_frames_shp\", dest=\"input_crops_frames_shp\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Crops frames shapefile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--method_data\", dest=\"method_data\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Method input data type: gcc, vol or ndvi\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--method_segmentation\", dest=\"method_segmentation\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Method segmentation: kmeans or percentile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--crop_minimum_value\", dest=\"crop_minimum_value\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Crop minimum value: gcc (per unit), vol (dm3) or NDVI (per unit)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--kmeans_clusters\", dest=\"kmeans_clusters\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Number of cluster for kmeans segmentation\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--percentile_minimum_threshold\", dest=\"percentile_minimum_threshold\", action=\"store\","<<"\n";
    strOut<<"                      type=\"string\", help=\"Minimum value (per unit) for percentile segmentation\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--input_orthomosaic\", dest=\"input_orthomosaic\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Input multispectral orthomosaic, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--blue_band_position\", dest=\"blue_band_position\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Blue band position in multispectral orthomosaic, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--green_band_position\", dest=\"green_band_position\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Green band position in multispectral orthomosaic, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--red_band_position\", dest=\"red_band_position\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Red band position in multispectral orthomosaic, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--nir_band_position\", dest=\"nir_band_position\", action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Nir band position in multispectral orthomosaic, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--factor_to_reflectance\", dest=\"factor_to_reflectance\", action=\"store\","<<"\n";
    strOut<<"                      type=\"string\", help=\"Factor for convert band values to reflectance (per unit), for ndvi method\","<<"\n";
    strOut<<"                      default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--soil_maximum_rgb_reflectance\", dest=\"soil_maximum_rgb_reflectance\", action=\"store\","<<"\n";
    strOut<<"                      type=\"string\", help=\"Soil maximum RGB reflectance (per unit) from orthomosaic, for ndvi method\","<<"\n";
    strOut<<"                      default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--output_shp\", dest=\"output_shp\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Output shapefile or none for use input shapefile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date_from_orthomosaic_file\", dest=\"date_from_orthomosaic_file\", action=\"store\","<<"\n";
    strOut<<"                      type=\"int\", help=\"Read date from orthomosaic file name: 1-yes, 0-No, for ndvi method\","<<"\n";
    strOut<<"                      default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--orthomosaic_file_string_separator\", dest=\"orthomosaic_file_string_separator\","<<"\n";
    strOut<<"                      action=\"store\", type=\"string\", help=\"Orthomosaic file string separator, for ndvi method\","<<"\n";
    strOut<<"                      default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--orthomosaic_file_date_string_position\", dest=\"orthomosaic_file_date_string_position\","<<"\n";
    strOut<<"                      action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Orthomosaic file date string position, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date_format\", dest=\"date_format\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Date format (%Y%m%d, ...)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date\", dest=\"date\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"None or date value no from orthomosaic files, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--input_field\", dest=\"input_field\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Input field name of GCC or VOL, for gcc or vol methods\", default=None)"<<"\n";
    strOut<<"    (options, args) = parser.parse_args()"<<"\n";
    strOut<<"    if not options.input_crops_frames_shp:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_shp = options.input_crops_frames_shp"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        print(\"Error:\\nInput crops shapefile does not exists:\\n{}\".format(input_shp))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.output_shp:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    use_input_shp = True"<<"\n";
    strOut<<"    output_shp = options.output_shp"<<"\n";
    strOut<<"    if output_shp != 'none':"<<"\n";
    strOut<<"        use_input_shp = False"<<"\n";
    strOut<<"    if not options.method_segmentation:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    method_segmentation = options.method_segmentation"<<"\n";
    strOut<<"    kmeans_clusters = -1"<<"\n";
    strOut<<"    percentile_minimum_threshold = -1."<<"\n";
    strOut<<"    if method_segmentation == 'kmeans':"<<"\n";
    strOut<<"        if not options.kmeans_clusters:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_kmeans_clusters = options.kmeans_clusters"<<"\n";
    strOut<<"        if not is_number(str_kmeans_clusters):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for kmeans clusters: {}\"."<<"\n";
    strOut<<"                  format(str_kmeans_clusters))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        kmeans_clusters = int(str_kmeans_clusters)"<<"\n";
    strOut<<"        if kmeans_clusters < 2 or kmeans_clusters > 20:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for kmeans clusters: {}\"."<<"\n";
    strOut<<"                  format(str_kmeans_clusters))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    elif method_segmentation == 'percentile':"<<"\n";
    strOut<<"        if not options.percentile_minimum_threshold:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_percentile_minimum_threshold = options.percentile_minimum_threshold"<<"\n";
    strOut<<"        if not is_number(str_percentile_minimum_threshold):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for percentile minimum threshold: {}\"."<<"\n";
    strOut<<"                  format(str_percentile_minimum_threshold))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        percentile_minimum_threshold = float(str_percentile_minimum_threshold)"<<"\n";
    strOut<<"        if percentile_minimum_threshold < 0 or percentile_minimum_threshold > 1:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for percentile minimum threshold: {}\"."<<"\n";
    strOut<<"                  format(str_percentile_minimum_threshold))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    if kmeans_clusters < 0 and percentile_minimum_threshold < 0:"<<"\n";
    strOut<<"        print(\"Error:\\nMethod segmentation must be: kmeans or percentile\")"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.method_data:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    method_data = options.method_data"<<"\n";
    strOut<<"    ndvi = False"<<"\n";
    strOut<<"    gcc = False"<<"\n";
    strOut<<"    vol = False"<<"\n";
    strOut<<"    if method_data == 'ndvi':"<<"\n";
    strOut<<"        ndvi = True"<<"\n";
    strOut<<"    elif method_data == 'gcc':"<<"\n";
    strOut<<"        gcc = True"<<"\n";
    strOut<<"    elif method_data == 'vol':"<<"\n";
    strOut<<"        vol = True"<<"\n";
    strOut<<"    if not ndvi and not gcc and not vol:"<<"\n";
    strOut<<"        print(\"Error:\\nMethod data must be: ndvi, gcc or vol\")"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.crop_minimum_value:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_crop_minimum_value = options.crop_minimum_value"<<"\n";
    strOut<<"    if not is_number(str_crop_minimum_value):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for crop minimum value: {}\"."<<"\n";
    strOut<<"              format(str_crop_minimum_value))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    crop_minimum_value = float(str_crop_minimum_value)"<<"\n";
    strOut<<"    if ndvi:"<<"\n";
    strOut<<"        if crop_minimum_value < 0 or crop_minimum_value > 1:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for crop minimum NDVI value: {}\"."<<"\n";
    strOut<<"                  format(str_crop_minimum_value))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    elif gcc:"<<"\n";
    strOut<<"        if crop_minimum_value < 0 or crop_minimum_value > 10:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for crop minimum GCC value: {}\"."<<"\n";
    strOut<<"                  format(str_crop_minimum_value))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    elif vol:"<<"\n";
    strOut<<"        if crop_minimum_value < 0 or crop_minimum_value > 10000:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for crop minimum VOL value: {}\"."<<"\n";
    strOut<<"                  format(str_crop_minimum_value))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    if not options.date_format:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    date_format = options.date_format.strip()"<<"\n";
    strOut<<"    input_field = None"<<"\n";
    strOut<<"    if not use_input_shp:"<<"\n";
    strOut<<"        str_error = copy_shapefile(input_shp, output_shp)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_shp = output_shp"<<"\n";
    strOut<<"    if gcc or vol:"<<"\n";
    strOut<<"        if not options.input_field:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_field = options.input_field"<<"\n";
    strOut<<"        str_error = process_gcc_or_vol(input_shp,"<<"\n";
    strOut<<"                                       crop_minimum_value,"<<"\n";
    strOut<<"                                       kmeans_clusters,"<<"\n";
    strOut<<"                                       percentile_minimum_threshold,"<<"\n";
    strOut<<"                                       gcc,"<<"\n";
    strOut<<"                                       vol,"<<"\n";
    strOut<<"                                       input_field,"<<"\n";
    strOut<<"                                       date_format)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        print(\"... Process finished\")"<<"\n";
    strOut<<"    elif ndvi:"<<"\n";
    strOut<<"        if not options.input_orthomosaic:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_orthomosaic = options.input_orthomosaic"<<"\n";
    strOut<<"        if not exists(input_orthomosaic):"<<"\n";
    strOut<<"            print(\"Error:\\nInput orthomosaic does not exists:\\n{}\".format(input_orthomosaic))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        if not options.blue_band_position:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_blue_band_position = options.blue_band_position"<<"\n";
    strOut<<"        if not is_number(str_blue_band_position):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for blue band position: {}\"."<<"\n";
    strOut<<"                  format(str_blue_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        blue_band_position = int(str_blue_band_position)"<<"\n";
    strOut<<"        if blue_band_position < 1 or blue_band_position > 8:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for blue band position: {}\"."<<"\n";
    strOut<<"                  format(str_blue_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_green_band_position = options.green_band_position"<<"\n";
    strOut<<"        if not is_number(str_green_band_position):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for green band position: {}\"."<<"\n";
    strOut<<"                  format(str_green_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        green_band_position = int(str_green_band_position)"<<"\n";
    strOut<<"        if green_band_position < 1 or green_band_position > 8:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for green band position: {}\"."<<"\n";
    strOut<<"                  format(str_green_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        if green_band_position == blue_band_position:"<<"\n";
    strOut<<"            print(\"Error:\\nBand positions must be different\")"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_red_band_position = options.red_band_position"<<"\n";
    strOut<<"        if not is_number(str_red_band_position):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for green band position: {}\"."<<"\n";
    strOut<<"                  format(str_red_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        red_band_position = int(str_red_band_position)"<<"\n";
    strOut<<"        if red_band_position < 1 or red_band_position > 8:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for green band position: {}\"."<<"\n";
    strOut<<"                  format(str_red_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        if red_band_position == blue_band_position or red_band_position == green_band_position:"<<"\n";
    strOut<<"            print(\"Error:\\nBand positions must be different\")"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_nir_band_position = options.nir_band_position"<<"\n";
    strOut<<"        if not is_number(str_nir_band_position):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for green band position: {}\"."<<"\n";
    strOut<<"                  format(str_nir_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        nir_band_position = int(str_nir_band_position)"<<"\n";
    strOut<<"        if nir_band_position < 1 or nir_band_position > 8:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for green band position: {}\"."<<"\n";
    strOut<<"                  format(str_nir_band_position))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        if nir_band_position == blue_band_position or nir_band_position == green_band_position \\"<<"\n";
    strOut<<"                or nir_band_position == red_band_position:"<<"\n";
    strOut<<"            print(\"Error:\\nBand positions must be different\")"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        if not options.soil_maximum_rgb_reflectance:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_factor_to_reflectance = options.factor_to_reflectance"<<"\n";
    strOut<<"        if not is_number(str_factor_to_reflectance):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for factor to reflectance: {}\"."<<"\n";
    strOut<<"                  format(str_factor_to_reflectance))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        factor_to_reflectance = float(str_factor_to_reflectance)"<<"\n";
    strOut<<"        if factor_to_reflectance < 0 or factor_to_reflectance > 10000.:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for factor to reflectance: {}\"."<<"\n";
    strOut<<"                  format(str_factor_to_reflectance))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_soil_maximum_rgb_reflectance = options.soil_maximum_rgb_reflectance"<<"\n";
    strOut<<"        if not is_number(str_soil_maximum_rgb_reflectance):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for percentile minimum threshold: {}\"."<<"\n";
    strOut<<"                  format(str_soil_maximum_rgb_reflectance))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        soil_maximum_rgb_reflectance = float(str_soil_maximum_rgb_reflectance)"<<"\n";
    strOut<<"        if soil_maximum_rgb_reflectance < 0 or soil_maximum_rgb_reflectance > 1:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for soil maximum RGB reflectance: {}\"."<<"\n";
    strOut<<"                  format(str_soil_maximum_rgb_reflectance))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        if options.date_from_orthomosaic_file == None:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        date_from_orthomosaic = False"<<"\n";
    strOut<<"        if options.date_from_orthomosaic_file == 1:"<<"\n";
    strOut<<"            date_from_orthomosaic = True"<<"\n";
    strOut<<"        date = None"<<"\n";
    strOut<<"        if not date_from_orthomosaic:"<<"\n";
    strOut<<"            if options.date == 'none':"<<"\n";
    strOut<<"                print(\"Error:\\nDate must be a value if not read from orthomosaic file name\")"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"            str_date = options.date"<<"\n";
    strOut<<"            is_date = True"<<"\n";
    strOut<<"            if len(options.date) == 6:"<<"\n";
    strOut<<"                str_date = '20' + str_date"<<"\n";
    strOut<<"            try:"<<"\n";
    strOut<<"                date = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"            except ValueError as error:"<<"\n";
    strOut<<"                is_date = False"<<"\n";
    strOut<<"            if not is_date:"<<"\n";
    strOut<<"                print(\"Error:\\nInvalid string date from orthomosaic name: {} and format: {}\"."<<"\n";
    strOut<<"                      format(options.date, date_format))"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            if not options.orthomosaic_file_string_separator:"<<"\n";
    strOut<<"                parser.print_help()"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"            orthomosaic_file_string_separator = options.orthomosaic_file_string_separator"<<"\n";
    strOut<<"            if not options.orthomosaic_file_date_string_position:"<<"\n";
    strOut<<"                parser.print_help()"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"            orthomosaic_file_date_string_position = options.orthomosaic_file_date_string_position"<<"\n";
    strOut<<"            orthomosaic_file_name_without_path = os.path.splitext(os.path.basename(input_orthomosaic))[0]"<<"\n";
    strOut<<"            orthomosaic_file_name_values = orthomosaic_file_name_without_path.split(orthomosaic_file_string_separator)"<<"\n";
    strOut<<"            if (orthomosaic_file_date_string_position < 0"<<"\n";
    strOut<<"                    or orthomosaic_file_date_string_position > len(orthomosaic_file_name_values)):"<<"\n";
    strOut<<"                print(\"Error:\\nInvalid value for orthomosaic files date string position: {}\"."<<"\n";
    strOut<<"                      format(str(orthomosaic_file_date_string_position)))"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"            str_date = orthomosaic_file_name_values[orthomosaic_file_date_string_position - 1]"<<"\n";
    strOut<<"            is_date = True"<<"\n";
    strOut<<"            if len(str_date) == 6:"<<"\n";
    strOut<<"                str_date = '20' + str_date"<<"\n";
    strOut<<"            try:"<<"\n";
    strOut<<"                date = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"            except ValueError as error:"<<"\n";
    strOut<<"                is_date = False"<<"\n";
    strOut<<"            if not is_date:"<<"\n";
    strOut<<"                print(\"Error:\\nInvalid string date from orthomosaic name: {} and format: {}\"."<<"\n";
    strOut<<"                      format(orthomosaic_file_name_values[orthomosaic_file_date_string_position - 1],"<<"\n";
    strOut<<"                             date_format))"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"        str_date = str(date.strftime('%Y')[2:4]) + str(date.strftime('%m')) + str(date.strftime('%d'))"<<"\n";
    strOut<<"        str_error = process_ndvi(input_shp,"<<"\n";
    strOut<<"                                 kmeans_clusters,"<<"\n";
    strOut<<"                                 percentile_minimum_threshold,"<<"\n";
    strOut<<"                                 input_orthomosaic,"<<"\n";
    strOut<<"                                 blue_band_position,"<<"\n";
    strOut<<"                                 green_band_position,"<<"\n";
    strOut<<"                                 red_band_position,"<<"\n";
    strOut<<"                                 nir_band_position,"<<"\n";
    strOut<<"                                 factor_to_reflectance,"<<"\n";
    strOut<<"                                 soil_maximum_rgb_reflectance,"<<"\n";
    strOut<<"                                 crop_minimum_value,"<<"\n";
    strOut<<"                                 str_date)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        print(\"... Process finished\")"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"if __name__ == '__main__':"<<"\n";
    strOut<<"    main()"<<"\n";

    file.close();
    return(true);
}

bool PAFyCToolsDialog::writePythonProgramCropWaterStressIndexUsingThermalOrthomosaic(QString pythonFileName,
                                                                                     QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_CWSITHO;
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
    strOut<<"import cv2 as cv"<<"\n";
    strOut<<"import math"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def copy_shapefile(input_shp, output_shp):"<<"\n";
    strOut<<"    str_error = ''"<<"\n";
    strOut<<"    input_base_name = os.path.splitext(os.path.basename(input_shp))[0]"<<"\n";
    strOut<<"    input_base_path = os.path.dirname(input_shp)"<<"\n";
    strOut<<"    output_base_path = os.path.dirname(output_shp)"<<"\n";
    strOut<<"    output_base_name = os.path.splitext(os.path.basename(output_shp))[0]"<<"\n";
    strOut<<"    for file in os.listdir(input_base_path):"<<"\n";
    strOut<<"        file_base_name = os.path.splitext(os.path.basename(file))[0]"<<"\n";
    strOut<<"        if file_base_name == input_base_name:"<<"\n";
    strOut<<"            file_extension = os.path.splitext(os.path.basename(file))[1]"<<"\n";
    strOut<<"            output_file = output_base_path + \"/\" + output_base_name + file_extension"<<"\n";
    strOut<<"            output_file = os.path.normcase(output_file)"<<"\n";
    strOut<<"            input_file = input_base_path + \"/\" + file"<<"\n";
    strOut<<"            input_file = os.path.normcase(input_file)"<<"\n";
    strOut<<"            try:"<<"\n";
    strOut<<"                shutil.copyfile(input_file, output_file)"<<"\n";
    strOut<<"            except EnvironmentError as e:"<<"\n";
    strOut<<"                str_error = \"Unable to copy file. %s\" % e"<<"\n";
    strOut<<"                return str_error"<<"\n";
    strOut<<"    return str_error"<<"\n";
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
    strOut<<"    A = trunc((I - 1867216.25) / 36524.25)"<<"\n";
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
    strOut<<"def sortFunction(e):"<<"\n";
    strOut<<"    return e['value']"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def process_cwsith(input_shp,"<<"\n";
    strOut<<"                   kmeans_clusters,"<<"\n";
    strOut<<"                   percentile_maximum_threshold,"<<"\n";
    strOut<<"                   input_orthomosaic,"<<"\n";
    strOut<<"                   temperature_air,"<<"\n";
    strOut<<"                   relative_humidity,"<<"\n";
    strOut<<"                   upper_line_coef_a,"<<"\n";
    strOut<<"                   upper_line_coef_b,"<<"\n";
    strOut<<"                   lower_line_coef_a,"<<"\n";
    strOut<<"                   lower_line_coef_b,"<<"\n";
    strOut<<"                   factor_to_temperature,"<<"\n";
    strOut<<"                   str_date):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    if kmeans_clusters < 0 and percentile_maximum_threshold < 0:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nkmeans_clusters or percentile_maximum_threshold must be greather than 0\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    elif kmeans_clusters > 0 and percentile_maximum_threshold > 0:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nkmeans_clusters or percentile_maximum_threshold must be greather than 0\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds = gdal.Open(input_orthomosaic)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    # orthomosaic_number_of_bands = orthomosaic_ds.GetRasterCount()"<<"\n";
    strOut<<"    # if orthomosaic_number_of_bands != 1:"<<"\n";
    strOut<<"    #     str_error = \"Function process\""<<"\n";
    strOut<<"    #     str_error += \"\\nOrthomosaic number of bands must be 1\""<<"\n";
    strOut<<"    #     return str_error"<<"\n";
    strOut<<"    orthomosaic_ds_rb = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds_rb = orthomosaic_ds.GetRasterBand(1)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError getting BLUE raster band from file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_geotransform = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    orthomosaic_crs = osr.SpatialReference()"<<"\n";
    strOut<<"    orthomosaic_crs.ImportFromWkt(orthomosaic_ds.GetProjectionRef())"<<"\n";
    strOut<<"    orthomosaic_crs_wkt = orthomosaic_crs.ExportToWkt()"<<"\n";
    strOut<<"    ulx, xres, xskew, uly, yskew, yres = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    lrx = ulx + (orthomosaic_ds.RasterXSize * xres)"<<"\n";
    strOut<<"    lry = uly + (orthomosaic_ds.RasterYSize * yres)"<<"\n";
    strOut<<"    out_ring = ogr.Geometry(ogr.wkbLinearRing)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    orthomosaic_poly = ogr.Geometry(ogr.wkbPolygon)"<<"\n";
    strOut<<"    orthomosaic_poly.AddGeometry(out_ring)"<<"\n";
    strOut<<"    rs_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    rs_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    orthomosaic_pixel_area = abs(rs_pixel_width) * abs(rs_pixel_height)"<<"\n";
    strOut<<"    orthomosaic_x_origin = orthomosaic_geotransform[0]"<<"\n";
    strOut<<"    orthomosaic_y_origin = orthomosaic_geotransform[3]"<<"\n";
    strOut<<"    orthomosaic_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    orthomosaic_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    driver = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        in_vec_ds = driver.Open(input_shp, 1)  # 0 means read-only. 1 means writeable."<<"\n";
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
    strOut<<"    in_layer_definition = in_layer.GetLayerDefn()"<<"\n";
    strOut<<"    number_of_features = in_layer.GetFeatureCount()"<<"\n";
    strOut<<"    cont_feature = 0"<<"\n";
    strOut<<"    input_values = []"<<"\n";
    strOut<<"    position_in_input_values_by_feature_position = {}"<<"\n";
    strOut<<"    output_field_name_tm = str_date"<<"\n";
    strOut<<"    output_field_name_tm = output_field_name_tm + '_' + 'tm'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name_tm = output_field_name_tm + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name_tm = output_field_name_tm + 'p'"<<"\n";
    strOut<<"    output_field_tm_id_index = in_layer_definition.GetFieldIndex(output_field_name_tm)"<<"\n";
    strOut<<"    if output_field_tm_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name_tm, ogr.OFTReal))#ogr.OFTReal))"<<"\n";
    strOut<<"    output_field_name_ts = str_date"<<"\n";
    strOut<<"    output_field_name_ts = output_field_name_ts + '_' + 'ts'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name_ts = output_field_name_ts + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name_ts = output_field_name_ts + 'p'"<<"\n";
    strOut<<"    output_field_ts_id_index = in_layer_definition.GetFieldIndex(output_field_name_ts)"<<"\n";
    strOut<<"    if output_field_ts_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name_ts, ogr.OFTReal))#ogr.OFTReal))"<<"\n";
    strOut<<"    output_field_name_nvs = str_date"<<"\n";
    strOut<<"    output_field_name_nvs = output_field_name_nvs + '_' + 'nvs'"<<"\n";
    strOut<<"    output_field_nvs_id_index = in_layer_definition.GetFieldIndex(output_field_name_nvs)"<<"\n";
    strOut<<"    if output_field_nvs_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name_nvs, ogr.OFTInteger))#ogr.OFTReal))"<<"\n";
    strOut<<"    output_field_name_c = str_date"<<"\n";
    strOut<<"    output_field_name_c = output_field_name_c + '_' + 'c'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name_c = output_field_name_c + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name_c = output_field_name_c + 'p'"<<"\n";
    strOut<<"    output_field_c_id_index = in_layer_definition.GetFieldIndex(output_field_name_c)"<<"\n";
    strOut<<"    if output_field_c_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name_c, ogr.OFTReal))#ogr.OFTReal))"<<"\n";
    strOut<<"    output_field_name_cx = str_date"<<"\n";
    strOut<<"    output_field_name_cx = output_field_name_cx + '_' + 'cx'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name_cx = output_field_name_cx + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name_cx = output_field_name_cx + 'p'"<<"\n";
    strOut<<"    output_field_cx_id_index = in_layer_definition.GetFieldIndex(output_field_name_cx)"<<"\n";
    strOut<<"    if output_field_cx_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name_cx, ogr.OFTReal))#ogr.OFTReal))"<<"\n";
    strOut<<"    output_field_name_cn = str_date"<<"\n";
    strOut<<"    output_field_name_cn = output_field_name_cn + '_' + 'cn'"<<"\n";
    strOut<<"    if kmeans_clusters > -1:"<<"\n";
    strOut<<"        output_field_name_cn = output_field_name_cn + 'k'"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        output_field_name_cn = output_field_name_cn + 'p'"<<"\n";
    strOut<<"    output_field_cn_id_index = in_layer_definition.GetFieldIndex(output_field_name_cn)"<<"\n";
    strOut<<"    if output_field_cn_id_index == -1:"<<"\n";
    strOut<<"        in_layer.CreateField(ogr.FieldDefn(output_field_name_cn, ogr.OFTReal))#ogr.OFTReal))"<<"\n";
    strOut<<"    es = (0.611 * math.exp((17.27 * temperature_air) / (237.3 + temperature_air)))"<<"\n";
    strOut<<"    ea = es * relative_humidity / 100."<<"\n";
    strOut<<"    VPD = es - ea"<<"\n";
    strOut<<"    dTul = upper_line_coef_a * VPD + upper_line_coef_b"<<"\n";
    strOut<<"    dTll = lower_line_coef_a * VPD + lower_line_coef_b"<<"\n";
    strOut<<"    for feature in in_layer:"<<"\n";
    strOut<<"        print('Processing plant: {}, of {}'.format(str(cont_feature + 1),"<<"\n";
    strOut<<"                                                   str(number_of_features)))"<<"\n";
    strOut<<"        plot_geometry_full = feature.GetGeometryRef().Clone()"<<"\n";
    strOut<<"        crs_transform = None"<<"\n";
    strOut<<"        if in_crs_wkt != orthomosaic_crs_wkt:"<<"\n";
    strOut<<"            crs_transform = osr.CoordinateTransformation(in_crs, orthomosaic_crs)"<<"\n";
    strOut<<"        if crs_transform:"<<"\n";
    strOut<<"            plot_geometry_full.Transform(crs_transform)"<<"\n";
    strOut<<"        plot_geometry = None"<<"\n";
    strOut<<"        if orthomosaic_poly.Overlaps(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = plot_geometry_full.Intersection(orthomosaic_poly)"<<"\n";
    strOut<<"        if orthomosaic_poly.Contains(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = plot_geometry_full"<<"\n";
    strOut<<"        if orthomosaic_poly.Within(plot_geometry_full):"<<"\n";
    strOut<<"            plot_geometry = orthomosaic_poly"<<"\n";
    strOut<<"        if not plot_geometry:"<<"\n";
    strOut<<"            continue"<<"\n";
    strOut<<"        plot_geometry = plot_geometry_full.Intersection(orthomosaic_poly)"<<"\n";
    strOut<<"        plot_geometry_area = plot_geometry.GetArea()"<<"\n";
    strOut<<"        if plot_geometry_area < (3 * orthomosaic_pixel_area):"<<"\n";
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
    strOut<<"        rs_x_off = int((plot_geom_x_min - orthomosaic_x_origin) / rs_pixel_width)"<<"\n";
    strOut<<"        rs_y_off = int((orthomosaic_y_origin - plot_geom_y_max) / rs_pixel_width)"<<"\n";
    strOut<<"        x_ul = orthomosaic_x_origin + rs_x_off * rs_pixel_width"<<"\n";
    strOut<<"        y_ul = orthomosaic_y_origin - rs_y_off * rs_pixel_width"<<"\n";
    strOut<<"        rs_x_count = int((plot_geom_x_max - plot_geom_x_min) / rs_pixel_width) + 1"<<"\n";
    strOut<<"        rs_y_count = int((plot_geom_y_max - plot_geom_y_min) / rs_pixel_width) + 1"<<"\n";
    strOut<<"        # Create memory target raster"<<"\n";
    strOut<<"        target_orthomosaic = gdal.GetDriverByName('MEM').Create('', rs_x_count, rs_y_count, 1, gdal.GDT_Byte)"<<"\n";
    strOut<<"        target_orthomosaic.SetGeoTransform(("<<"\n";
    strOut<<"            plot_geom_x_min, rs_pixel_width, 0,"<<"\n";
    strOut<<"            plot_geom_y_max, 0, rs_pixel_height,"<<"\n";
    strOut<<"        ))"<<"\n";
    strOut<<"        # Create for target raster the same projection as for the value raster"<<"\n";
    strOut<<"        raster_srs = osr.SpatialReference()"<<"\n";
    strOut<<"        raster_srs.ImportFromWkt(orthomosaic_ds.GetProjectionRef())"<<"\n";
    strOut<<"        target_orthomosaic.SetProjection(raster_srs.ExportToWkt())"<<"\n";
    strOut<<"        target_orthomosaic.SetProjection(raster_srs.ExportToWkt())"<<"\n";
    strOut<<"        feature_drv = ogr.GetDriverByName('ESRI Shapefile')"<<"\n";
    strOut<<"        feature_ds= feature_drv.CreateDataSource(\"/vsimem/memory_name.shp\")"<<"\n";
    strOut<<"        # geometryType = plot_geometry.getGeometryType()"<<"\n";
    strOut<<"        feature_layer = feature_ds.CreateLayer(\"layer\", orthomosaic_crs, geom_type=plot_geometry.GetGeometryType())"<<"\n";
    strOut<<"        featureDefnHeaders = feature_layer.GetLayerDefn()"<<"\n";
    strOut<<"        out_feature = ogr.Feature(featureDefnHeaders)"<<"\n";
    strOut<<"        out_feature.SetGeometry(plot_geometry)"<<"\n";
    strOut<<"        feature_layer.CreateFeature(out_feature)"<<"\n";
    strOut<<"        feature_ds.FlushCache()"<<"\n";
    strOut<<"        # Rasterize zone polygon to raster blue"<<"\n";
    strOut<<"        gdal.RasterizeLayer(target_orthomosaic, [1], feature_layer, burn_values=[1])"<<"\n";
    strOut<<"        feature_orthomosaic_band_mask = target_orthomosaic.GetRasterBand(1)"<<"\n";
    strOut<<"        feature_orthomosaic_data_mask = (feature_orthomosaic_band_mask.ReadAsArray(0, 0,"<<"\n";
    strOut<<"                                                                                  rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                         .astype(float))"<<"\n";
    strOut<<"        # Mask zone of raster blue"<<"\n";
    strOut<<"        feature_orthomosaic_data = (orthomosaic_ds_rb.ReadAsArray(rs_x_off, rs_y_off, rs_x_count, rs_y_count)"<<"\n";
    strOut<<"                                    .astype(float))"<<"\n";
    strOut<<"        feature_raster_array = numpy.ma.masked_array(feature_orthomosaic_data,"<<"\n";
    strOut<<"                                                         numpy.logical_not(feature_orthomosaic_data_mask))"<<"\n";
    strOut<<"        orthomosaic_first_indexes, orthomosaic_second_indexes = feature_raster_array.nonzero()"<<"\n";
    strOut<<"        temperature_values = []"<<"\n";
    strOut<<"        crop_temperature_values = []"<<"\n";
    strOut<<"        for i in range(len(orthomosaic_first_indexes)):"<<"\n";
    strOut<<"            fi = orthomosaic_first_indexes[i]"<<"\n";
    strOut<<"            si = orthomosaic_second_indexes[i]"<<"\n";
    strOut<<"            temperature = feature_raster_array[fi][si] * factor_to_temperature"<<"\n";
    strOut<<"            temperature_values.append(temperature)"<<"\n";
    strOut<<"        if kmeans_clusters > -1:"<<"\n";
    strOut<<"            input_values_cv = numpy.zeros([len(temperature_values), 1], dtype=numpy.float32)"<<"\n";
    strOut<<"            cont_value = 0"<<"\n";
    strOut<<"            for temperature_value in temperature_values:"<<"\n";
    strOut<<"                input_values_cv[cont_value][0] = temperature_values[cont_value]"<<"\n";
    strOut<<"                cont_value = cont_value + 1"<<"\n";
    strOut<<"            # Define criteria = ( type, max_iter = 10 , epsilon = 1.0 )"<<"\n";
    strOut<<"            # criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 10, 1.0)"<<"\n";
    strOut<<"            criteria = (cv.TERM_CRITERIA_MAX_ITER, 100, 1.0)"<<"\n";
    strOut<<"            flags = cv.KMEANS_RANDOM_CENTERS"<<"\n";
    strOut<<"            compactness, labels, centers = cv.kmeans(input_values_cv, kmeans_clusters,"<<"\n";
    strOut<<"                                                     None, criteria, 10, flags)"<<"\n";
    strOut<<"            pos_center_min_value = -1"<<"\n";
    strOut<<"            center_min_value = 100000000."<<"\n";
    strOut<<"            for i in range(kmeans_clusters):"<<"\n";
    strOut<<"                if centers[i] < center_min_value:"<<"\n";
    strOut<<"                    center_min_value = centers[i]"<<"\n";
    strOut<<"                    pos_center_min_value = i"<<"\n";
    strOut<<"            cont_value = 0"<<"\n";
    strOut<<"            for temperature_value in temperature_values:"<<"\n";
    strOut<<"                if labels[cont_value] == pos_center_min_value:"<<"\n";
    strOut<<"                    crop_temperature_values.append(temperature_value)"<<"\n";
    strOut<<"                cont_value = cont_value + 1"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            temperature_values.sort(key=sortFunction)"<<"\n";
    strOut<<"            threshold_value = -1"<<"\n";
    strOut<<"            cont_value = 0"<<"\n";
    strOut<<"            for i in range(0, len(temperature_values)):"<<"\n";
    strOut<<"                crop_temperature_values.append(temperature_values[cont_value])"<<"\n";
    strOut<<"                cont_value = cont_value + 1"<<"\n";
    strOut<<"                if cont_value / len(temperature_values) > percentile_maximum_threshold:"<<"\n";
    strOut<<"                    threshold_value = temperature_values[cont_value-1]"<<"\n";
    strOut<<"                    break"<<"\n";
    strOut<<"        crop_temperature_mean = 0"<<"\n";
    strOut<<"        for crop_temperature_value in crop_temperature_values:"<<"\n";
    strOut<<"            crop_temperature_mean = crop_temperature_mean + crop_temperature_value"<<"\n";
    strOut<<"        crop_temperature_mean = crop_temperature_mean / len(crop_temperature_values)"<<"\n";
    strOut<<"        crop_temperature_std = 0"<<"\n";
    strOut<<"        if len(crop_temperature_values) > 1:"<<"\n";
    strOut<<"            for crop_temperature_value in crop_temperature_values:"<<"\n";
    strOut<<"                crop_temperature_std = crop_temperature_std + pow(crop_temperature_value - crop_temperature_mean, 2.)"<<"\n";
    strOut<<"            crop_temperature_std = sqrt(crop_temperature_std / (len(crop_temperature_values) - 1))"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            crop_temperature_std = -1."<<"\n";
    strOut<<"        dTx = crop_temperature_mean - temperature_air"<<"\n";
    strOut<<"        cwsi = (dTx - dTll) / (dTul - dTll)"<<"\n";
    strOut<<"        dTx_min = crop_temperature_mean - crop_temperature_std - temperature_air"<<"\n";
    strOut<<"        cwsi_min = (dTx_min - dTll) / (dTul - dTll)"<<"\n";
    strOut<<"        dTx_max = crop_temperature_mean + crop_temperature_std - temperature_air"<<"\n";
    strOut<<"        cwsi_max = (dTx_max - dTll) / (dTul - dTll)"<<"\n";
    strOut<<"        cont_feature = cont_feature + 1"<<"\n";
    strOut<<"        feature.SetField(output_field_name_tm, crop_temperature_mean)"<<"\n";
    strOut<<"        feature.SetField(output_field_name_ts, crop_temperature_std)"<<"\n";
    strOut<<"        feature.SetField(output_field_name_nvs, len(crop_temperature_values))"<<"\n";
    strOut<<"        feature.SetField(output_field_name_c, cwsi)"<<"\n";
    strOut<<"        feature.SetField(output_field_name_cx, cwsi_max)"<<"\n";
    strOut<<"        feature.SetField(output_field_name_cn, cwsi_min)"<<"\n";
    strOut<<"        in_layer.SetFeature(feature)"<<"\n";
    strOut<<"    in_vec_ds = None"<<"\n";
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
    strOut<<"    parser.add_option(\"--method_segmentation\", dest=\"method_segmentation\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Method segmentation: kmeans or percentile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--kmeans_clusters\", dest=\"kmeans_clusters\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Number of cluster for kmeans segmentation\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--percentile_maximum_threshold\", dest=\"percentile_maximum_threshold\", action=\"store\","<<"\n";
    strOut<<"                      type=\"string\", help=\"Maximum value (per unit) for percentile segmentation\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--input_orthomosaic\", dest=\"input_orthomosaic\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Input thermal orthomosaic\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--temperature\", dest=\"temperature\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Air temperature (celsius degrees)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--relative_humidity\", dest=\"relative_humidity\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Relative humidity (percentage)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--upper_line_coef_a\", dest=\"upper_line_coef_a\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Upper line coefficient A (slope)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--upper_line_coef_b\", dest=\"upper_line_coef_b\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Upper line coefficient B (y value for x equal to 0)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--lower_line_coef_a\", dest=\"lower_line_coef_a\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Lower line coefficient A (slope)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--lower_line_coef_b\", dest=\"lower_line_coef_b\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Lower line coefficient B (y value for x equal to 0)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--factor_to_temperature\", dest=\"factor_to_temperature\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Factor for convert band values in thermal raster to temperature (celsius degrees)\")"<<"\n";
    strOut<<"    parser.add_option(\"--output_shp\", dest=\"output_shp\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Output shapefile or none for use input shapefile\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date_from_orthomosaic_file\", dest=\"date_from_orthomosaic_file\", action=\"store\","<<"\n";
    strOut<<"                      type=\"int\", help=\"Read date from orthomosaic file name: 1-yes, 0-No, for ndvi method\","<<"\n";
    strOut<<"                      default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--orthomosaic_file_string_separator\", dest=\"orthomosaic_file_string_separator\","<<"\n";
    strOut<<"                      action=\"store\", type=\"string\", help=\"Orthomosaic file string separator, for ndvi method\","<<"\n";
    strOut<<"                      default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--orthomosaic_file_date_string_position\", dest=\"orthomosaic_file_date_string_position\","<<"\n";
    strOut<<"                      action=\"store\", type=\"int\","<<"\n";
    strOut<<"                      help=\"Orthomosaic file date string position, for ndvi method\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date_format\", dest=\"date_format\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"Date format (%Y%m%d, ...)\", default=None)"<<"\n";
    strOut<<"    parser.add_option(\"--date\", dest=\"date\", action=\"store\", type=\"string\","<<"\n";
    strOut<<"                      help=\"None or date value no from orthomosaic files, for ndvi method\", default=None)"<<"\n";
    strOut<<"    (options, args) = parser.parse_args()"<<"\n";
    strOut<<"    if not options.input_crops_frames_shp:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_shp = options.input_crops_frames_shp"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        print(\"Error:\\nInput crops shapefile does not exists:\\n{}\".format(input_shp))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.output_shp:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    use_input_shp = True"<<"\n";
    strOut<<"    output_shp = options.output_shp"<<"\n";
    strOut<<"    if output_shp != 'none':"<<"\n";
    strOut<<"        use_input_shp = False"<<"\n";
    strOut<<"    if not options.method_segmentation:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    method_segmentation = options.method_segmentation"<<"\n";
    strOut<<"    kmeans_clusters = -1"<<"\n";
    strOut<<"    percentile_maximum_threshold = -1."<<"\n";
    strOut<<"    if method_segmentation == 'kmeans':"<<"\n";
    strOut<<"        if not options.kmeans_clusters:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_kmeans_clusters = options.kmeans_clusters"<<"\n";
    strOut<<"        if not is_number(str_kmeans_clusters):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for kmeans clusters: {}\"."<<"\n";
    strOut<<"                  format(str_kmeans_clusters))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        kmeans_clusters = int(str_kmeans_clusters)"<<"\n";
    strOut<<"        if kmeans_clusters < 2 or kmeans_clusters > 20:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for kmeans clusters: {}\"."<<"\n";
    strOut<<"                  format(str_kmeans_clusters))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    elif method_segmentation == 'percentile':"<<"\n";
    strOut<<"        if not options.percentile_maximum_threshold:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_percentile_maximum_threshold = options.percentile_maximum_threshold"<<"\n";
    strOut<<"        if not is_number(str_percentile_maximum_threshold):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for percentile maximum threshold: {}\"."<<"\n";
    strOut<<"                  format(str_percentile_maximum_threshold))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        percentile_maximum_threshold = float(str_percentile_maximum_threshold)"<<"\n";
    strOut<<"        if percentile_maximum_threshold < 0 or percentile_maximum_threshold > 1:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for percentile maximum threshold: {}\"."<<"\n";
    strOut<<"                  format(str_percentile_maximum_threshold))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    if kmeans_clusters < 0 and percentile_maximum_threshold < 0:"<<"\n";
    strOut<<"        print(\"Error:\\nMethod segmentation must be: kmeans or percentile\")"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_orthomosaic = options.input_orthomosaic"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        print(\"Error:\\nInput orthomosaic does not exists:\\n{}\".format(input_orthomosaic))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not options.temperature:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_temperature = options.temperature"<<"\n";
    strOut<<"    if not is_number(str_temperature):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for temperature value: {}\"."<<"\n";
    strOut<<"              format(str_temperature))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    temperature_air = float(str_temperature)"<<"\n";
    strOut<<"    if not options.relative_humidity:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_relative_humidity = options.relative_humidity"<<"\n";
    strOut<<"    if not is_number(str_relative_humidity):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for relative humidity value: {}\"."<<"\n";
    strOut<<"              format(str_relative_humidity))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    relative_humidity = float(str_relative_humidity)"<<"\n";
    strOut<<"    if not options.upper_line_coef_a:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_upper_line_coef_a = options.upper_line_coef_a"<<"\n";
    strOut<<"    if not is_number(str_upper_line_coef_a):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for upper line coefficient A value: {}\"."<<"\n";
    strOut<<"              format(str_upper_line_coef_a))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    upper_line_coef_a = float(str_upper_line_coef_a)"<<"\n";
    strOut<<"    if not options.upper_line_coef_b:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_upper_line_coef_b = options.upper_line_coef_b"<<"\n";
    strOut<<"    if not is_number(str_upper_line_coef_b):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for upper line coefficient B value: {}\"."<<"\n";
    strOut<<"              format(str_upper_line_coef_b))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    upper_line_coef_b = float(str_upper_line_coef_b)"<<"\n";
    strOut<<"    if not options.lower_line_coef_a:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_lower_line_coef_a = options.lower_line_coef_a"<<"\n";
    strOut<<"    if not is_number(str_lower_line_coef_a):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for lower line coefficient A value: {}\"."<<"\n";
    strOut<<"              format(str_lower_line_coef_a))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    lower_line_coef_a = float(str_lower_line_coef_a)"<<"\n";
    strOut<<"    if not options.lower_line_coef_b:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_lower_line_coef_b = options.lower_line_coef_b"<<"\n";
    strOut<<"    if not is_number(str_lower_line_coef_b):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for lower line coefficient B value: {}\"."<<"\n";
    strOut<<"              format(str_lower_line_coef_b))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    lower_line_coef_b = float(str_lower_line_coef_b)"<<"\n";
    strOut<<"    if not options.factor_to_temperature:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    str_factor_to_temperature = options.factor_to_temperature"<<"\n";
    strOut<<"    if not is_number(str_factor_to_temperature):"<<"\n";
    strOut<<"        print(\"Error:\\nInvalid value for factor to temperature: {}\"."<<"\n";
    strOut<<"              format(str_factor_to_temperature))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    factor_to_temperature = float(str_factor_to_temperature)"<<"\n";
    strOut<<"    if options.date_from_orthomosaic_file == None:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    date_from_orthomosaic = False"<<"\n";
    strOut<<"    if options.date_from_orthomosaic_file == 1:"<<"\n";
    strOut<<"        date_from_orthomosaic = True"<<"\n";
    strOut<<"    date = None"<<"\n";
    strOut<<"    if not options.date_format:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    date_format = options.date_format.strip()"<<"\n";
    strOut<<"    if not date_from_orthomosaic:"<<"\n";
    strOut<<"        if options.date == 'none':"<<"\n";
    strOut<<"            print(\"Error:\\nDate must be a value if not read from orthomosaic file name\")"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_date = options.date"<<"\n";
    strOut<<"        is_date = True"<<"\n";
    strOut<<"        if len(options.date) == 6:"<<"\n";
    strOut<<"            str_date = '20' + str_date"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            date = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"        except ValueError as error:"<<"\n";
    strOut<<"            is_date = False"<<"\n";
    strOut<<"        if not is_date:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid string date from orthomosaic name: {} and format: {}\"."<<"\n";
    strOut<<"                  format(options.date, date_format))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        if not options.orthomosaic_file_string_separator:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        orthomosaic_file_string_separator = options.orthomosaic_file_string_separator"<<"\n";
    strOut<<"        if not options.orthomosaic_file_date_string_position:"<<"\n";
    strOut<<"            parser.print_help()"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        orthomosaic_file_date_string_position = options.orthomosaic_file_date_string_position"<<"\n";
    strOut<<"        orthomosaic_file_name_without_path = os.path.splitext(os.path.basename(input_orthomosaic))[0]"<<"\n";
    strOut<<"        orthomosaic_file_name_values = orthomosaic_file_name_without_path.split(orthomosaic_file_string_separator)"<<"\n";
    strOut<<"        if (orthomosaic_file_date_string_position < 0"<<"\n";
    strOut<<"                or orthomosaic_file_date_string_position > len(orthomosaic_file_name_values)):"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid value for orthomosaic files date string position: {}\"."<<"\n";
    strOut<<"                  format(str(orthomosaic_file_date_string_position)))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        str_date = orthomosaic_file_name_values[orthomosaic_file_date_string_position - 1]"<<"\n";
    strOut<<"        is_date = True"<<"\n";
    strOut<<"        if len(str_date) == 6:"<<"\n";
    strOut<<"            str_date = '20' + str_date"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            date = datetime.datetime.strptime(str_date, date_format)"<<"\n";
    strOut<<"        except ValueError as error:"<<"\n";
    strOut<<"            is_date = False"<<"\n";
    strOut<<"        if not is_date:"<<"\n";
    strOut<<"            print(\"Error:\\nInvalid string date from orthomosaic name: {} and format: {}\"."<<"\n";
    strOut<<"                  format(orthomosaic_file_name_values[orthomosaic_file_date_string_position - 1],"<<"\n";
    strOut<<"                         date_format))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    str_date = str(date.strftime('%Y')[2:4]) + str(date.strftime('%m')) + str(date.strftime('%d'))"<<"\n";
    strOut<<"    input_field = None"<<"\n";
    strOut<<"    if not use_input_shp:"<<"\n";
    strOut<<"        str_error = copy_shapefile(input_shp, output_shp)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_shp = output_shp"<<"\n";
    strOut<<"    process_cwsith(input_shp,"<<"\n";
    strOut<<"                   kmeans_clusters,"<<"\n";
    strOut<<"                   percentile_maximum_threshold,"<<"\n";
    strOut<<"                   input_orthomosaic,"<<"\n";
    strOut<<"                   temperature_air,"<<"\n";
    strOut<<"                   relative_humidity,"<<"\n";
    strOut<<"                   upper_line_coef_a,"<<"\n";
    strOut<<"                   upper_line_coef_b,"<<"\n";
    strOut<<"                   lower_line_coef_a,"<<"\n";
    strOut<<"                   lower_line_coef_b,"<<"\n";
    strOut<<"                   factor_to_temperature,"<<"\n";
    strOut<<"                   str_date)"<<"\n";
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

bool PAFyCToolsDialog::writePythonProgramSoilZoningBasedInReflectivity(QString pythonFileName,
                                                                       QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_SZBR;
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
    strOut<<""<<"\n";
    strOut<<"# import optparse"<<"\n";
    strOut<<"import argparse"<<"\n";
    strOut<<"import numpy as np"<<"\n";
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
    strOut<<"import cv2 as cv"<<"\n";
    strOut<<"import math"<<"\n";
    strOut<<"from pathlib import Path"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"class GdalErrorHandler(object):"<<"\n";
    strOut<<"    def __init__(self):"<<"\n";
    strOut<<"        self.err_level = gdal.CE_None"<<"\n";
    strOut<<"        self.err_no = 0"<<"\n";
    strOut<<"        self.err_msg = ''"<<"\n";
    strOut<<""<<"\n";
    strOut<<"    def handler(self, err_level, err_no, err_msg):"<<"\n";
    strOut<<"        self.err_level = err_level"<<"\n";
    strOut<<"        self.err_no = err_no"<<"\n";
    strOut<<"        self.err_msg = err_msg"<<"\n";
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
    strOut<<"def process(input_orthomosaic,"<<"\n";
    strOut<<"            input_shp,"<<"\n";
    strOut<<"            factor_to_reflectance,"<<"\n";
    strOut<<"            bands_to_use,"<<"\n";
    strOut<<"            red_band_number,"<<"\n";
    strOut<<"            nir_band_number,"<<"\n";
    strOut<<"            minimum_ndvi,"<<"\n";
    strOut<<"            maximum_ndvi,"<<"\n";
    strOut<<"            minimum_explained_variance,"<<"\n";
    strOut<<"            only_one_principal_component,"<<"\n";
    strOut<<"            max_number_of_kmeans_clusters,"<<"\n";
    strOut<<"            minimum_classification_area,"<<"\n";
    strOut<<"            output_path):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    if input_shp:"<<"\n";
    strOut<<"        if not exists(input_shp):"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds = gdal.Open(input_orthomosaic)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_number_of_bands = orthomosaic_ds.RasterCount"<<"\n";
    strOut<<"    if red_band_number > orthomosaic_number_of_bands:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nRed band number is greather than orthomosaic number of bands\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if nir_band_number > orthomosaic_number_of_bands:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNir band number is greather than orthomosaic number of bands\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    for band_number in bands_to_use:"<<"\n";
    strOut<<"        if band_number > orthomosaic_number_of_bands or band_number < 1:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += (\"\\nBand to use number: {} is out of valid number for orthomosaic number of bands\""<<"\n";
    strOut<<"                          .format(str(band_number)))"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"    xSize = orthomosaic_ds.RasterXSize"<<"\n";
    strOut<<"    ySize = orthomosaic_ds.RasterYSize"<<"\n";
    strOut<<"    orthomosaic_geotransform = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    gsd_x = abs(orthomosaic_geotransform[1])"<<"\n";
    strOut<<"    gsd_y = abs(orthomosaic_geotransform[5])"<<"\n";
    strOut<<"    gsd = gsd_x"<<"\n";
    strOut<<"    median_filter_number_of_pixels = ceil(minimum_classification_area / gsd)"<<"\n";
    strOut<<"    if median_filter_number_of_pixels % 2 == 0:"<<"\n";
    strOut<<"        median_filter_number_of_pixels = median_filter_number_of_pixels + 1"<<"\n";
    strOut<<"    median_filter_position = round((median_filter_number_of_pixels ** 2 + 1) / 2)"<<"\n";
    strOut<<"    projection = orthomosaic_ds.GetProjection()"<<"\n";
    strOut<<"    orthomosaic_crs = osr.SpatialReference()"<<"\n";
    strOut<<"    orthomosaic_crs.ImportFromWkt(orthomosaic_ds.GetProjectionRef())"<<"\n";
    strOut<<"    orthomosaic_crs_wkt = orthomosaic_crs.ExportToWkt()"<<"\n";
    strOut<<"    ulx, xres, xskew, uly, yskew, yres = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    lrx = ulx + (orthomosaic_ds.RasterXSize * xres)"<<"\n";
    strOut<<"    lry = uly + (orthomosaic_ds.RasterYSize * yres)"<<"\n";
    strOut<<"    out_ring = ogr.Geometry(ogr.wkbLinearRing)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    orthomosaic_poly = ogr.Geometry(ogr.wkbPolygon)"<<"\n";
    strOut<<"    orthomosaic_poly.AddGeometry(out_ring)"<<"\n";
    strOut<<"    rs_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    rs_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    orthomosaic_pixel_area = abs(rs_pixel_width) * abs(rs_pixel_height)"<<"\n";
    strOut<<"    orthomosaic_x_origin = orthomosaic_geotransform[0]"<<"\n";
    strOut<<"    orthomosaic_y_origin = orthomosaic_geotransform[3]"<<"\n";
    strOut<<"    orthomosaic_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    orthomosaic_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    values_by_band = {}"<<"\n";
    strOut<<"    band_red = orthomosaic_ds.GetRasterBand(red_band_number)"<<"\n";
    strOut<<"    band_red_no_data_value = band_red.GetNoDataValue()"<<"\n";
    strOut<<"    band_red_data = band_red.ReadAsArray()"<<"\n";
    strOut<<"    band_redmasked_data = np.ma.masked_where(band_red_data == band_red_no_data_value, band_red_data)"<<"\n";
    strOut<<"    band_redindexes_without_no_data_value = band_redmasked_data.nonzero()"<<"\n";
    strOut<<"    columns_by_row = {}"<<"\n";
    strOut<<"    for i in range(len(band_redindexes_without_no_data_value[0])):"<<"\n";
    strOut<<"        row = band_redindexes_without_no_data_value[0][i]"<<"\n";
    strOut<<"        column = band_redindexes_without_no_data_value[1][i]"<<"\n";
    strOut<<"        if not row in columns_by_row:"<<"\n";
    strOut<<"            columns_by_row[row] = []"<<"\n";
    strOut<<"        columns_by_row[row].append(column)"<<"\n";
    strOut<<"    band_redmasked_data = None"<<"\n";
    strOut<<"    band_redindexes_without_no_data_value = None"<<"\n";
    strOut<<"    band_nir = orthomosaic_ds.GetRasterBand(nir_band_number)"<<"\n";
    strOut<<"    band_nir_no_data_value = band_nir.GetNoDataValue()"<<"\n";
    strOut<<"    band_nir_data = band_nir.ReadAsArray()"<<"\n";
    strOut<<"    valid_indexes = []"<<"\n";
    strOut<<"    invalid_indexes = []"<<"\n";
    strOut<<"    print('Filtering by NDVI value ...', flush=True)"<<"\n";
    strOut<<"    for row in columns_by_row:"<<"\n";
    strOut<<"        for j in range(len(columns_by_row[row])):"<<"\n";
    strOut<<"            column = columns_by_row[row][j]"<<"\n";
    strOut<<"            red_value = band_red_data[row][column] * factor_to_reflectance"<<"\n";
    strOut<<"            nir_value = band_nir_data[row][column] * factor_to_reflectance"<<"\n";
    strOut<<"            ndvi_value = (nir_value - red_value) / (nir_value + red_value)"<<"\n";
    strOut<<"            index = [row, column]"<<"\n";
    strOut<<"            if ndvi_value >= minimum_ndvi and ndvi_value <= maximum_ndvi:"<<"\n";
    strOut<<"                valid_indexes.append(index)"<<"\n";
    strOut<<"            else:"<<"\n";
    strOut<<"                invalid_indexes.append(index)"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    data = np.zeros((len(valid_indexes), len(bands_to_use)))"<<"\n";
    strOut<<"    invalid_positions_in_valid_indexes = []"<<"\n";
    strOut<<"    for j in range(len(bands_to_use)):"<<"\n";
    strOut<<"        band_number = bands_to_use[j]"<<"\n";
    strOut<<"        print('Getting data for band {} ...'.format(band_number), flush=True)"<<"\n";
    strOut<<"        band_data = None"<<"\n";
    strOut<<"        band_no_data_value = None"<<"\n";
    strOut<<"        if band_number == red_band_number:"<<"\n";
    strOut<<"            band_data = band_red_data"<<"\n";
    strOut<<"            band_no_data_value = band_red_no_data_value"<<"\n";
    strOut<<"        elif band_number == nir_band_number:"<<"\n";
    strOut<<"            band_data = band_nir_data"<<"\n";
    strOut<<"            band_no_data_value = band_nir_no_data_value"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            band = orthomosaic_ds.GetRasterBand(band_number)"<<"\n";
    strOut<<"            band_no_data_value = band.GetNoDataValue()"<<"\n";
    strOut<<"            band_data = band.ReadAsArray()  # rows, columns"<<"\n";
    strOut<<"        for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"            if i in invalid_positions_in_valid_indexes:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            row = valid_indexes[i][0]"<<"\n";
    strOut<<"            column = valid_indexes[i][1]"<<"\n";
    strOut<<"            value = band_data[row][column]"<<"\n";
    strOut<<"            if value == band_no_data_value:"<<"\n";
    strOut<<"                index = [row, column]"<<"\n";
    strOut<<"                invalid_positions_in_valid_indexes.append(i)"<<"\n";
    strOut<<"            data[i][j] = value * factor_to_reflectance"<<"\n";
    strOut<<"        print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    band_red_data = None"<<"\n";
    strOut<<"    band_nir_data = None"<<"\n";
    strOut<<"    if len(invalid_positions_in_valid_indexes) > 0:"<<"\n";
    strOut<<"        print('Removing pixels with no data value in some band ...', flush=True)"<<"\n";
    strOut<<"        valid_indexes_with_out_outliers = []"<<"\n";
    strOut<<"        data_without_outliers = np.zeros((len(valid_indexes) - len(invalid_positions_in_valid_indexes),"<<"\n";
    strOut<<"                                          len(bands_to_use)))"<<"\n";
    strOut<<"        pos = -1"<<"\n";
    strOut<<"        for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"            if i in invalid_positions_in_valid_indexes:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            pos = pos + 1"<<"\n";
    strOut<<"            row = valid_indexes[i][0]"<<"\n";
    strOut<<"            column = valid_indexes[i][1]"<<"\n";
    strOut<<"            for j in range(len(bands_to_use)):"<<"\n";
    strOut<<"                data_without_outliers[pos][j] = data[i][j]"<<"\n";
    strOut<<"            index = [row, column]"<<"\n";
    strOut<<"            valid_indexes_with_out_outliers.append(index)"<<"\n";
    strOut<<"        data = None"<<"\n";
    strOut<<"        valid_indexes = None"<<"\n";
    strOut<<"        data = data_without_outliers"<<"\n";
    strOut<<"        valid_indexes = valid_indexes_with_out_outliers"<<"\n";
    strOut<<"        data_without_outliers = None"<<"\n";
    strOut<<"        valid_indexes_with_out_outliers = None"<<"\n";
    strOut<<"        print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    print('Computing principal components and transforming data values to new base ...', flush=True)"<<"\n";
    strOut<<"    standardized_data = (data - data.mean(axis=0)) / data.std(axis=0)"<<"\n";
    strOut<<"    data = None"<<"\n";
    strOut<<"    covariance_matrix = np.cov(standardized_data, ddof=1, rowvar=False)"<<"\n";
    strOut<<"    eigenvalues, eigenvectors = np.linalg.eig(covariance_matrix)"<<"\n";
    strOut<<"    # np.argsort can only provide lowest to highest; use [::-1] to reverse the list"<<"\n";
    strOut<<"    order_of_importance = np.argsort(eigenvalues)[::-1]"<<"\n";
    strOut<<"    # utilize the sort order to sort eigenvalues and eigenvectors"<<"\n";
    strOut<<"    sorted_eigenvalues = eigenvalues[order_of_importance]"<<"\n";
    strOut<<"    sorted_eigenvectors = eigenvectors[:, order_of_importance]  # sort the columns"<<"\n";
    strOut<<"    # use sorted_eigenvalues to ensure the explained variances correspond to the eigenvectors"<<"\n";
    strOut<<"    explained_variance = sorted_eigenvalues / np.sum(sorted_eigenvalues)"<<"\n";
    strOut<<"    explained_variance_by_selected_components = 0"<<"\n";
    strOut<<"    number_of_pca_components = 0"<<"\n";
    strOut<<"    while explained_variance_by_selected_components < minimum_explained_variance:"<<"\n";
    strOut<<"        explained_variance_by_selected_components = (explained_variance_by_selected_components"<<"\n";
    strOut<<"                                                     + explained_variance[number_of_pca_components])"<<"\n";
    strOut<<"        number_of_pca_components = number_of_pca_components + 1"<<"\n";
    strOut<<"    if number_of_pca_components > 1 and only_one_principal_component:"<<"\n";
    strOut<<"        number_of_pca_components = 1"<<"\n";
    strOut<<"    reduced_data = np.matmul(standardized_data,"<<"\n";
    strOut<<"                             sorted_eigenvectors[:, :number_of_pca_components])  # transform the original data"<<"\n";
    strOut<<"    standardized_data = None"<<"\n";
    strOut<<"    criteria = (cv.TERM_CRITERIA_MAX_ITER, 100, 1.0)"<<"\n";
    strOut<<"    flags = cv.KMEANS_RANDOM_CENTERS"<<"\n";
    strOut<<"    input_values_cv = np.zeros([len(valid_indexes), number_of_pca_components], dtype=np.float32)"<<"\n";
    strOut<<"    for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"        for j in range(number_of_pca_components):"<<"\n";
    strOut<<"            input_values_cv[i][j] = reduced_data[i][j]"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    mse = {}"<<"\n";
    strOut<<"    fileformat = \"GTiff\""<<"\n";
    strOut<<"    driver = gdal.GetDriverByName(fileformat)"<<"\n";
    strOut<<"    if not output_path:"<<"\n";
    strOut<<"        output_path = os.path.dirname(os.path.abspath(input_orthomosaic))"<<"\n";
    strOut<<"    var_path = Path(input_orthomosaic)"<<"\n";
    strOut<<"    base_name = var_path.stem"<<"\n";
    strOut<<"    file_ext = '.tif'"<<"\n";
    strOut<<"    for kmeans_clusters in range(1, max_number_of_kmeans_clusters + 1):"<<"\n";
    strOut<<"        print('Computing results for {} clusters ...'.format(str(kmeans_clusters)), flush=True)"<<"\n";
    strOut<<"        compactness, labels, centers = cv.kmeans(input_values_cv, kmeans_clusters,"<<"\n";
    strOut<<"                                                 None, criteria, 10, flags)"<<"\n";
    strOut<<"        mse[kmeans_clusters] = compactness / len(valid_indexes)"<<"\n";
    strOut<<"        str_mse = str(round(mse[kmeans_clusters] * 100))"<<"\n";
    strOut<<"        dst_filename = (output_path + '/' + base_name + '_' + 'npc_'"<<"\n";
    strOut<<"                        + str(number_of_pca_components) + '_nckms_' + str(kmeans_clusters)"<<"\n";
    strOut<<"                        + '_mse_' + str_mse + file_ext)"<<"\n";
    strOut<<"        dst_filename = os.path.normpath(dst_filename)"<<"\n";
    strOut<<"        dst_ds = driver.Create(dst_filename, xsize=xSize, ysize=ySize, bands=1, eType=gdal.GDT_Byte)"<<"\n";
    strOut<<"        dst_ds.SetGeoTransform(orthomosaic_geotransform)"<<"\n";
    strOut<<"        dst_ds.SetProjection(projection)"<<"\n";
    strOut<<"        # np_raster = np.zeros((ySize, xSize), dtype=np.uint8)"<<"\n";
    strOut<<"        np_raster = np.full((ySize, xSize), 0, dtype=np.uint8)"<<"\n";
    strOut<<"        for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"            row = valid_indexes[i][0]"<<"\n";
    strOut<<"            column = valid_indexes[i][1]"<<"\n";
    strOut<<"            np_raster[row][column] = labels[i] + 1"<<"\n";
    strOut<<"        np_raster = cv.medianBlur(np_raster, median_filter_number_of_pixels)"<<"\n";
    strOut<<"        for i in range(len(invalid_indexes)):"<<"\n";
    strOut<<"            row = invalid_indexes[i][0]"<<"\n";
    strOut<<"            column = invalid_indexes[i][1]"<<"\n";
    strOut<<"            np_raster[row][column] = 0"<<"\n";
    strOut<<"        dst_ds.GetRasterBand(1).SetNoDataValue(0)"<<"\n";
    strOut<<"        dst_ds.GetRasterBand(1).WriteArray(np_raster)"<<"\n";
    strOut<<"        dst_ds = None"<<"\n";
    strOut<<"        print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    return str_error"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def clip_raster(input_raster,"<<"\n";
    strOut<<"                input_shp,"<<"\n";
    strOut<<"                no_data_value,"<<"\n";
    strOut<<"                output_path):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    output_raster_suffix = ''"<<"\n";
    strOut<<"    if not output_path:"<<"\n";
    strOut<<"        output_path = os.path.dirname(os.path.abspath(input_raster))"<<"\n";
    strOut<<"    raster_base_name = os.path.basename(input_raster).split('.')[0]"<<"\n";
    strOut<<"    output_raster = output_path + '\\\\' + raster_base_name"<<"\n";
    strOut<<"    #output_raster = os.path.splitext(input_raster)[0]"<<"\n";
    strOut<<"    output_raster = output_raster + \"_rois\""<<"\n";
    strOut<<"    output_raster = output_raster + os.path.splitext(input_raster)[1]"<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    if not exists(input_raster):"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_raster)"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    # return str_error, output_raster"<<"\n";
    strOut<<"    if exists(output_raster):"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            os.remove(output_raster)"<<"\n";
    strOut<<"        except FileNotFoundError as e:"<<"\n";
    strOut<<"            str_error = (\"Removing output raster:\\n{}\".format(output_raster))"<<"\n";
    strOut<<"            str_error = str_error + (\"\\nError\t\" + e.strerror)"<<"\n";
    strOut<<"            return str_error, output_raster"<<"\n";
    strOut<<"        except OSError as e:"<<"\n";
    strOut<<"            str_error = (\"Removing output raster:\\n{}\".format(output_raster))"<<"\n";
    strOut<<"            str_error = str_error + (\"\\nError\t\" + e.strerror)"<<"\n";
    strOut<<"            return str_error, output_raster"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        input_raster_ds = gdal.Open(input_raster)"<<"\n";
    strOut<<"    except Exception as e:"<<"\n";
    strOut<<"        assert err.err_level == gdal.CE_Failure, ("<<"\n";
    strOut<<"                'The handler error level should now be at failure')"<<"\n";
    strOut<<"        assert err.err_msg == e.args[0], 'raised exception should contain the message'"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error = ('Handled warning: level={}, no={}, msg={}'.format("<<"\n";
    strOut<<"                err.err_level, err.err_no, err.err_msg))"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    raster_no_data_value = input_raster_ds.GetRasterBand(1).GetNoDataValue()"<<"\n";
    strOut<<"    if not raster_no_data_value:"<<"\n";
    strOut<<"        datatype = gdal.GetDataTypeName(input_raster_ds.GetRasterBand(1).DataType)"<<"\n";
    strOut<<"        raster_no_data_value = no_data_value"<<"\n";
    strOut<<"    orthomosaic_geotransform = input_raster_ds.GetGeoTransform()"<<"\n";
    strOut<<"    gsd_x = abs(orthomosaic_geotransform[1])"<<"\n";
    strOut<<"    gsd_y = abs(orthomosaic_geotransform[5])"<<"\n";
    strOut<<"    gdalwarp_str_options = \" -cutline \" + input_shp"<<"\n";
    strOut<<"    gdalwarp_str_options += \" -crop_to_cutline -dstnodata \" + str(no_data_value)"<<"\n";
    strOut<<"    gdalwarp_str_options += \" -tr \""<<"\n";
    strOut<<"    gdalwarp_str_options += \"{:.12f}\".format(gsd_x)"<<"\n";
    strOut<<"    gdalwarp_str_options += \" \""<<"\n";
    strOut<<"    gdalwarp_str_options += \"{:.12f}\".format(gsd_y)"<<"\n";
    strOut<<"    gdalwarp_str_options += \" -co COMPRESS=LZW\""<<"\n";
    strOut<<"    print('Clipping raster ...', flush=True)"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        output_raster_ds = gdal.Warp(output_raster, input_raster_ds, options = gdalwarp_str_options)"<<"\n";
    strOut<<"    except Exception as e:"<<"\n";
    strOut<<"        assert err.err_level == gdal.CE_Failure, ("<<"\n";
    strOut<<"                'The handler error level should now be at failure')"<<"\n";
    strOut<<"        assert err.err_msg == e.args[0], 'raised exception should contain the message'"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error = ('Handled warning: level={}, no={}, msg={}'.format("<<"\n";
    strOut<<"                err.err_level, err.err_no, err.err_msg))"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    return str_error, output_raster"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def main():"<<"\n";
    strOut<<"    parser = argparse.ArgumentParser()"<<"\n";
    strOut<<"    parser.add_argument(\"--input_orthomosaic\", help=\"Input orthomosaic\", type=str)"<<"\n";
    strOut<<"    parser.add_argument(\"--no_data_value\", type=int,"<<"\n";
    strOut<<"                        help=\"Raster no data value, if not defined in file\")"<<"\n";
    strOut<<"    parser.add_argument(\"--input_rois_shp\", help=\"Input input rois shapefile, if exists\", type=str)"<<"\n";
    strOut<<"    parser.add_argument(\"--factor_to_reflectance\", type=float,"<<"\n";
    strOut<<"                        help=\"Multiplicative factor for convert raster values to reflectance\")"<<"\n";
    strOut<<"    parser.add_argument(\"--bands_to_use\", nargs=\"+\", type=int, help=\"Bands to use, starting 1\")"<<"\n";
    strOut<<"    parser.add_argument(\"--red_band_number\", type=int, help=\"Red band number, starting 1\")"<<"\n";
    strOut<<"    parser.add_argument(\"--nir_band_number\", type=int, help=\"Nir band number, starting 1\")"<<"\n";
    strOut<<"    parser.add_argument(\"--minimum_ndvi\", type=float, help=\"Minimmum NDVI, in range [-1,-1]\")"<<"\n";
    strOut<<"    parser.add_argument(\"--maximum_ndvi\", type=float, help=\"Minimmum NDVI, in range [-1,-1]\")"<<"\n";
    strOut<<"    parser.add_argument(\"--minimum_explained_variance\", type=float,"<<"\n";
    strOut<<"                        help=\"Minimmum explained variance by PCA components, in range [0,1]\")"<<"\n";
    strOut<<"    parser.add_argument(\"--only_one_principal_component\", type=int,"<<"\n";
    strOut<<"                        help=\"Use only one principal compponent. Not if compute from explained variance\")"<<"\n";
    strOut<<"    parser.add_argument(\"--max_number_of_kmeans_clusters\", type=int,"<<"\n";
    strOut<<"                        help=\"Maximum number of clusters in Kmeans classification process\")"<<"\n";
    strOut<<"    parser.add_argument(\"--minimum_classification_area\", type=float,"<<"\n";
    strOut<<"                        help=\"Minimum classification area, in meters\")"<<"\n";
    strOut<<"    parser.add_argument(\"--output_path\", type=str,"<<"\n";
    strOut<<"                        help=\"Output path or empty for multispectral orthomosaic path\")"<<"\n";
    strOut<<"    args = parser.parse_args()"<<"\n";
    strOut<<"    if not args.input_orthomosaic:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_orthomosaic = args.input_orthomosaic"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        print(\"Error:\\nInput orthomosaic does not exists:\\n{}\".format(input_orthomosaic))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not args.no_data_value:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"    no_data_value = args.no_data_value"<<"\n";
    strOut<<"    input_rois_shp = None"<<"\n";
    strOut<<"    if args.input_rois_shp:"<<"\n";
    strOut<<"        input_rois_shp = args.input_rois_shp"<<"\n";
    strOut<<"        if input_rois_shp:"<<"\n";
    strOut<<"            if not exists(input_rois_shp):"<<"\n";
    strOut<<"                print(\"Error:\\nInput ROIs shapefile does not exists:\\n{}\".format(input_rois_shp))"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"    if not args.factor_to_reflectance:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"    factor_to_reflectance = args.factor_to_reflectance"<<"\n";
    strOut<<"    if not args.bands_to_use:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    bands_to_use = args.bands_to_use"<<"\n";
    strOut<<"    if not args.red_band_number:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    red_band_number = args.red_band_number"<<"\n";
    strOut<<"    if not args.nir_band_number:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    nir_band_number = args.nir_band_number"<<"\n";
    strOut<<"    if not args.minimum_ndvi:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    minimum_ndvi = args.minimum_ndvi"<<"\n";
    strOut<<"    if not args.maximum_ndvi:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    maximum_ndvi = args.maximum_ndvi"<<"\n";
    strOut<<"    if not args.minimum_explained_variance:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    minimum_explained_variance = args.minimum_explained_variance"<<"\n";
    strOut<<"    if not args.max_number_of_kmeans_clusters:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not args.only_one_principal_component:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    int_only_one_principal_component = args.only_one_principal_component"<<"\n";
    strOut<<"    if int_only_one_principal_component < 0 or args.only_one_principal_component > 1:"<<"\n";
    strOut<<"        print(\"Error:\\nParameter only_one_principal_component must be 0 or 1\")"<<"\n";
    strOut<<"    only_one_principal_component = False"<<"\n";
    strOut<<"    if int_only_one_principal_component == 1:"<<"\n";
    strOut<<"        only_one_principal_component = True"<<"\n";
    strOut<<"    max_number_of_kmeans_clusters = args.max_number_of_kmeans_clusters"<<"\n";
    strOut<<"    if not args.minimum_classification_area:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"    minimum_classification_area = args.minimum_classification_area"<<"\n";
    strOut<<"    output_path = args.output_path"<<"\n";
    strOut<<"    if output_path:"<<"\n";
    strOut<<"        if not exists(output_path):"<<"\n";
    strOut<<"            print(\"Error:\\nOutput path does not exists:\\n{}\".format(output_path))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    if input_rois_shp:"<<"\n";
    strOut<<"        str_error, input_orthomosaic_rois = clip_raster(input_orthomosaic,"<<"\n";
    strOut<<"                                                        input_rois_shp,"<<"\n";
    strOut<<"                                                        no_data_value,"<<"\n";
    strOut<<"                                                        output_path)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_orthomosaic = input_orthomosaic_rois"<<"\n";
    strOut<<"        input_rois_shp = None"<<"\n";
    strOut<<"    str_error = process(input_orthomosaic,"<<"\n";
    strOut<<"                        input_rois_shp,"<<"\n";
    strOut<<"                        factor_to_reflectance,"<<"\n";
    strOut<<"                        bands_to_use,"<<"\n";
    strOut<<"                        red_band_number,"<<"\n";
    strOut<<"                        nir_band_number,"<<"\n";
    strOut<<"                        minimum_ndvi,"<<"\n";
    strOut<<"                        maximum_ndvi,"<<"\n";
    strOut<<"                        minimum_explained_variance,"<<"\n";
    strOut<<"                        only_one_principal_component,"<<"\n";
    strOut<<"                        max_number_of_kmeans_clusters,"<<"\n";
    strOut<<"                        minimum_classification_area,"<<"\n";
    strOut<<"                        output_path)"<<"\n";
    strOut<<"    if str_error:"<<"\n";
    strOut<<"        print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    print(\"... Process finished\", flush=True)"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"if __name__ == '__main__':"<<"\n";
    strOut<<"    err = GdalErrorHandler()"<<"\n";
    strOut<<"    gdal.PushErrorHandler(err.handler)"<<"\n";
    strOut<<"    gdal.UseExceptions()  # Exceptions will get raised on anything >= gdal.CE_Failure"<<"\n";
    strOut<<"    assert err.err_level == gdal.CE_None, 'the error level starts at 0'"<<"\n";
    strOut<<"    main()"<<"\n";
    file.close();
    return(true);
}

bool PAFyCToolsDialog::writePythonProgramMonitoringFloraAtHighAltitude(QString pythonFileName,
                                                                       QString &strError)
{
    QString command=PAFYCTOOLSGUI_COMMAND_MFHA;
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
    strOut<<""<<"\n";
    strOut<<"# import optparse"<<"\n";
    strOut<<"import argparse"<<"\n";
    strOut<<"import numpy as np"<<"\n";
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
    strOut<<"import cv2 as cv"<<"\n";
    strOut<<"import math"<<"\n";
    strOut<<"from pathlib import Path"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"class GdalErrorHandler(object):"<<"\n";
    strOut<<"    def __init__(self):"<<"\n";
    strOut<<"        self.err_level = gdal.CE_None"<<"\n";
    strOut<<"        self.err_no = 0"<<"\n";
    strOut<<"        self.err_msg = ''"<<"\n";
    strOut<<""<<"\n";
    strOut<<"    def handler(self, err_level, err_no, err_msg):"<<"\n";
    strOut<<"        self.err_level = err_level"<<"\n";
    strOut<<"        self.err_no = err_no"<<"\n";
    strOut<<"        self.err_msg = err_msg"<<"\n";
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
    strOut<<"def process(input_orthomosaic,"<<"\n";
    strOut<<"            input_rois_shp,"<<"\n";
    strOut<<"            factor_to_reflectance,"<<"\n";
    strOut<<"            bands_to_use,"<<"\n";
    strOut<<"            red_band_number,"<<"\n";
    strOut<<"            nir_band_number,"<<"\n";
    strOut<<"            minimum_ndvi,"<<"\n";
    strOut<<"            minimum_nir_reflectance,"<<"\n";
    strOut<<"            grid_spacing,"<<"\n";
    strOut<<"            minimum_explained_variance,"<<"\n";
    strOut<<"            only_one_principal_component,"<<"\n";
    strOut<<"            weight_factor_by_cluster,"<<"\n";
    strOut<<"            input_dsm,"<<"\n";
    strOut<<"            input_dtm,"<<"\n";
    strOut<<"            crop_minimum_height,"<<"\n";
    strOut<<"            output_path):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    if input_rois_shp:"<<"\n";
    strOut<<"        if not exists(input_rois_shp):"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nNot exists file: {}\".format(input_rois_shp)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    dsm_ds = None"<<"\n";
    strOut<<"    dsm_rb = None"<<"\n";
    strOut<<"    dsm_crs = None"<<"\n";
    strOut<<"    dsm_gsd_x = None"<<"\n";
    strOut<<"    dsm_gsd_y = None"<<"\n";
    strOut<<"    dsm_xSize = None"<<"\n";
    strOut<<"    dsm_ySize = None"<<"\n";
    strOut<<"    dsm_ulx = None"<<"\n";
    strOut<<"    dsm_uly = None"<<"\n";
    strOut<<"    dsm_lrx = None"<<"\n";
    strOut<<"    dsm_lry = None"<<"\n";
    strOut<<"    dsm_geotransform = None"<<"\n";
    strOut<<"    dsm_data = None"<<"\n";
    strOut<<"    dtm_ds = None"<<"\n";
    strOut<<"    dtm_rb = None"<<"\n";
    strOut<<"    dtm_crs = None"<<"\n";
    strOut<<"    dtm_gsd_x = None"<<"\n";
    strOut<<"    dtm_gsd_y = None"<<"\n";
    strOut<<"    dtm_xSize = None"<<"\n";
    strOut<<"    dtm_ySize = None"<<"\n";
    strOut<<"    dtm_ulx = None"<<"\n";
    strOut<<"    dtm_uly = None"<<"\n";
    strOut<<"    dtm_lrx = None"<<"\n";
    strOut<<"    dtm_lry = None"<<"\n";
    strOut<<"    dtm_geotransform = None"<<"\n";
    strOut<<"    dtm_data = None"<<"\n";
    strOut<<"    if crop_minimum_height > 0.0:"<<"\n";
    strOut<<"        if not exists(input_dsm):"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nNot exists file: {}\".format(input_dsm)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"        if not exists(input_dtm):"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nNot exists file: {}\".format(input_dtm)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            dsm_ds = gdal.Open(input_dsm)"<<"\n";
    strOut<<"        except ValueError:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nError opening dataset file:\\n{}\".format(input_dsm)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            dsm_rb = dsm_ds.GetRasterBand(1)"<<"\n";
    strOut<<"        except ValueError:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nError getting raster band from file:\\n{}\".format(input_dsm)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            dtm_ds = gdal.Open(input_dtm)"<<"\n";
    strOut<<"        except ValueError:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nError opening dataset file:\\n{}\".format(input_dtm)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            dtm_rb = dtm_ds.GetRasterBand(1)"<<"\n";
    strOut<<"        except ValueError:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += \"\\nError getting raster band from file:\\n{}\".format(input_dtm)"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"        dsm_crs = osr.SpatialReference()"<<"\n";
    strOut<<"        dsm_crs.ImportFromWkt(dsm_ds.GetProjectionRef())"<<"\n";
    strOut<<"        dsm_crs.SetAxisMappingStrategy(osr.OAMS_TRADITIONAL_GIS_ORDER)"<<"\n";
    strOut<<"        dsm_crs_wkt = dsm_crs.ExportToWkt()"<<"\n";
    strOut<<"        dsm_xSize = dsm_ds.RasterXSize"<<"\n";
    strOut<<"        dsm_ySize = dsm_ds.RasterYSize"<<"\n";
    strOut<<"        dsm_geotransform = dsm_ds.GetGeoTransform()"<<"\n";
    strOut<<"        dsm_gsd_x = abs(dsm_geotransform[1])"<<"\n";
    strOut<<"        dsm_gsd_y = abs(dsm_geotransform[5])"<<"\n";
    strOut<<"        dsm_ulx, dsm_xres, dsm_xskew, dsm_uly, dsm_yskew, dsm_yres = dsm_ds.GetGeoTransform()"<<"\n";
    strOut<<"        dsm_lrx = dsm_ulx + (dsm_ds.RasterXSize * dsm_xres)"<<"\n";
    strOut<<"        dsm_lry = dsm_uly + (dsm_ds.RasterYSize * dsm_yres)"<<"\n";
    strOut<<"        dsm_data = dsm_rb.ReadAsArray()"<<"\n";
    strOut<<"        dtm_crs = osr.SpatialReference()"<<"\n";
    strOut<<"        dtm_crs.ImportFromWkt(dtm_ds.GetProjectionRef())"<<"\n";
    strOut<<"        dtm_crs.SetAxisMappingStrategy(osr.OAMS_TRADITIONAL_GIS_ORDER)"<<"\n";
    strOut<<"        dtm_crs_wkt = dtm_crs.ExportToWkt()"<<"\n";
    strOut<<"        dtm_xSize = dtm_ds.RasterXSize"<<"\n";
    strOut<<"        dtm_ySize = dtm_ds.RasterYSize"<<"\n";
    strOut<<"        dtm_geotransform = dtm_ds.GetGeoTransform()"<<"\n";
    strOut<<"        dtm_gsd_x = abs(dtm_geotransform[1])"<<"\n";
    strOut<<"        dtm_gsd_y = abs(dtm_geotransform[5])"<<"\n";
    strOut<<"        dtm_ulx, dtm_xres, dtm_xskew, dtm_uly, dtm_yskew, dtm_yres = dtm_ds.GetGeoTransform()"<<"\n";
    strOut<<"        dtm_lrx = dtm_ulx + (dtm_ds.RasterXSize * dtm_xres)"<<"\n";
    strOut<<"        dtm_lry = dtm_uly + (dtm_ds.RasterYSize * dtm_yres)"<<"\n";
    strOut<<"        dtm_data = dtm_rb.ReadAsArray()"<<"\n";
    strOut<<"    orthomosaic_ds = None"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        orthomosaic_ds = gdal.Open(input_orthomosaic)"<<"\n";
    strOut<<"    except ValueError:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nError opening dataset file:\\n{}\".format(input_orthomosaic)"<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    orthomosaic_number_of_bands = orthomosaic_ds.RasterCount"<<"\n";
    strOut<<"    if red_band_number > orthomosaic_number_of_bands:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nRed band number is greather than orthomosaic number of bands\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    if nir_band_number > orthomosaic_number_of_bands:"<<"\n";
    strOut<<"        str_error = \"Function process\""<<"\n";
    strOut<<"        str_error += \"\\nNir band number is greather than orthomosaic number of bands\""<<"\n";
    strOut<<"        return str_error"<<"\n";
    strOut<<"    for band_number in bands_to_use:"<<"\n";
    strOut<<"        if band_number > orthomosaic_number_of_bands or band_number < 1:"<<"\n";
    strOut<<"            str_error = \"Function process\""<<"\n";
    strOut<<"            str_error += (\"\\nBand to use number: {} is out of valid number for orthomosaic number of bands\""<<"\n";
    strOut<<"                          .format(str(band_number)))"<<"\n";
    strOut<<"            return str_error"<<"\n";
    strOut<<"    xSize = orthomosaic_ds.RasterXSize"<<"\n";
    strOut<<"    ySize = orthomosaic_ds.RasterYSize"<<"\n";
    strOut<<"    orthomosaic_geotransform = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    gsd_x = abs(orthomosaic_geotransform[1])"<<"\n";
    strOut<<"    gsd_y = abs(orthomosaic_geotransform[5])"<<"\n";
    strOut<<"    gsd = gsd_x"<<"\n";
    strOut<<"    projection = orthomosaic_ds.GetProjection()"<<"\n";
    strOut<<"    orthomosaic_crs = osr.SpatialReference()"<<"\n";
    strOut<<"    transform_orthomosaic_to_dsm = None"<<"\n";
    strOut<<"    transform_orthomosaic_to_dtm = None"<<"\n";
    strOut<<"    orthomosaic_crs.ImportFromWkt(orthomosaic_ds.GetProjectionRef())"<<"\n";
    strOut<<"    orthomosaic_crs.SetAxisMappingStrategy(osr.OAMS_TRADITIONAL_GIS_ORDER)"<<"\n";
    strOut<<"    if crop_minimum_height > 0.0:"<<"\n";
    strOut<<"        transform_orthomosaic_to_dsm = osr.CoordinateTransformation(orthomosaic_crs, dsm_crs)"<<"\n";
    strOut<<"        transform_orthomosaic_to_dtm = osr.CoordinateTransformation(orthomosaic_crs, dtm_crs)"<<"\n";
    strOut<<"    orthomosaic_crs_wkt = orthomosaic_crs.ExportToWkt()"<<"\n";
    strOut<<"    ulx, xres, xskew, uly, yskew, yres = orthomosaic_ds.GetGeoTransform()"<<"\n";
    strOut<<"    lrx = ulx + (orthomosaic_ds.RasterXSize * xres)"<<"\n";
    strOut<<"    lry = uly + (orthomosaic_ds.RasterYSize * yres)"<<"\n";
    strOut<<"    out_ring = ogr.Geometry(ogr.wkbLinearRing)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, uly)"<<"\n";
    strOut<<"    out_ring.AddPoint(lrx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, lry)"<<"\n";
    strOut<<"    out_ring.AddPoint(ulx, uly)"<<"\n";
    strOut<<"    orthomosaic_poly = ogr.Geometry(ogr.wkbPolygon)"<<"\n";
    strOut<<"    orthomosaic_poly.AddGeometry(out_ring)"<<"\n";
    strOut<<"    rs_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    rs_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    orthomosaic_pixel_area = abs(rs_pixel_width) * abs(rs_pixel_height)"<<"\n";
    strOut<<"    orthomosaic_x_origin = orthomosaic_geotransform[0]"<<"\n";
    strOut<<"    orthomosaic_y_origin = orthomosaic_geotransform[3]"<<"\n";
    strOut<<"    orthomosaic_pixel_width = orthomosaic_geotransform[1]"<<"\n";
    strOut<<"    orthomosaic_pixel_height = orthomosaic_geotransform[5]"<<"\n";
    strOut<<"    values_by_band = {}"<<"\n";
    strOut<<"    band_red = orthomosaic_ds.GetRasterBand(red_band_number)"<<"\n";
    strOut<<"    band_red_no_data_value = band_red.GetNoDataValue()"<<"\n";
    strOut<<"    band_red_data = band_red.ReadAsArray()"<<"\n";
    strOut<<"    band_redmasked_data = np.ma.masked_where(band_red_data == band_red_no_data_value, band_red_data)"<<"\n";
    strOut<<"    band_redindexes_without_no_data_value = band_redmasked_data.nonzero()"<<"\n";
    strOut<<"    columns_by_row = {}"<<"\n";
    strOut<<"    for i in range(len(band_redindexes_without_no_data_value[0])):"<<"\n";
    strOut<<"        row = band_redindexes_without_no_data_value[0][i]"<<"\n";
    strOut<<"        column = band_redindexes_without_no_data_value[1][i]"<<"\n";
    strOut<<"        if not row in columns_by_row:"<<"\n";
    strOut<<"            columns_by_row[row] = []"<<"\n";
    strOut<<"        columns_by_row[row].append(column)"<<"\n";
    strOut<<"    band_redmasked_data = None"<<"\n";
    strOut<<"    band_redindexes_without_no_data_value = None"<<"\n";
    strOut<<"    band_nir = orthomosaic_ds.GetRasterBand(nir_band_number)"<<"\n";
    strOut<<"    band_nir_no_data_value = band_nir.GetNoDataValue()"<<"\n";
    strOut<<"    band_nir_data = band_nir.ReadAsArray()"<<"\n";
    strOut<<"    valid_indexes = []"<<"\n";
    strOut<<"    invalid_indexes = []"<<"\n";
    strOut<<"    if crop_minimum_height > 0.0:"<<"\n";
    strOut<<"        print('Filtering by NIR Reflectance, NDVI value and crop height...', flush=True)"<<"\n";
    strOut<<"    else:"<<"\n";
    strOut<<"        print('Filtering by NIR Reflectance and NDVI value ...', flush=True)"<<"\n";
    strOut<<"    ndvi_valid_values_by_row = {}"<<"\n";
    strOut<<"    for row in columns_by_row:"<<"\n";
    strOut<<"        for j in range(len(columns_by_row[row])):"<<"\n";
    strOut<<"            column = columns_by_row[row][j]"<<"\n";
    strOut<<"            index = [row, column]"<<"\n";
    strOut<<"            nir_value = band_nir_data[row][column] * factor_to_reflectance"<<"\n";
    strOut<<"            if nir_value < minimum_nir_reflectance:"<<"\n";
    strOut<<"                invalid_indexes.append(index)"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            red_value = band_red_data[row][column] * factor_to_reflectance"<<"\n";
    strOut<<"            ndvi_value = (nir_value - red_value) / (nir_value + red_value)"<<"\n";
    strOut<<"            if ndvi_value < minimum_ndvi:"<<"\n";
    strOut<<"                invalid_indexes.append(index)"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            if crop_minimum_height > 0.0:"<<"\n";
    strOut<<"                x_coord = column * gsd_x + ulx + (gsd_x / 2.)  # add half the cell size"<<"\n";
    strOut<<"                y_coord = uly - row * gsd_y - (gsd_y / 2.)  # to centre the point"<<"\n";
    strOut<<"                x_coord_dsm = x_coord"<<"\n";
    strOut<<"                y_coord_dsm = y_coord"<<"\n";
    strOut<<"                x_coord_dsm, y_coord_dsm, _ = transform_orthomosaic_to_dsm.TransformPoint(x_coord_dsm, y_coord_dsm)"<<"\n";
    strOut<<"                dsm_column = math.floor((x_coord_dsm - dsm_ulx) / dsm_gsd_x)"<<"\n";
    strOut<<"                dsm_row = math.floor((dsm_uly - y_coord_dsm) / dsm_gsd_y)"<<"\n";
    strOut<<"                if dsm_column < 0 or dsm_column > dsm_xSize:"<<"\n";
    strOut<<"                    invalid_indexes.append(index)"<<"\n";
    strOut<<"                    continue"<<"\n";
    strOut<<"                if dsm_row < 0 or dsm_row > dsm_ySize:"<<"\n";
    strOut<<"                    invalid_indexes.append(index)"<<"\n";
    strOut<<"                    continue"<<"\n";
    strOut<<"                dsm_height = dsm_data[dsm_row][dsm_column]"<<"\n";
    strOut<<"                x_coord_dtm = x_coord"<<"\n";
    strOut<<"                y_coord_dtm = y_coord"<<"\n";
    strOut<<"                x_coord_dtm, y_coord_dtm, _ = transform_orthomosaic_to_dtm.TransformPoint(x_coord_dtm, y_coord_dtm)"<<"\n";
    strOut<<"                dtm_column = math.floor((x_coord_dtm - dtm_ulx) / dtm_gsd_x)"<<"\n";
    strOut<<"                dtm_row = math.floor((dtm_uly - y_coord_dtm) / dtm_gsd_y)"<<"\n";
    strOut<<"                if dtm_column < 0 or dtm_column > dtm_xSize:"<<"\n";
    strOut<<"                    invalid_indexes.append(index)"<<"\n";
    strOut<<"                    continue"<<"\n";
    strOut<<"                if dtm_row < 0 or dtm_row > dtm_ySize:"<<"\n";
    strOut<<"                    continue"<<"\n";
    strOut<<"                dtm_height = dtm_data[dtm_row][dtm_column]"<<"\n";
    strOut<<"                crop_height = dsm_height - dtm_height"<<"\n";
    strOut<<"                if crop_height < crop_minimum_height:"<<"\n";
    strOut<<"                    invalid_indexes.append(index)"<<"\n";
    strOut<<"                    continue"<<"\n";
    strOut<<"            valid_indexes.append(index)"<<"\n";
    strOut<<"            if not row in ndvi_valid_values_by_row:"<<"\n";
    strOut<<"                ndvi_valid_values_by_row[row] = {}"<<"\n";
    strOut<<"            ndvi_valid_values_by_row[row][column] = ndvi_value"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    if crop_minimum_height > 0.0:"<<"\n";
    strOut<<"        dsm_data = None"<<"\n";
    strOut<<"        dtm_data = None"<<"\n";
    strOut<<"    data = np.zeros((len(valid_indexes), len(bands_to_use)))"<<"\n";
    strOut<<"    invalid_positions_in_valid_indexes = []"<<"\n";
    strOut<<"    for j in range(len(bands_to_use)):"<<"\n";
    strOut<<"        band_number = bands_to_use[j]"<<"\n";
    strOut<<"        print('Getting data for band {} ...'.format(band_number), flush=True)"<<"\n";
    strOut<<"        band_data = None"<<"\n";
    strOut<<"        band_no_data_value = None"<<"\n";
    strOut<<"        if band_number == red_band_number:"<<"\n";
    strOut<<"            band_data = band_red_data"<<"\n";
    strOut<<"            band_no_data_value = band_red_no_data_value"<<"\n";
    strOut<<"        elif band_number == nir_band_number:"<<"\n";
    strOut<<"            band_data = band_nir_data"<<"\n";
    strOut<<"            band_no_data_value = band_nir_no_data_value"<<"\n";
    strOut<<"        else:"<<"\n";
    strOut<<"            band = orthomosaic_ds.GetRasterBand(band_number)"<<"\n";
    strOut<<"            band_no_data_value = band.GetNoDataValue()"<<"\n";
    strOut<<"            band_data = band.ReadAsArray()  # rows, columns"<<"\n";
    strOut<<"        for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"            if i in invalid_positions_in_valid_indexes:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            row = valid_indexes[i][0]"<<"\n";
    strOut<<"            column = valid_indexes[i][1]"<<"\n";
    strOut<<"            value = band_data[row][column]"<<"\n";
    strOut<<"            if value == band_no_data_value:"<<"\n";
    strOut<<"                index = [row, column]"<<"\n";
    strOut<<"                invalid_positions_in_valid_indexes.append(i)"<<"\n";
    strOut<<"            data[i][j] = value * factor_to_reflectance"<<"\n";
    strOut<<"        print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    band_red_data = None"<<"\n";
    strOut<<"    band_nir_data = None"<<"\n";
    strOut<<"    number_of_kmeans_clusters = len(weight_factor_by_cluster)"<<"\n";
    strOut<<"    if len(invalid_positions_in_valid_indexes) > 0:"<<"\n";
    strOut<<"        print('Removing pixels with no data value in some band ...', flush=True)"<<"\n";
    strOut<<"        valid_indexes_with_out_outliers = []"<<"\n";
    strOut<<"        data_without_outliers = np.zeros((len(valid_indexes) - len(invalid_positions_in_valid_indexes),"<<"\n";
    strOut<<"                                          len(bands_to_use)))"<<"\n";
    strOut<<"        pos = -1"<<"\n";
    strOut<<"        for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"            if i in invalid_positions_in_valid_indexes:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            pos = pos + 1"<<"\n";
    strOut<<"            row = valid_indexes[i][0]"<<"\n";
    strOut<<"            column = valid_indexes[i][1]"<<"\n";
    strOut<<"            for j in range(len(bands_to_use)):"<<"\n";
    strOut<<"                data_without_outliers[pos][j] = data[i][j]"<<"\n";
    strOut<<"            index = [row, column]"<<"\n";
    strOut<<"            valid_indexes_with_out_outliers.append(index)"<<"\n";
    strOut<<"        data = None"<<"\n";
    strOut<<"        valid_indexes = None"<<"\n";
    strOut<<"        data = data_without_outliers"<<"\n";
    strOut<<"        valid_indexes = valid_indexes_with_out_outliers"<<"\n";
    strOut<<"        data_without_outliers = None"<<"\n";
    strOut<<"        valid_indexes_with_out_outliers = None"<<"\n";
    strOut<<"        print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    print('Computing principal components and transforming data values to new base ...', flush=True)"<<"\n";
    strOut<<"    standardized_data = (data - data.mean(axis=0)) / data.std(axis=0)"<<"\n";
    strOut<<"    data = None"<<"\n";
    strOut<<"    covariance_matrix = np.cov(standardized_data, ddof=1, rowvar=False)"<<"\n";
    strOut<<"    eigenvalues, eigenvectors = np.linalg.eig(covariance_matrix)"<<"\n";
    strOut<<"    # np.argsort can only provide lowest to highest; use [::-1] to reverse the list"<<"\n";
    strOut<<"    order_of_importance = np.argsort(eigenvalues)[::-1]"<<"\n";
    strOut<<"    # utilize the sort order to sort eigenvalues and eigenvectors"<<"\n";
    strOut<<"    sorted_eigenvalues = eigenvalues[order_of_importance]"<<"\n";
    strOut<<"    sorted_eigenvectors = eigenvectors[:, order_of_importance]  # sort the columns"<<"\n";
    strOut<<"    # use sorted_eigenvalues to ensure the explained variances correspond to the eigenvectors"<<"\n";
    strOut<<"    explained_variance = sorted_eigenvalues / np.sum(sorted_eigenvalues)"<<"\n";
    strOut<<"    explained_variance_by_selected_components = 0"<<"\n";
    strOut<<"    number_of_pca_components = 0"<<"\n";
    strOut<<"    while explained_variance_by_selected_components < minimum_explained_variance:"<<"\n";
    strOut<<"        explained_variance_by_selected_components = (explained_variance_by_selected_components"<<"\n";
    strOut<<"                                                     + explained_variance[number_of_pca_components])"<<"\n";
    strOut<<"        number_of_pca_components = number_of_pca_components + 1"<<"\n";
    strOut<<"    if number_of_pca_components > 1 and only_one_principal_component:"<<"\n";
    strOut<<"        number_of_pca_components = 1"<<"\n";
    strOut<<"    reduced_data = np.matmul(standardized_data,"<<"\n";
    strOut<<"                             sorted_eigenvectors[:, :number_of_pca_components])  # transform the original data"<<"\n";
    strOut<<"    standardized_data = None"<<"\n";
    strOut<<"    criteria = (cv.TERM_CRITERIA_MAX_ITER, 100, 1.0)"<<"\n";
    strOut<<"    flags = cv.KMEANS_RANDOM_CENTERS"<<"\n";
    strOut<<"    input_values_cv = np.zeros([len(valid_indexes), number_of_pca_components], dtype=np.float32)"<<"\n";
    strOut<<"    for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"        for j in range(number_of_pca_components):"<<"\n";
    strOut<<"            input_values_cv[i][j] = reduced_data[i][j]"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    print('Computing results for {} clusters ...'.format(str(number_of_kmeans_clusters)), flush=True)"<<"\n";
    strOut<<"    compactness, labels, centers = cv.kmeans(input_values_cv, number_of_kmeans_clusters,"<<"\n";
    strOut<<"                                             None, criteria, 10, flags)"<<"\n";
    strOut<<"    ndvi_by_cluster = {}"<<"\n";
    strOut<<"    for i in range(len(valid_indexes)):"<<"\n";
    strOut<<"        row = valid_indexes[i][0]"<<"\n";
    strOut<<"        column = valid_indexes[i][1]"<<"\n";
    strOut<<"        n_cluster = labels[i].item() + 1"<<"\n";
    strOut<<"        if row in ndvi_valid_values_by_row:"<<"\n";
    strOut<<"            if column in ndvi_valid_values_by_row[row]:"<<"\n";
    strOut<<"                if not n_cluster in ndvi_by_cluster:"<<"\n";
    strOut<<"                    ndvi_by_cluster[n_cluster] = {}"<<"\n";
    strOut<<"                    ndvi_by_cluster[n_cluster]['values'] = []"<<"\n";
    strOut<<"                    ndvi_by_cluster[n_cluster]['values_rows'] = []"<<"\n";
    strOut<<"                    ndvi_by_cluster[n_cluster]['values_columns'] = []"<<"\n";
    strOut<<"                    ndvi_by_cluster[n_cluster]['mean'] = 0."<<"\n";
    strOut<<"                    ndvi_by_cluster[n_cluster]['std'] = 0."<<"\n";
    strOut<<"                ndvi_value = ndvi_valid_values_by_row[row][column]"<<"\n";
    strOut<<"                ndvi_by_cluster[n_cluster]['values'].append(ndvi_value)"<<"\n";
    strOut<<"                ndvi_by_cluster[n_cluster]['values_rows'].append(row)"<<"\n";
    strOut<<"                ndvi_by_cluster[n_cluster]['values_columns'].append(column)"<<"\n";
    strOut<<"                ndvi_by_cluster[n_cluster]['mean'] = ndvi_by_cluster[n_cluster]['mean'] + ndvi_value"<<"\n";
    strOut<<"    output_grid_by_column_by_row = {}"<<"\n";
    strOut<<"    for n_cluster in ndvi_by_cluster:"<<"\n";
    strOut<<"        number_of_values = len(ndvi_by_cluster[n_cluster]['values'])"<<"\n";
    strOut<<"        ndvi_by_cluster[n_cluster]['mean'] = ndvi_by_cluster[n_cluster]['mean'] / float(number_of_values)"<<"\n";
    strOut<<"        # if number_of_values > 1:"<<"\n";
    strOut<<"        ndvi_mean_value = ndvi_by_cluster[n_cluster]['mean']"<<"\n";
    strOut<<"        for i_ndvi_value in range(len(ndvi_by_cluster[n_cluster]['values'])):"<<"\n";
    strOut<<"            ndvi_value = ndvi_by_cluster[n_cluster]['values'][i_ndvi_value]"<<"\n";
    strOut<<"            ndvi_column = ndvi_by_cluster[n_cluster]['values_columns'][i_ndvi_value]"<<"\n";
    strOut<<"            ndvi_row = ndvi_by_cluster[n_cluster]['values_rows'][i_ndvi_value]"<<"\n";
    strOut<<"            x_coord = ndvi_column * gsd_x + ulx + (gsd_x / 2.)  # add half the cell size"<<"\n";
    strOut<<"            y_coord = uly - ndvi_row * gsd_y - (gsd_y / 2.)  # to centre the point"<<"\n";
    strOut<<"            grid_column = math.floor((x_coord - math.floor(ulx)) / grid_spacing)"<<"\n";
    strOut<<"            # grid_row = math.floor((x_coord - math.ceil(uly)) / grid_spacing)"<<"\n";
    strOut<<"            grid_row = math.floor((math.ceil(uly) - y_coord) / grid_spacing)"<<"\n";
    strOut<<"            if not grid_column in output_grid_by_column_by_row:"<<"\n";
    strOut<<"                output_grid_by_column_by_row[grid_column] = {}"<<"\n";
    strOut<<"            if not grid_row in output_grid_by_column_by_row[grid_column]:"<<"\n";
    strOut<<"                output_grid_by_column_by_row[grid_column][grid_row] = {}"<<"\n";
    strOut<<"            if not n_cluster in output_grid_by_column_by_row[grid_column][grid_row]:"<<"\n";
    strOut<<"                output_grid_by_column_by_row[grid_column][grid_row][n_cluster] = 0"<<"\n";
    strOut<<"            output_grid_by_column_by_row[grid_column][grid_row][n_cluster] = ("<<"\n";
    strOut<<"                    output_grid_by_column_by_row[grid_column][grid_row][n_cluster] + 1)"<<"\n";
    strOut<<"            ndvi_diff_value = ndvi_value - ndvi_mean_value"<<"\n";
    strOut<<"            ndvi_by_cluster[n_cluster]['std'] = ndvi_by_cluster[n_cluster]['std'] + ndvi_diff_value ** 2."<<"\n";
    strOut<<"        ndvi_by_cluster[n_cluster]['std'] = math.sqrt(ndvi_by_cluster[n_cluster]['std'] / number_of_values)"<<"\n";
    strOut<<"    cluster_descending_order_position = []"<<"\n";
    strOut<<"    position_in_vector_descending_order_by_cluster = {}"<<"\n";
    strOut<<"    for n_cluster in ndvi_by_cluster:"<<"\n";
    strOut<<"        max_mean_ndvi_value = 0."<<"\n";
    strOut<<"        n_cluster_max_mean_ndvi_value = -1"<<"\n";
    strOut<<"        for n_cluster_bis in ndvi_by_cluster:"<<"\n";
    strOut<<"            if n_cluster_bis in cluster_descending_order_position:"<<"\n";
    strOut<<"                continue"<<"\n";
    strOut<<"            if ndvi_by_cluster[n_cluster_bis]['mean'] > max_mean_ndvi_value:"<<"\n";
    strOut<<"                max_mean_ndvi_value = ndvi_by_cluster[n_cluster_bis]['mean']"<<"\n";
    strOut<<"                n_cluster_max_mean_ndvi_value = n_cluster_bis"<<"\n";
    strOut<<"        position_in_vector_descending_order_by_cluster[n_cluster_max_mean_ndvi_value] \\"<<"\n";
    strOut<<"            = len(cluster_descending_order_position)"<<"\n";
    strOut<<"        cluster_descending_order_position.append(n_cluster_max_mean_ndvi_value)"<<"\n";
    strOut<<"    fileformat = \"GTiff\""<<"\n";
    strOut<<"    driver = gdal.GetDriverByName(fileformat)"<<"\n";
    strOut<<"    if not output_path:"<<"\n";
    strOut<<"        output_path = os.path.dirname(os.path.abspath(input_orthomosaic))"<<"\n";
    strOut<<"    var_path = Path(input_orthomosaic)"<<"\n";
    strOut<<"    base_name = var_path.stem"<<"\n";
    strOut<<"    file_ext = '.tif'"<<"\n";
    strOut<<"    for i_n_cluster in range(len(cluster_descending_order_position)):"<<"\n";
    strOut<<"        n_cluster = cluster_descending_order_position[i_n_cluster]"<<"\n";
    strOut<<"        str_mean = str(int(round(ndvi_by_cluster[n_cluster]['mean'] * 100)))"<<"\n";
    strOut<<"        str_std = str(int(round(ndvi_by_cluster[n_cluster]['std'] * 100)))"<<"\n";
    strOut<<"        dst_filename = (output_path + '/' + base_name + '_nckms_' + str(i_n_cluster+1)"<<"\n";
    strOut<<"                        + '_mean_' + str_mean + '_std_' + str_std + file_ext)"<<"\n";
    strOut<<"        dst_filename = os.path.normpath(dst_filename)"<<"\n";
    strOut<<"        dst_ds = driver.Create(dst_filename, xsize=xSize, ysize=ySize, bands=1, eType=gdal.GDT_Byte)"<<"\n";
    strOut<<"        dst_ds.SetGeoTransform(orthomosaic_geotransform)"<<"\n";
    strOut<<"        dst_ds.SetProjection(projection)"<<"\n";
    strOut<<"        # np_raster = np.zeros((ySize, xSize), dtype=np.uint8)"<<"\n";
    strOut<<"        np_raster = np.full((ySize, xSize), 255, dtype=np.uint8)"<<"\n";
    strOut<<"        for i_ndvi_value in range(len(ndvi_by_cluster[n_cluster]['values'])):"<<"\n";
    strOut<<"            ndvi_value = ndvi_by_cluster[n_cluster]['values'][i_ndvi_value]"<<"\n";
    strOut<<"            ndvi_row = ndvi_by_cluster[n_cluster]['values_rows'][i_ndvi_value]"<<"\n";
    strOut<<"            ndvi_column = ndvi_by_cluster[n_cluster]['values_columns'][i_ndvi_value]"<<"\n";
    strOut<<"            np_raster[ndvi_row][ndvi_column] = int(round(ndvi_value * 100))"<<"\n";
    strOut<<"        dst_ds.GetRasterBand(1).SetNoDataValue(0)"<<"\n";
    strOut<<"        dst_ds.GetRasterBand(1).WriteArray(np_raster)"<<"\n";
    strOut<<"        dst_ds = None"<<"\n";
    strOut<<"    outShapefile = (output_path + '/' + base_name + '_grid.shp')"<<"\n";
    strOut<<"    outShapefile = os.path.normpath(outShapefile)"<<"\n";
    strOut<<"    outDriver = ogr.GetDriverByName(\"ESRI Shapefile\")"<<"\n";
    strOut<<"    # Remove output shapefile if it already exists"<<"\n";
    strOut<<"    if os.path.exists(outShapefile):"<<"\n";
    strOut<<"        outDriver.DeleteDataSource(outShapefile)"<<"\n";
    strOut<<"    # Create the output shapefile"<<"\n";
    strOut<<"    outDataSource = outDriver.CreateDataSource(outShapefile)"<<"\n";
    strOut<<"    outLayer = outDataSource.CreateLayer(\"grid\", orthomosaic_crs, ogr.wkbPolygon)"<<"\n";
    strOut<<"    idField = ogr.FieldDefn(\"id\", ogr.OFTInteger)"<<"\n";
    strOut<<"    outLayer.CreateField(idField)"<<"\n";
    strOut<<"    fractionCoverField = ogr.FieldDefn(\"frac_cover\", ogr.OFTReal)"<<"\n";
    strOut<<"    fractionCoverField.SetPrecision(2)"<<"\n";
    strOut<<"    outLayer.CreateField(fractionCoverField)"<<"\n";
    strOut<<"    indexField = ogr.FieldDefn(\"index\",ogr.OFTReal)"<<"\n";
    strOut<<"    indexField.SetPrecision(2)"<<"\n";
    strOut<<"    outLayer.CreateField(indexField)"<<"\n";
    strOut<<"    for i_n_cluster in range(len(cluster_descending_order_position)):"<<"\n";
    strOut<<"        cluster_percentage_field_name = \"cl_fc_\" + str(i_n_cluster + 1)"<<"\n";
    strOut<<"        cluster_percentage_Field = ogr.FieldDefn(cluster_percentage_field_name, ogr.OFTReal)"<<"\n";
    strOut<<"        cluster_percentage_Field.SetPrecision(2)"<<"\n";
    strOut<<"        outLayer.CreateField(cluster_percentage_Field)"<<"\n";
    strOut<<"        cluster_index_field_name = \"cl_idx_\" + str(i_n_cluster + 1)"<<"\n";
    strOut<<"        cluster_index_Field = ogr.FieldDefn(cluster_index_field_name, ogr.OFTReal)"<<"\n";
    strOut<<"        cluster_index_Field.SetPrecision(2)"<<"\n";
    strOut<<"        outLayer.CreateField(cluster_index_Field)"<<"\n";
    strOut<<"    feature_count = 0"<<"\n";
    strOut<<"    number_of_pixels_in_grid = round((grid_spacing * grid_spacing) / (gsd_x * gsd_y))"<<"\n";
    strOut<<"    for grid_column in output_grid_by_column_by_row:"<<"\n";
    strOut<<"        for grid_row in output_grid_by_column_by_row[grid_column]:"<<"\n";
    strOut<<"            featureDefn = outLayer.GetLayerDefn()"<<"\n";
    strOut<<"            feature = ogr.Feature(featureDefn)"<<"\n";
    strOut<<"            ring = ogr.Geometry(ogr.wkbLinearRing)"<<"\n";
    strOut<<"            grid_ul_x = math.floor(ulx) + grid_column * grid_spacing"<<"\n";
    strOut<<"            grid_ul_y = math.ceil(uly) - grid_row * grid_spacing"<<"\n";
    strOut<<"            grid_ur_x = math.floor(ulx) + (grid_column + 1) * grid_spacing"<<"\n";
    strOut<<"            grid_ur_y = math.ceil(uly) - grid_row * grid_spacing"<<"\n";
    strOut<<"            grid_lr_x = math.floor(ulx) + (grid_column + 1) * grid_spacing"<<"\n";
    strOut<<"            grid_lr_y = math.ceil(uly) - (grid_row + 1) * grid_spacing"<<"\n";
    strOut<<"            grid_ll_x = math.floor(ulx) + grid_column * grid_spacing"<<"\n";
    strOut<<"            grid_ll_y = math.ceil(uly) - (grid_row + 1) * grid_spacing"<<"\n";
    strOut<<"            ring.AddPoint(grid_ul_x, grid_ul_y)"<<"\n";
    strOut<<"            ring.AddPoint(grid_ur_x, grid_ur_y)"<<"\n";
    strOut<<"            ring.AddPoint(grid_lr_x, grid_lr_y)"<<"\n";
    strOut<<"            ring.AddPoint(grid_ll_x, grid_ll_y)"<<"\n";
    strOut<<"            ring.AddPoint(grid_ul_x, grid_ul_y)"<<"\n";
    strOut<<"            poly = ogr.Geometry(ogr.wkbPolygon)"<<"\n";
    strOut<<"            poly.AddGeometry(ring)"<<"\n";
    strOut<<"            feature.SetGeometry(poly)"<<"\n";
    strOut<<"            feature_count = feature_count + 1"<<"\n";
    strOut<<"            feature.SetField(\"id\", feature_count)"<<"\n";
    strOut<<"            number_of_pixels_in_clusters = 0"<<"\n";
    strOut<<"            for n_cluster in output_grid_by_column_by_row[grid_column][grid_row]:"<<"\n";
    strOut<<"                ordered_cluster = position_in_vector_descending_order_by_cluster[n_cluster]"<<"\n";
    strOut<<"                number_of_pixels_in_clusters = (number_of_pixels_in_clusters"<<"\n";
    strOut<<"                                                + output_grid_by_column_by_row[grid_column][grid_row][n_cluster])"<<"\n";
    strOut<<"            # fraction_cover = (number_of_pixels_in_clusters * xSize * ySize) / (grid_spacing * grid_spacing) * 100."<<"\n";
    strOut<<"            fraction_cover = ( number_of_pixels_in_clusters / number_of_pixels_in_grid ) * 100."<<"\n";
    strOut<<"            feature.SetField(\"frac_cover\", fraction_cover)"<<"\n";
    strOut<<"            index = 0."<<"\n";
    strOut<<"            for n_cluster in output_grid_by_column_by_row[grid_column][grid_row]:"<<"\n";
    strOut<<"                ordered_cluster = position_in_vector_descending_order_by_cluster[n_cluster]"<<"\n";
    strOut<<"                # percentage_pixels_in_cluster = (output_grid_by_column_by_row[grid_column][n_cluster]"<<"\n";
    strOut<<"                #                                 / number_of_pixels_in_clusters * 100.)"<<"\n";
    strOut<<"                percentage_pixels_in_cluster = float((output_grid_by_column_by_row[grid_column][grid_row][n_cluster]"<<"\n";
    strOut<<"                                                / number_of_pixels_in_grid ) * 100.)"<<"\n";
    strOut<<"                index_in_cluster = float(weight_factor_by_cluster[ordered_cluster] * percentage_pixels_in_cluster)"<<"\n";
    strOut<<"                index = index + index_in_cluster"<<"\n";
    strOut<<"                cluster_percentage_field_name = \"cl_fc_\" + str(ordered_cluster + 1)"<<"\n";
    strOut<<"                cluster_index_field_name = \"cl_idx_\" + str(ordered_cluster + 1)"<<"\n";
    strOut<<"                feature.SetField(cluster_percentage_field_name, percentage_pixels_in_cluster)"<<"\n";
    strOut<<"                feature.SetField(cluster_index_field_name, index_in_cluster)"<<"\n";
    strOut<<"            feature.SetField(\"index\", index)"<<"\n";
    strOut<<"            outLayer.CreateFeature(feature)"<<"\n";
    strOut<<"            feature = None"<<"\n";
    strOut<<"    outDataSource = None"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    return str_error"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def clip_raster(input_raster,"<<"\n";
    strOut<<"                input_shp,"<<"\n";
    strOut<<"                no_data_value,"<<"\n";
    strOut<<"                output_path,"<<"\n";
    strOut<<"                remove_existing):"<<"\n";
    strOut<<"    str_error = None"<<"\n";
    strOut<<"    output_raster = \"\""<<"\n";
    strOut<<"    if not exists(input_shp):"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_shp)"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    if not exists(input_raster):"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error += \"\\nNot exists file: {}\".format(input_raster)"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    shp_var_path = Path(input_shp)"<<"\n";
    strOut<<"    shp_base_name = shp_var_path.stem"<<"\n";
    strOut<<"    output_raster_suffix = ''"<<"\n";
    strOut<<"    if not output_path:"<<"\n";
    strOut<<"        output_path = os.path.dirname(os.path.abspath(input_raster))"<<"\n";
    strOut<<"    raster_base_name = os.path.basename(input_raster).split('.')[0]"<<"\n";
    strOut<<"    output_raster = output_path + '\\\\' + raster_base_name"<<"\n";
    strOut<<"    # output_raster = os.path.splitext(input_raster)[0]"<<"\n";
    strOut<<"    output_raster = output_raster + \"_rois\""<<"\n";
    strOut<<"    output_raster = output_raster + \"_\""<<"\n";
    strOut<<"    output_raster = output_raster + shp_base_name"<<"\n";
    strOut<<"    output_raster = output_raster + os.path.splitext(input_raster)[1]"<<"\n";
    strOut<<"    # return str_error, output_raster"<<"\n";
    strOut<<"    if exists(output_raster):"<<"\n";
    strOut<<"        if not remove_existing:"<<"\n";
    strOut<<"            return str_error, output_raster"<<"\n";
    strOut<<"        try:"<<"\n";
    strOut<<"            os.remove(output_raster)"<<"\n";
    strOut<<"        except FileNotFoundError as e:"<<"\n";
    strOut<<"            str_error = (\"Removing output raster:\\n{}\".format(output_raster))"<<"\n";
    strOut<<"            str_error = str_error + (\"\\nError\\t\" + e.strerror)"<<"\n";
    strOut<<"            return str_error, output_raster"<<"\n";
    strOut<<"        except OSError as e:"<<"\n";
    strOut<<"            str_error = (\"Removing output raster:\\n{}\".format(output_raster))"<<"\n";
    strOut<<"            str_error = str_error + (\"\\nError\\t\" + e.strerror)"<<"\n";
    strOut<<"            return str_error, output_raster"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        input_raster_ds = gdal.Open(input_raster)"<<"\n";
    strOut<<"    except Exception as e:"<<"\n";
    strOut<<"        assert err.err_level == gdal.CE_Failure, ("<<"\n";
    strOut<<"            'The handler error level should now be at failure')"<<"\n";
    strOut<<"        assert err.err_msg == e.args[0], 'raised exception should contain the message'"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error = ('Handled warning: level={}, no={}, msg={}'.format("<<"\n";
    strOut<<"            err.err_level, err.err_no, err.err_msg))"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    raster_no_data_value = input_raster_ds.GetRasterBand(1).GetNoDataValue()"<<"\n";
    strOut<<"    if not raster_no_data_value:"<<"\n";
    strOut<<"        datatype = gdal.GetDataTypeName(input_raster_ds.GetRasterBand(1).DataType)"<<"\n";
    strOut<<"        raster_no_data_value = no_data_value"<<"\n";
    strOut<<"    orthomosaic_geotransform = input_raster_ds.GetGeoTransform()"<<"\n";
    strOut<<"    gsd_x = abs(orthomosaic_geotransform[1])"<<"\n";
    strOut<<"    gsd_y = abs(orthomosaic_geotransform[5])"<<"\n";
    strOut<<"    gdalwarp_str_options = \" -cutline \" + input_shp"<<"\n";
    strOut<<"    gdalwarp_str_options += \" -crop_to_cutline -dstnodata \" + str(no_data_value)"<<"\n";
    strOut<<"    gdalwarp_str_options += \" -tr \""<<"\n";
    strOut<<"    gdalwarp_str_options += \"{:.12f}\".format(gsd_x)"<<"\n";
    strOut<<"    gdalwarp_str_options += \" \""<<"\n";
    strOut<<"    gdalwarp_str_options += \"{:.12f}\".format(gsd_y)"<<"\n";
    strOut<<"    gdalwarp_str_options += \" -co COMPRESS=LZW\""<<"\n";
    strOut<<"    print('Clipping raster ...', flush=True)"<<"\n";
    strOut<<"    try:"<<"\n";
    strOut<<"        output_raster_ds = gdal.Warp(output_raster, input_raster_ds, options=gdalwarp_str_options)"<<"\n";
    strOut<<"    except Exception as e:"<<"\n";
    strOut<<"        assert err.err_level == gdal.CE_Failure, ("<<"\n";
    strOut<<"            'The handler error level should now be at failure')"<<"\n";
    strOut<<"        assert err.err_msg == e.args[0], 'raised exception should contain the message'"<<"\n";
    strOut<<"        str_error = \"Function clip_raster\""<<"\n";
    strOut<<"        str_error = ('Handled warning: level={}, no={}, msg={}'.format("<<"\n";
    strOut<<"            err.err_level, err.err_no, err.err_msg))"<<"\n";
    strOut<<"        return str_error, output_raster"<<"\n";
    strOut<<"    print('   ... Process finished', flush=True)"<<"\n";
    strOut<<"    return str_error, output_raster"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"def main():"<<"\n";
    strOut<<"    parser = argparse.ArgumentParser()"<<"\n";
    strOut<<"    parser.add_argument(\"--input_orthomosaic\", help=\"Input orthomosaic\", type=str)"<<"\n";
    strOut<<"    parser.add_argument(\"--no_data_value\", type=int,"<<"\n";
    strOut<<"                        help=\"Raster no data value, if not defined in file\")"<<"\n";
    strOut<<"    parser.add_argument(\"--input_rois_shp\", help=\"Input input rois shapefile, if exists\", type=str)"<<"\n";
    strOut<<"    parser.add_argument(\"--factor_to_reflectance\", type=float,"<<"\n";
    strOut<<"                        help=\"Multiplicative factor for convert raster values to reflectance\")"<<"\n";
    strOut<<"    parser.add_argument(\"--bands_to_use\", nargs=\"+\", type=int, help=\"Bands to use, starting 1\")"<<"\n";
    strOut<<"    parser.add_argument(\"--red_band_number\", type=int, help=\"Red band number, starting 1\")"<<"\n";
    strOut<<"    parser.add_argument(\"--nir_band_number\", type=int, help=\"Nir band number, starting 1\")"<<"\n";
    strOut<<"    parser.add_argument(\"--minimum_ndvi\", type=float, help=\"Minimmum NDVI, in range [-1,-1]\")"<<"\n";
    strOut<<"    parser.add_argument(\"--minimum_nir_reflectance\", type=float, help=\"Minimmum NIR reflectance, in range [0,-1]\")"<<"\n";
    strOut<<"    parser.add_argument(\"--minimum_explained_variance\", type=float,"<<"\n";
    strOut<<"                        help=\"Minimmum explained variance by PCA components, in range [0,1]\")"<<"\n";
    strOut<<"    parser.add_argument(\"--only_one_principal_component\", type=int,"<<"\n";
    strOut<<"                        help=\"Use only one principal compponent. Not if compute from explained variance\")"<<"\n";
    strOut<<"    parser.add_argument(\"--grid_spacing\", type=float,"<<"\n";
    strOut<<"                        help=\"Grid spacing, in meters\")"<<"\n";
    strOut<<"    parser.add_argument(\"--weight_factor_by_cluster\", nargs=\"+\", type=float,"<<"\n";
    strOut<<"                        help=\"Weight factor by cluster\")"<<"\n";
    strOut<<"    parser.add_argument(\"--input_dsm\", dest=\"input_dsm\", action=\"store\", type=str,"<<"\n";
    strOut<<"                        help=\"DSM geotiff, or '' for no use it, crop_minimum_height == 0.0\", default=None)"<<"\n";
    strOut<<"    parser.add_argument(\"--input_dtm\", dest=\"input_dtm\", action=\"store\", type=str,"<<"\n";
    strOut<<"                        help=\"DTM geotiff, or '' for no use it, crop_minimum_height == 0.0\", default=None)"<<"\n";
    strOut<<"    parser.add_argument(\"--crop_minimum_height, or 0.0 for no use it\", dest=\"crop_minimum_height\", action=\"store\", type=float,"<<"\n";
    strOut<<"                        help=\"Crop minimum height, in meters\", default=None)"<<"\n";
    strOut<<"    parser.add_argument(\"--output_path\", type=str,"<<"\n";
    strOut<<"                        help=\"Output path or empty for multispectral orthomosaic path\")"<<"\n";
    strOut<<"    args = parser.parse_args()"<<"\n";
    strOut<<"    if not args.input_orthomosaic:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    input_orthomosaic = args.input_orthomosaic"<<"\n";
    strOut<<"    if not exists(input_orthomosaic):"<<"\n";
    strOut<<"        print(\"Error:\\nInput orthomosaic does not exists:\\n{}\".format(input_orthomosaic))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not args.no_data_value:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"    no_data_value = args.no_data_value"<<"\n";
    strOut<<"    input_rois_shp = None"<<"\n";
    strOut<<"    if args.input_rois_shp:"<<"\n";
    strOut<<"        input_rois_shp = args.input_rois_shp"<<"\n";
    strOut<<"        if input_rois_shp:"<<"\n";
    strOut<<"            if not exists(input_rois_shp):"<<"\n";
    strOut<<"                print(\"Error:\\nInput ROIs shapefile does not exists:\\n{}\".format(input_rois_shp))"<<"\n";
    strOut<<"                return"<<"\n";
    strOut<<"    if not args.factor_to_reflectance:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"    factor_to_reflectance = args.factor_to_reflectance"<<"\n";
    strOut<<"    if not args.bands_to_use:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    bands_to_use = args.bands_to_use"<<"\n";
    strOut<<"    if not args.red_band_number:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    red_band_number = args.red_band_number"<<"\n";
    strOut<<"    if not args.nir_band_number:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    nir_band_number = args.nir_band_number"<<"\n";
    strOut<<"    if not args.minimum_ndvi:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    minimum_ndvi = args.minimum_ndvi"<<"\n";
    strOut<<"    if not args.minimum_nir_reflectance:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    minimum_nir_reflectance = args.minimum_nir_reflectance"<<"\n";
    strOut<<"    if not args.grid_spacing:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    grid_spacing = args.grid_spacing"<<"\n";
    strOut<<"    if not args.minimum_explained_variance:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    minimum_explained_variance = args.minimum_explained_variance"<<"\n";
    strOut<<"    if not args.only_one_principal_component:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    int_only_one_principal_component = args.only_one_principal_component"<<"\n";
    strOut<<"    if int_only_one_principal_component < 0 or args.only_one_principal_component > 1:"<<"\n";
    strOut<<"        print(\"Error:\\nParameter only_one_principal_component must be 0 or 1\")"<<"\n";
    strOut<<"    only_one_principal_component = False"<<"\n";
    strOut<<"    if int_only_one_principal_component == 1:"<<"\n";
    strOut<<"        only_one_principal_component = True"<<"\n";
    strOut<<"    if not args.weight_factor_by_cluster:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"    weight_factor_by_cluster = args.weight_factor_by_cluster"<<"\n";
    strOut<<"    if not args.input_dsm:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if not args.input_dtm:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    if args.crop_minimum_height == None:"<<"\n";
    strOut<<"        parser.print_help()"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    crop_minimum_height = args.crop_minimum_height"<<"\n";
    strOut<<"    input_dsm = ''"<<"\n";
    strOut<<"    input_dtm = ''"<<"\n";
    strOut<<"    if crop_minimum_height > 0.0:"<<"\n";
    strOut<<"        input_dsm = args.input_dsm"<<"\n";
    strOut<<"        if not exists(input_dsm):"<<"\n";
    strOut<<"            print(\"Error:\\nInput DSM does not exists:\\n{}\".format(input_dsm))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_dtm = args.input_dtm"<<"\n";
    strOut<<"        if not exists(input_dtm):"<<"\n";
    strOut<<"            print(\"Error:\\nInput DTM does not exists:\\n{}\".format(input_dtm))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    output_path = args.output_path"<<"\n";
    strOut<<"    if output_path:"<<"\n";
    strOut<<"        if not exists(output_path):"<<"\n";
    strOut<<"            print(\"Error:\\nOutput path does not exists:\\n{}\".format(output_path))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"    if input_rois_shp:"<<"\n";
    strOut<<"        remove_existing = False"<<"\n";
    strOut<<"        str_error, input_orthomosaic_rois = clip_raster(input_orthomosaic,"<<"\n";
    strOut<<"                                                        input_rois_shp,"<<"\n";
    strOut<<"                                                        no_data_value,"<<"\n";
    strOut<<"                                                        output_path,"<<"\n";
    strOut<<"                                                        remove_existing)"<<"\n";
    strOut<<"        if str_error:"<<"\n";
    strOut<<"            print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"            return"<<"\n";
    strOut<<"        input_orthomosaic = input_orthomosaic_rois"<<"\n";
    strOut<<"        input_rois_shp = None"<<"\n";
    strOut<<"    str_error = process(input_orthomosaic,"<<"\n";
    strOut<<"                        input_rois_shp,"<<"\n";
    strOut<<"                        factor_to_reflectance,"<<"\n";
    strOut<<"                        bands_to_use,"<<"\n";
    strOut<<"                        red_band_number,"<<"\n";
    strOut<<"                        nir_band_number,"<<"\n";
    strOut<<"                        minimum_ndvi,"<<"\n";
    strOut<<"                        minimum_nir_reflectance,"<<"\n";
    strOut<<"                        grid_spacing,"<<"\n";
    strOut<<"                        minimum_explained_variance,"<<"\n";
    strOut<<"                        only_one_principal_component,"<<"\n";
    strOut<<"                        weight_factor_by_cluster,"<<"\n";
    strOut<<"                        input_dsm,"<<"\n";
    strOut<<"                        input_dtm,"<<"\n";
    strOut<<"                        crop_minimum_height,"<<"\n";
    strOut<<"                        output_path)"<<"\n";
    strOut<<"    if str_error:"<<"\n";
    strOut<<"        print(\"Error:\\n{}\".format(str_error))"<<"\n";
    strOut<<"        return"<<"\n";
    strOut<<"    print(\"... Process finished\", flush=True)"<<"\n";
    strOut<<""<<"\n";
    strOut<<""<<"\n";
    strOut<<"if __name__ == '__main__':"<<"\n";
    strOut<<"    err = GdalErrorHandler()"<<"\n";
    strOut<<"    gdal.PushErrorHandler(err.handler)"<<"\n";
    strOut<<"    gdal.UseExceptions()  # Exceptions will get raised on anything >= gdal.CE_Failure"<<"\n";
    strOut<<"    assert err.err_level == gdal.CE_None, 'the error level starts at 0'"<<"\n";
    strOut<<"    main()"<<"\n";
    strOut<<""<<"\n";
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
        QString qgisPythonGdalPolygonizeFileName=strDir+PAFYCTOOLSGUI_QGIS_PYTHON_GDAL_POLYGONIZE;
        if(!QFile::exists(qgisPythonGdalPolygonizeFileName))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Not exists QGIS python GDAL polygonize file:\n%1").arg(qgisPythonGdalPolygonizeFileName);
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
    for(int npf=0;npf<mFilesToRemove.size();npf++)
    {
        QFile::remove(mFilesToRemove[npf]);
    }
    mFilesToRemove.clear();
    for(int npf=0;npf<mFoldersToRemove.size();npf++)
    {
        QString folderToRemove=mFoldersToRemove[npf];
        removeDir(folderToRemove);
    }
    mFoldersToRemove.clear();
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
        msgBox.setInformativeText("Exists files in output path.\nDo you want to change the output path?");
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
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_PLPPC_PP_PWOL,Qt::CaseInsensitive)==0)
    {
        if(!process_plppc_pp_pwol(qgisPath,outputPath,strAuxError))
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
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_BCVRM,Qt::CaseInsensitive)==0)
    {
        if(!process_bcvrm(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_CMNDVI,Qt::CaseInsensitive)==0)
    {
        if(!process_cmndvi(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_CMGCCVOL,Qt::CaseInsensitive)==0)
    {
        if(!process_cmgccvol(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_CWSITHO,Qt::CaseInsensitive)==0)
    {
        if(!process_cwsitho(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_SZBR,Qt::CaseInsensitive)==0)
    {
        if(!process_szbr(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(command.compare(PAFYCTOOLSGUI_COMMAND_MFHA,Qt::CaseInsensitive)==0)
    {
        if(!process_mfha(qgisPath,outputPath,strAuxError))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("Processing commad:\n%1\nerror:\n%2")
                    .arg(command).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
}

void PAFyCToolsDialog::copyShapefile(QString &fileName,
                                     QString &newFileName)
{
    if(!QFile::exists(fileName))
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("\nNot exists shapefile:\n%1").arg(fileName);
        QMessageBox::information(this,title,msg);
        return;
    }
    QFileInfo fileInfo(fileName);
    QString suffix=fileInfo.suffix();
    if(suffix.compare("shp",Qt::CaseInsensitive)!=0)
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("\nFile witout shp extension: %1").arg(fileName);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString baseNameWithOutExtension=fileInfo.completeBaseName();
    QString path=fileInfo.absolutePath();
    QDir auxDir(QDir::currentPath());
    if(!auxDir.exists(path))
    {
        QString title=PAFYCTOOLSGUI_TITLE;
        QString msg=QObject::tr("\nNot exists path:\n%1").arg(path);
        QMessageBox::information(this,title,msg);
        return;
    }
    QFileInfo newFileInfo(newFileName);
    QString newCompleteBaseName=newFileInfo.completeBaseName();
    QString newPath=newFileInfo.absolutePath();
    QStringList fileExtensions;
    fileExtensions<<"shp";
    fileExtensions<<"cpg";
    fileExtensions<<"mshp";
    fileExtensions<<"shx";
    fileExtensions<<"dbf";
    fileExtensions<<"sbn";
    fileExtensions<<"sbx";
    fileExtensions<<"fbn";
    fileExtensions<<"fbx";
    fileExtensions<<"ain";
    fileExtensions<<"aih";
    fileExtensions<<"prj";
    fileExtensions<<"shp.xml";
    fileExtensions<<"html";
    fileExtensions<<"lbl";
    fileExtensions<<"qpj";
    for(int i=0;i<fileExtensions.size();i++)
    {
        QString suffix=fileExtensions.at(i);
        QString fileNameToCopy=path+"/"+baseNameWithOutExtension+"."+suffix;
        QString newFileName=newPath+"/"+newCompleteBaseName+"."+suffix;
        if(!QFile::copy(fileNameToCopy,newFileName))
        {
            QString title=PAFYCTOOLSGUI_TITLE;
            QString msg=QObject::tr("\nError copying file: %1\nto file:\n%2")
                    .arg(fileNameToCopy).arg(newFileName);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    return;
}

