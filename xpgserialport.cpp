#include <QDebug>
#include <stdint.h>
#include "xpgserialport.h"

XpgSerialPort::XpgSerialPort()
{
    InitData();
}

XpgSerialPort::~XpgSerialPort()
{

}

void XpgSerialPort::InitData(){
    ComList.clear();
    timerComList.setInterval(1000);
    connect(&timerComList, SIGNAL(timeout()), this, SLOT(getComList()));
    timerComList.start();
    Data.clear();
}

/**
 * @desc
 * 获取可用串口列表
 */
void XpgSerialPort::getComList(){
    QList<QString> tmp_ComList;
    foreach(const QSerialPortInfo &info, this->availablePorts()){
        tmp_ComList.append(info.portName());
    }
    //如果列表有变化则发送信号
    if(ComList.isEmpty() || ComList != tmp_ComList){
        ComList = tmp_ComList;
    }
    else{
        return;
    }
    emit ComListChange(ComList);
}

/**
*@desc
* 打开串口
*/
bool XpgSerialPort::OpenCom(QString COM, QString RATE, int DATA, int STOP, int CHECK){
    QSerialPortInfo tmp_info;
    foreach(const QSerialPortInfo &info, this->availablePorts()){
        if(COM == info.portName()){
            tmp_info = info;
            break;
        }
    }
    this->setPort(tmp_info);
    if(this->open(QIODevice::ReadWrite)){
        this->setBaudRate(RATE.toInt());
        switch(DATA){
        case 0:
            this->setDataBits(this->Data5);
            break;
        case 1:
            this->setDataBits(this->Data6);
            break;
        case 2:
            this->setDataBits(this->Data7);
            break;
        case 3:
            this->setDataBits(this->Data8);
            break;
        }
        switch(STOP){
        case 0:
            this->setStopBits(this->OneStop);
            break;
        case 1:
            this->setStopBits(this->OneAndHalfStop);
            break;
        case 2:
            this->setStopBits(this->TwoStop);
            break;
        }
        switch(CHECK){
        case 0:
            this->setParity(this->NoParity);
            break;
        case 1:
            this->setParity(this->OddParity);
            break;
        case 2:
            this->setParity(this->EvenParity);
            break;
        case 3:
            this->setParity(this->SpaceParity);
            break;
        }
        connect(this, SIGNAL(readyRead()), this, SLOT(readComData()));
        return true;
    }else{
        return false;
    }
}

void XpgSerialPort::readComData(){
    QByteArray tmp = this->readAll();
    emit DataReady(tmp);
}

void XpgSerialPort::CloseCom(){
    disconnect(this, SIGNAL(readyRead()), this, SLOT(readComData()));
    this->close();

}
