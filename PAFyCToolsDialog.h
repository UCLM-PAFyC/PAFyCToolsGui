#ifndef PAFYCTOOLSDIALOG_H
#define PAFYCTOOLSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QDateTime>

namespace Ui {
class PAFyCToolsDialog;
}

class ParametersManager;

namespace ProcessTools{
class ProgressExternalProcessDialog;
}

class PAFyCToolsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PAFyCToolsDialog(QWidget *parent = nullptr);
    ~PAFyCToolsDialog();

private slots:
    void on_qgisPathPushButton_clicked();

    void on_outputPathPushButton_clicked();

    void on_commandComboBox_currentIndexChanged(int index);

    void on_helpPushButton_clicked();

    void on_parametersPushButton_clicked();

    void on_ProgressExternalProcessDialog_closed();

    void on_processPushButton_clicked();

private:
    void copyShapefile(QString &fileName,
                       QString& newFileName);
    bool initialize(QString& strError);
    bool process_ccfpgp(QString& qgisPath,
                          QString& outputPath,
                          QString& strError);
    bool process_plppc_pp(QString& qgisPath,
                          QString& outputPath,
                          QString& strError);
    bool process_plppc_pp_pwol(QString& qgisPath,
                               QString& outputPath,
                               QString& strError);
    bool process_plppc_pl(QString& qgisPath,
                          QString& outputPath,
                          QString& strError);
    bool process_plppc_pf(QString& qgisPath,
                          QString& outputPath,
                          QString& strError);
    bool process_bcvrm(QString& qgisPath,
                       QString& outputPath,
                       QString& strError);
    bool process_cmgccvol(QString& qgisPath,
                          QString& outputPath,
                          QString& strError);
    bool process_cmndvi(QString& qgisPath,
                        QString& outputPath,
                        QString& strError);
    bool process_cwsitho(QString& qgisPath,
                         QString& outputPath,
                         QString& strError);
    bool process_szbr(QString& qgisPath,
                         QString& outputPath,
                         QString& strError);
    bool process_mfha(QString& qgisPath,
                         QString& outputPath,
                         QString& strError);
    bool removeDir(QString dirName,
                   bool onlyContent=false);

private:
    bool writePythonProgramCropsCharacterizationFromPhotogrammetricGeomaticProducts(QString pythonFileName,
                                                                                    QString &strError);
    bool writePythonProgramCropMonitoringFromPhotogrammetricGeomaticProducts(QString pythonFileName,
                                                                             QString& strError);
    bool writePythonProgramCropWaterStressIndexUsingThermalOrthomosaic(QString pythonFileName,
                                                                       QString& strError);
    bool writePythonProgramSoilZoningBasedInReflectivity(QString pythonFileName,
                                                         QString &strError);
    bool writePythonProgramMonitoringFloraAtHighAltitude(QString pythonFileName,
                                                         QString &strError);
    Ui::PAFyCToolsDialog *ui;
    QSettings *mPtrSettings;
    QString mBasePath;
    QString mLastPath;
    QVector<QString> mCommands;
    QMap<QString,QVector<QString> > mSubCommandsByCommand;
    ParametersManager* mPtrParametersManager;
    ParametersManager* mPtrParametersManagerModelManagementCommands;
    ProcessTools::ProgressExternalProcessDialog* mPtrProgressExternalProcessDialog;
    QString mStrExecution;
    QDateTime mInitialDateTime;
    QString mProgressExternalProcessTitle;
    QString mTempPath;
    QString mOutputPath;
    QVector<QString> mResultFiles;
    QMap<QString,QVector<QString> > mModelManagementCommandsByCommand;
    QVector<QString> mPythonFiles;
    QVector<QString> mFilesToRemove;
    QVector<QString> mFoldersToRemove;
};

#endif // PAFYCTOOLSDIALOG_H
