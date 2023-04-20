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
    bool initialize(QString& strError);

private:
    Ui::PAFyCToolsDialog *ui;
    QSettings *mPtrSettings;
    QString mBasePath;
    QString mLastPath;
    QVector<QString> mCommands;
    QMap<QString,QVector<QString> > mSubCommandsByCommand;
    ParametersManager* mPtrParametersManager;
    ProcessTools::ProgressExternalProcessDialog* mPtrProgressExternalProcessDialog;
    QString mStrExecution;
    QDateTime mInitialDateTime;
    QString mProgressExternalProcessTitle;
    QString mTempPath;
    QString mOutputPath;
    QVector<QString> mResultFiles;
};

#endif // PAFYCTOOLSDIALOG_H
