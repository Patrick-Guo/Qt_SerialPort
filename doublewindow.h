#ifndef DOUBLEWINDOW_H
#define DOUBLEWINDOW_H

#include <QWidget>
#include <QTimer>
#include <xpgserialport.h>

namespace Ui {
class DoubleWindow;
}

class DoubleWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DoubleWindow(QWidget *parent = 0);
    ~DoubleWindow();
    void InitUi();
    void InitData();
    void ResetWindow();
    void CleanRec();
    void SendData(QString COM, QString s);

private slots:
    void UpdateUi(QList<QString> ComList);
    void on_pushButton_open1_clicked();
    void on_pushButton_open2_clicked();
    void com1OnDataReady(QByteArray Data);
    void com2OnDataReady(QByteArray Data);
    void readbuf1();
    void readbuf2();
    void on_pushButton_double_clicked();
    void on_pushButton_left1_clicked();
    void on_pushButton_left2_clicked();
    void on_pushButton_left3_clicked();
    void on_pushButton_left4_clicked();
    void on_pushButton_right1_clicked();
    void on_pushButton_right2_clicked();
    void on_pushButton_right3_clicked();
    void on_pushButton_right4_clicked();
    void on_pushButton_v3_clicked();
    void on_pushButton_v4_clicked();
    void on_pushButton_send1_clicked();
    void on_pushButton_send2_clicked();

private:
    Ui::DoubleWindow *ui;
    XpgSerialPort *com1, *com2;
    QByteArray com1Buf, com2Buf;
    QTimer com1Timer, com2Timer;
    bool openFlag;
};

#endif // DOUBLEWINDOW_H
