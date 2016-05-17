#include <QDebug>
#include <QTime>
#include <QDir>
#include <QMessageBox>
#include "doublewindow.h"
#include "ui_doublewindow.h"

DoubleWindow::DoubleWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DoubleWindow)
{
    ui->setupUi(this);
    InitUi();
    InitData();
}

DoubleWindow::~DoubleWindow()
{
    delete ui;
}


void DoubleWindow::InitUi(){
    QString background_url = qApp->applicationDirPath()+ "/image/background.png";
    ui->label_background->setStyleSheet("border-image:url(" + background_url + ")");
    ui->textEdit_rec1->setReadOnly(true);
    ui->textEdit_rec2->setReadOnly(true);
}

void DoubleWindow::InitData(){
    openFlag = true;
    com1 = new XpgSerialPort();
    com2 = new XpgSerialPort();
    connect(com1, SIGNAL(ComListChange(QList<QString>)), this, SLOT(UpdateUi(QList<QString>)));
    com1Buf.clear();
    com2Buf.clear();
    com1Timer.setInterval(500);
    com2Timer.setInterval(500);
    connect(&com1Timer, SIGNAL(timeout()), this, SLOT(readbuf1()));
    connect(&com2Timer, SIGNAL(timeout()), this, SLOT(readbuf2()));
    com1Timer.start();
    com2Timer.start();
    connect(com1, SIGNAL(DataReady(QByteArray)), this, SLOT(com1OnDataReady(QByteArray)));
    connect(com2, SIGNAL(DataReady(QByteArray)), this, SLOT(com2OnDataReady(QByteArray)));
}

void DoubleWindow::UpdateUi(QList<QString> ComList){
    QString tmp_com1, tmp_com2;
    ui->com_1->clear();
    ui->com_2->clear();
    int i = 0;
    foreach(QString tmp, ComList){
        ui->com_1->insertItem(i, tmp);
        ui->com_2->insertItem(i, tmp);
        if(tmp_com1 == tmp)
            ui->com_1->setCurrentText(tmp);
        if(tmp_com2 == tmp)
            ui->com_2->setCurrentText(tmp);
        i++;
    }
}

void DoubleWindow::SendData(QString COM, QString s){
    QByteArray Data = QByteArray::fromHex(s.toLatin1());
    if(COM == "COM1" && com1->isOpen()){
        com1->write(Data);
    }else if(COM == "COM2" && com2->isOpen()){
        com2->write(Data);
    }
}

void DoubleWindow::CleanRec(){
    ui->textEdit_rec1->clear();
    ui->textEdit_rec2->clear();
}

void DoubleWindow::ResetWindow(){
    if(com1->isOpen())
        com1->CloseCom();
    if(com2->isOpen())
        com2->CloseCom();
    ui->pushButton_open1->setText("打开串口");
    ui->pushButton_open2->setText("打开串口");
}

//---------槽函数---------//
void DoubleWindow::on_pushButton_open1_clicked(){
    if(!com1->isOpen()){
        QString COM = ui->com_1->currentText();
        QString RATE = ui->rate_1->currentText();
        int DATA = ui->data_1->currentIndex();
        int STOP = ui->stop_1->currentIndex();
        int CHECK = ui->check_1->currentIndex();
        bool IsOpen = com1->OpenCom(COM, RATE, DATA, STOP, CHECK);
        if(IsOpen){
            ui->pushButton_open1->setText("关闭串口");
        }else{
            QMessageBox::about(this, "错误", "打开失败！");
        }
    }else{
        com1->CloseCom();
        ui->pushButton_open1->setText("打开串口");
    }
}

void DoubleWindow::on_pushButton_open2_clicked(){
    if(!com2->isOpen()){
        QString COM = ui->com_2->currentText();
        QString RATE = ui->rate_2->currentText();
        int DATA = ui->data_2->currentIndex();
        int STOP = ui->stop_2->currentIndex();
        int CHECK = ui->check_2->currentIndex();
        bool IsOpen = com2->OpenCom(COM, RATE, DATA, STOP, CHECK);
        if(IsOpen){
            ui->pushButton_open2->setText("关闭串口");

        }else{
            QMessageBox::about(this, "错误", "打开失败！");
        }
    }else{
        com2->CloseCom();
        ui->pushButton_open2->setText("打开串口");
    }
}

//----
void DoubleWindow::com1OnDataReady(QByteArray Data){
    if(com2->isOpen() && openFlag){
        com2->write(Data);
    }
    com1Buf.append(Data);
    while(com1Buf.length() >= 4){
        //if(com1Buf.at(0)==0x56 && com1Buf.at(1)==0x00){
        if(com1Buf.at(0)==0x00 && com1Buf.at(1)==0x00 && com1Buf.at(2)==0x00 && com1Buf.at(3)==0x03 && ui->checkBox->isChecked()){
            ui->textEdit_rec1->moveCursor(QTextCursor::End);
            ui->textEdit_rec1->append("");
            QTime mytime;
            ui->textEdit_rec1->insertPlainText(mytime.currentTime().toString("hh:mm:ss.zzz")+":");
        //}else if(com1Buf.at(0)==0x76 && com1Buf.at(1)==0x00){
        }else if(com1Buf.at(0)==-1 && com1Buf.at(1)==-1){
            ui->textEdit_rec1->moveCursor(QTextCursor::End);
            ui->textEdit_rec1->append("");
            QTime mytime;
            ui->textEdit_rec1->insertPlainText(mytime.currentTime().toString("hh:mm:ss.zzz")+":");
        }
        char temp[3];
        sprintf(temp, "%02X", (unsigned char) com1Buf.at(0));
        ui->textEdit_rec1->moveCursor(QTextCursor::End);
        ui->textEdit_rec1->insertPlainText(temp);
        ui->textEdit_rec1->insertPlainText(" ");
        com1Buf.remove(0,1);
    }
}

void DoubleWindow::readbuf1(){
    if(0<com1Buf.length() && com1Buf.length()<4)
    {

        while(com1Buf.length()){
            char temp[3] = "";
            sprintf(temp, "%02X", (unsigned char) com1Buf.at(0));
            ui->textEdit_rec1->moveCursor(QTextCursor::End);
            ui->textEdit_rec1->insertPlainText(temp);
            ui->textEdit_rec1->insertPlainText(" ");
            com1Buf.remove(0,1);
        }
        ui->textEdit_rec1->moveCursor(QTextCursor::End);
    }
}

void DoubleWindow::com2OnDataReady(QByteArray Data){
    if(com1->isOpen() && openFlag){
        com1->write(Data);
    }
    com2Buf.append(Data);
    while(com2Buf.length() >= 4){
        if(com2Buf.at(0)==0x00 && com2Buf.at(1)==0x00 && com2Buf.at(2)==0x00 && com2Buf.at(3)==0x03 && ui->checkBox->isChecked()){
        //if(com2Buf.at(0)==0x56 && com2Buf.at(1)==0x00){
            ui->textEdit_rec2->moveCursor(QTextCursor::End);
            ui->textEdit_rec2->append("");
            QTime mytime;
            ui->textEdit_rec2->insertPlainText(mytime.currentTime().toString("hh:mm:ss.zzz")+":");
        }else if(com2Buf.at(0)==-1 && com2Buf.at(1)==-1){
        //}else if(com2Buf.at(0)==0x76 && com2Buf.at(1)==0x00){
            ui->textEdit_rec2->moveCursor(QTextCursor::End);
            ui->textEdit_rec2->append("");
            QTime mytime;
            ui->textEdit_rec2->insertPlainText(mytime.currentTime().toString("hh:mm:ss.zzz")+":");
        }
        char temp[3];
        sprintf(temp, "%02X", (unsigned char) com2Buf.at(0));
        ui->textEdit_rec2->moveCursor(QTextCursor::End);
        ui->textEdit_rec2->insertPlainText(temp);
        ui->textEdit_rec2->insertPlainText(" ");
        com2Buf.remove(0,1);
    }
}

void DoubleWindow::readbuf2(){
    if(0<com2Buf.length() && com2Buf.length()<4)
    {
        while(com2Buf.length()){
            char temp[3] = "";
            sprintf(temp, "%02X", (unsigned char) com2Buf.at(0));
            ui->textEdit_rec2->moveCursor(QTextCursor::End);
            ui->textEdit_rec2->insertPlainText(temp);
            ui->textEdit_rec2->insertPlainText(" ");
            com2Buf.remove(0,1);
        }
        ui->textEdit_rec2->moveCursor(QTextCursor::End);
    }
}

//----
void DoubleWindow::on_pushButton_double_clicked()
{
    if(openFlag){
        openFlag = false;
        ui->pushButton_double->setText("打开双串口");
    }else{
        openFlag = true;
        ui->pushButton_double->setText("关闭双串口");
    }
}
//----
void DoubleWindow::on_pushButton_left1_clicked()
{
    QString s = ui->lineEdit_default1->text();
    SendData("COM1", s);
}

void DoubleWindow::on_pushButton_left2_clicked()
{
    QString s = ui->lineEdit_default2->text();
    SendData("COM1", s);
}

void DoubleWindow::on_pushButton_left3_clicked()
{
    QString s = ui->lineEdit_default3->text();
    SendData("COM1", s);
}

void DoubleWindow::on_pushButton_left4_clicked()
{
    QString s = ui->lineEdit_default4->text();
    SendData("COM1", s);
}

void DoubleWindow::on_pushButton_right1_clicked()
{
    QString s = ui->lineEdit_default1->text();
    SendData("COM2", s);
}

void DoubleWindow::on_pushButton_right2_clicked()
{
    QString s = ui->lineEdit_default2->text();
    SendData("COM2", s);
}

void DoubleWindow::on_pushButton_right3_clicked()
{
    QString s = ui->lineEdit_default3->text();
    SendData("COM2", s);
}

void DoubleWindow::on_pushButton_right4_clicked()
{
    QString s = ui->lineEdit_default4->text();
    SendData("COM2", s);
}

void DoubleWindow::on_pushButton_v3_clicked()
{
    ui->lineEdit_default1->setText("00 00 00 03 00 02 03 0b");
    ui->lineEdit_default2->setText("00 00 00 03 00 02 03 08");
    ui->lineEdit_default3->setText("00 00 00 03 00 02 03 06");
    ui->lineEdit_default4->setText("00 00 00 03 00 28 03 07 00 20 39 38 32 61 36 38 65 33 61 30 39 61 34 66 62 30 39 34 62 37 65 31 30 34 35 35 63 66 61 64 36 38 00 02 00 01");
}

void DoubleWindow::on_pushButton_v4_clicked()
{
    ui->lineEdit_default1->setText("FF FF 00 05 0B 00 00 00 10");
    ui->lineEdit_default2->setText("FF FF 00 06 09 00 00 00 02 11");
    ui->lineEdit_default3->setText("FF FF 00 05 01 00 00 00 06");
    ui->lineEdit_default4->setText("FF FF 00 47 02 01 00 00 30 30 30 30 30 30 30 34 30 30 30 30 30 30 30 34 30 30 30 30 30 30 30 31 30 30 30 30 30 30 30 31 39 38 32 61 36 38 65 33 61 30 39 61 34 66 62 30 39 34 62 37 65 31 30 34 35 35 63 66 61 64 36 38 00 00 1B");
}

void DoubleWindow::on_pushButton_send1_clicked()
{
    QString s = ui->textEdit_send1->toPlainText();
    SendData("COM1", s);
}

void DoubleWindow::on_pushButton_send2_clicked()
{
    QString s = ui->textEdit_send1->toPlainText();
    SendData("COM2", s);
}
