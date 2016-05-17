#include "xpgtool.h"
#include <QDebug>
XpgTool::XpgTool()
{
    InitData();
}

XpgTool::~XpgTool()
{

}

void XpgTool::InitData(){
    QString ini_url = qApp->applicationDirPath()+ "/auto_analyze.ini";
    setting = new QSettings(ini_url,QSettings::IniFormat);
    setting->setIniCodec("UTF-8");
}

void XpgTool::WriteIni(QString local, QString value){
    setting->setValue(local, value);
}

void XpgTool::WriteIni(QString local, QByteArray value){
    value = InsertSpace(value);
    QString Data = value.data();
    setting->setValue(local,Data);
}

QString XpgTool::ReadIni(QString local){
    QString Data = setting->value(local).toString();
    return Data;
}

unsigned int XpgTool::GetByte(QByteArray Data, int position){
    QByteArray temp = Data.mid(position, 1);
    unsigned int dataLen = temp.toHex().toInt(NULL, 16);
    return dataLen;
}

unsigned int XpgTool::GetWord(QByteArray Data, int position){
    QByteArray temp = Data.mid(position, 2);
    unsigned int dataLen = temp.toHex().toUInt(NULL, 16);
    return dataLen;
}

unsigned char XpgTool::CheckSum(QByteArray Data, int position, int end){
    unsigned char checksum = 0;
    for(int i=position; i<Data.size()-end; i++){
        checksum += Data.at(i);
    }
    return checksum;
}

QByteArray XpgTool::InsertSpace(QByteArray Data){
    Data = Data.toHex().toUpper();
    for(int i=Data.length(); i>0; i-=2){
        Data.insert(i," ");
    }
    return Data;
}

QByteArray XpgTool::Insert55(QByteArray Data){
    for(int i=Data.length(); i>2; i--){
        if((unsigned char)Data[i] == 0xff){
            Data.insert(i+1, 0x55);
        }
    }
    return Data;
}

QByteArray  XpgTool::IntToByte(int Number, int NumSize)
{
    QByteArray abyte0;
    switch(NumSize){
    case 1:
        abyte0.resize(1);
        abyte0[0] = (uchar)  (0xff & Number);
        break;
    case 2:
        abyte0.resize(2);
        abyte0[1] = (uchar)  (0x00ff & Number);
        abyte0[0] = (uchar) ((0xff00 & Number) >> 8);
        break;
    case 3:
        abyte0.resize(3);
        abyte0[3] = (uchar)  (0x0000ff & Number);
        abyte0[2] = (uchar) ((0x00ff00 & Number) >> 8);
        abyte0[1] = (uchar) ((0xff0000 & Number) >> 16);
        break;
    case 4:
        abyte0.resize(4);
        abyte0[3] = (uchar)  (0x000000ff & Number);
        abyte0[2] = (uchar) ((0x0000ff00 & Number) >> 8);
        abyte0[1] = (uchar) ((0x00ff0000 & Number) >> 16);
        abyte0[0] = (uchar) ((0xff000000 & Number) >> 24);
        break;
    }
    return abyte0;
}
