#include "mcusettingwindow.h"
#include "ui_mcusettingwindow.h"
#include <QDebug>

McuSettingWindow::McuSettingWindow(XpgTool *xpgTool ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::McuSettingWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Tool);
    this->xpgTool = xpgTool;
}

McuSettingWindow::~McuSettingWindow()
{
    delete ui;
}

void McuSettingWindow::initData(){
    ui->lineEdit_softver->setText(xpgTool->ReadIni("setting/softver"));
    ui->lineEdit_hardver->setText(xpgTool->ReadIni("setting/hardver"));
    ui->spinBox_upload->setValue(xpgTool->ReadIni("setting/uploadtime").toInt());
    ui->spinBox_ota->setValue(xpgTool->ReadIni("setting/otapiece").toInt());
    ui->spinBox_binding->setValue(xpgTool->ReadIni("setting/bordingtime").toInt());
    ui->comboBox->setCurrentText(xpgTool->ReadIni("setting/otatype"));
    ui->comboBox_moduletype->setCurrentText(xpgTool->ReadIni("setting/moduletype"));
    ui->lineEdit_enviroment->setText(xpgTool->ReadIni("environment/environment"));
}

void McuSettingWindow::on_pushButton_save_clicked()
{
    xpgTool->WriteIni("setting/softver", ui->lineEdit_softver->text());
    xpgTool->WriteIni("setting/hardver", ui->lineEdit_hardver->text());
    xpgTool->WriteIni("setting/uploadtime", ui->spinBox_upload->text());
    xpgTool->WriteIni("setting/otapiece", ui->spinBox_ota->text());
    xpgTool->WriteIni("setting/bordingtime", ui->spinBox_binding->text());
    xpgTool->WriteIni("setting/otatype", ui->comboBox->currentText());
    xpgTool->WriteIni("setting/moduletype", ui->comboBox_moduletype->currentText());
    xpgTool->WriteIni("environment/environment", ui->lineEdit_enviroment->text());
    this->hide();
}
