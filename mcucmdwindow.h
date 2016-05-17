#ifndef MCUCMDWINDOW_H
#define MCUCMDWINDOW_H

#include <QWidget>

namespace Ui {
class McuCmdWindow;
}

class McuCmdWindow : public QWidget
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
    explicit McuCmdWindow(QWidget *parent = 0);
    ~McuCmdWindow();

private:
    Ui::McuCmdWindow *ui;

signals:
    void SENDCMD(CMD);
private slots:
    void on_pushButton_airlink_clicked();
    void on_pushButton_softap_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_gettime_clicked();
    void on_pushButton_onbording_clicked();
    void on_pushButton_startota_clicked();
    void on_pushButton_test_clicked();
    void on_pushButton_upload_clicked();
    void on_pushButton_restart_clicked();
    void on_pushButton_stopupload_clicked();
};

#endif // MCUCMDWINDOW_H
