#include <QDebug>
#include "xpgframer.h"

XpgFramer::XpgFramer(XpgSerialPort * xpgSerialPort, XpgTool *xpgTool, QObject *parent) : QObject(parent)
{
    if(xpgSerialPort == NULL){
        qDebug()<<"error xpgSerialPort";
        return;
    }
    this->xpgSerialPort = xpgSerialPort;
    this->xpgTool = xpgTool;
    InitData();
    Flag = false;
    tmp = 0;
}

XpgFramer::~XpgFramer()
{

}

void XpgFramer::InitData(){
    LineData.clear();
    connect(xpgSerialPort, SIGNAL(DataReady(QByteArray)), this, SLOT(dataToV4(QByteArray)), Qt::DirectConnection);
}

void XpgFramer::dataToV4(QByteArray Data){
    while(Data.length()){
        LineData.append(Data.at(0));
        Data.remove(0,1);
        while((LineData.at(0) != -1 && LineData.at(1) != -1) && LineData.length()>0){
            LineData.remove(0,1);
        }
        //LineData.replace(QByteArray::fromHex("FF55"), QByteArray::fromHex("FF"));
        if(LineData.length()>4){
            int dataLen = xpgTool->GetWord(LineData,2);
            if((LineData.length() == (dataLen + 4))){
                int i = LineData.indexOf(QByteArray::fromHex("FF55"), tmp);
                if(i == tmp){
                    i = LineData.indexOf(QByteArray::fromHex("FF55"), tmp);
                }else{
                    tmp = i;
                }
                if(i > 0){
                    LineData.replace(i, 2, QByteArray::fromHex("FF"));
                    tmp+=1;
                    continue;
                }else{
                    emit(LineDataReady(LineData));
                    LineData.clear();
                    tmp = 0;
                }
            }
        }
    }
}
