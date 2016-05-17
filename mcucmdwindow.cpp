#include "mcucmdwindow.h"
#include "ui_mcucmdwindow.h"

McuCmdWindow::McuCmdWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::McuCmdWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
}

McuCmdWindow::~McuCmdWindow()
{
    delete ui;
}

void McuCmdWindow::on_pushButton_airlink_clicked()
{
    emit SENDCMD(AirLink);
}

void McuCmdWindow::on_pushButton_softap_clicked()
{
    emit SENDCMD(SoftAp);
}

void McuCmdWindow::on_pushButton_reset_clicked()
{
    emit SENDCMD(Reset);
}

void McuCmdWindow::on_pushButton_gettime_clicked()
{
    emit SENDCMD(GetTime);
}

void McuCmdWindow::on_pushButton_onbording_clicked()
{
    emit SENDCMD(OnBording);
}

void McuCmdWindow::on_pushButton_startota_clicked()
{
    emit SENDCMD(StartOTA);
}

void McuCmdWindow::on_pushButton_test_clicked()
{
    emit SENDCMD(Test);
}

void McuCmdWindow::on_pushButton_upload_clicked()
{
    emit SENDCMD(Upload);
}

void McuCmdWindow::on_pushButton_restart_clicked()
{
    emit SENDCMD(Restart);
}

void McuCmdWindow::on_pushButton_stopupload_clicked()
{
    emit SENDCMD(StopUpload);
}
