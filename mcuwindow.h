#ifndef MCUWINDOW_H
#define MCUWINDOW_H

#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidgetItem>
#include <xpgserialport.h>
#include <xpganalyze.h>
#include <xpgframer.h>
#include <xpgtool.h>
#include <xpgjson.h>
#include <QTimer>
#include <mcucmdwindow.h>
#include <mcusettingwindow.h>
#include <QFileDialog>
#include <uploaddata.h>

#define SpinBox 0
#define ComboBox 1
#define LineEdit 2

namespace Ui {
class McuWindow;
}

class McuWindow : public QWidget
{
    Q_OBJECT

    enum CMD{
        AirLink,
        SoftAp,
        Reset,
        GetTime,
        OnBording,
        StartOTA,
        Test,
        Upload,
        Restart,
        StopUpload
    };

public:
    explicit McuWindow(QWidget *parent = 0);
    ~McuWindow();
    void InitUi();
    void InitData();
    void SendData(QString Data);
    void DataPointSetValue(QByteArray Data);
    void CleanRec();
    void ResetWindow();
    void CheckKey();
    void SendFirstUploadData();
    QByteArray GetUpLoadData();

private slots:
    void SendCMD(CMD);
    void UpdateUi(QList<QString> ComList);
    void on_pushButton_open3_clicked();
    void fromFramer(QByteArray Data);
    void fromRespond(QByteArray Data);
    void sendUploadPackage(bool IsFirstPackage);
    void on_pushButton_send3_clicked();
    void on_pushButton_productkey_clicked();
    void stopUpload();

    void on_listWidget_itemClicked(QListWidgetItem *item);
    void jsonState(bool state);
    void updateDataPointWindow(QList<json_struct> jsonList);
    void autoUpLoad();
    void on_pushButton_updatejson_clicked();
    void on_pushButton_upload_clicked();
    void on_pushButton_autoupload_clicked();
    void on_pushButton_showcmd_clicked();
    void on_pushButton_setting_clicked();

private:
    Ui::McuWindow *ui;
    XpgSerialPort *com1;
    XpgAnalyze *analyze;
    XpgFramer *xpgFramer;
    XpgJson *xpgJson;
    XpgTool *xpgTool;    
    QList<QByteArray> DataList;
    QTimer *upLoadTimer;
    UploadData *uploadData;
    bool openFlag;
    McuCmdWindow *mcuCmdWindow;
    McuSettingWindow *mcuSettingWindow;
    QByteArray upLoadData;
    bool upLoadFlag;
};

#endif // MCUWINDOW_H
