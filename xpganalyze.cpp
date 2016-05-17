#include "xpganalyze.h"

XpgAnalyze::XpgAnalyze(XpgFramer *xpgFramer,XpgTool *xpgTool ,QObject *parent) : QObject(parent)
{
    if(xpgFramer == NULL){
        qDebug()<<"error XpgFramer";
        return;
    }
    this->xpgFramer = xpgFramer;
    this->xpgTool = xpgTool;
    InitData();
}

XpgAnalyze::~XpgAnalyze()
{

}

void XpgAnalyze::InitData(){
//    QString ini_url = qApp->applicationDirPath()+ "/auto_analyze.ini";
//    setting = new QSettings(ini_url,QSettings::IniFormat);
//    setting->setIniCodec("UTF-8");
    connect(xpgFramer, SIGNAL(LineDataReady(QByteArray)), this, SLOT(FindRespond(QByteArray)));

    binFile = new QFile();
    hexFile = new QFile();
    appLocal = QCoreApplication::applicationDirPath();
    binFile->setFileName(appLocal+"/bin/mcu.bin");
    hexFile->setFileName(appLocal+"/bin/mcu.hex");
    package_sn = 0;
}

void XpgAnalyze::FindRespond(QByteArray Data){
    unsigned char cmd = Data.at(4);
    QString tmp = "Respond/";
    switch((int)cmd){
    case 1:
        tmp += "02";
        break;
    case 3:
        tmp += "04";
        break;
    case 7:
        tmp += "08";
        break;
    case 13:
        tmp += "0E";
        break;
    case 15:
        tmp += "10";
        break;
    case 17:
        tmp += "12";
        break;
    case 19:
        tmp += "14";
        break;
    case 21:
        tmp += "16";
        break;
    case 25:
        MD5Len = Data.mid(12,2);
        MD5 = Data.mid(14, MD5Len.toHex().toInt(NULL, 16));
        binFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
        binFile->close();
        hexFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
        hexFile->close();
        tmp += "1A";
        break;
    case 27:
        //TODO：待模组完善后添加获取分片大小功能
        tmp += "1C";
        break;
    case 29:{
        QByteArray MCUData;
        MCUData = Data.mid(12,Data.length()-13);
        unsigned int tmp_sn = xpgTool->GetWord(Data, 8);
        QString type = xpgTool->ReadIni("setting/otatype");
        if(package_sn != tmp_sn){
            package_sn = tmp_sn;
            if(type == "hex"){
                if(hexFile->open(QIODevice::WriteOnly | QIODevice::Append)){
                    hexFile->write(MCUData);
                    hexFile->close();
                }else{
                    qDebug()<<"open failed";
                }
            }else{
                if(binFile->open(QIODevice::WriteOnly | QIODevice::Append)){
                    binFile->write(MCUData);
                    binFile->close();
                }else{
                    qDebug()<<"open failed";
                }
            }
        }
        tmp += "1E";
        break;}
    case 30:
        emit SendUploadPackage(false);
        return;
        break;
    case 31:
        tmp += "20";
        break;
    case 39:
        emit StopUpload();
        tmp += "28";
        break;
    default:
        return;
    }
    tmp = xpgTool->ReadIni(tmp);
    QByteArray Respond = QByteArray::fromHex(tmp.toLatin1());
    Respond[5] = Data.at(5);
    if((int)cmd==1){
        int bordingTime = xpgTool->ReadIni("setting/bordingtime").toInt();
        QString moduleType = xpgTool->ReadIni("setting/moduletype");
        QByteArray a = xpgTool->IntToByte(bordingTime, 2);
        Respond.replace(72, 2, a);

        QString hardver = xpgTool->ReadIni("setting/hardver");
        QString softver = xpgTool->ReadIni("setting/softver");
        QByteArray A_hardver = hardver.toLatin1();
        QByteArray A_softver = softver.toLatin1();
        Respond.replace(24, 8, A_hardver);
        Respond.replace(32, 8, A_softver);

        if(moduleType == "central"){
            Respond.replace(2, 2, QByteArray::fromHex(QString("0x4F").toLatin1()));
            QString s = "0000000000000001";
            Respond.insert(74, QByteArray::fromHex(s.toLatin1()));
        }
        qDebug()<<Respond.toHex();


    }else if((int)cmd==3 && Data.at(8)==0x02){
        Respond[8] = 0x03;
        emit RespondReady(Respond);
        return;
    }
    unsigned char checksum = xpgTool->CheckSum(Respond, 2, 1);
    checksum &= 0x000000FF;
    Respond[Respond.length()-1] = checksum;
    emit RespondReady(Respond);
}

//QString XpgAnalyze::GetIniData(QString local){
//    QString Data = setting->value(local).toString();
//    return Data;
//}

//void XpgAnalyze::WriteIni(QString local, QByteArray value){
//    value = xpgTool->InsertSpace(value);
//    QString Data = value.data();
//    setting->setValue(local,Data);
//}

bool XpgAnalyze::SaveProductKey(QString key){
    QString key_32;
    QByteArray key_64;
    key.replace(" ","");
    if(key.length() == 32){
        key_32 = key;
        key_64 = key_32.toLatin1();
    }else if(key.length() == 64){
        key_32 = QByteArray::fromHex(key.toLatin1());
        key_64 = QByteArray::fromHex(key.toLatin1());
    }else{
        return false;
    }
    xpgTool->WriteIni("product_key/key",key_32);
    key = xpgTool->ReadIni("Respond/02");
    QByteArray tmp = QByteArray::fromHex(key.toLatin1());
    tmp.replace(40,32,key_64);
    xpgTool->WriteIni("Respond/02", tmp);
    return true;
}

//bool XpgAnalyze::changEnvironment(QString str){
//    if(str == "生产环境" || str == "测试环境" || str == "GDMS环境" || str == "开发环境" || str == "Stage环境" || str == "国外环境"){
//        xpgTool->WriteIni("environment/environment", str);
//        return true;
//    }else{
//        return false;
//    }
//}

data_analyze XpgAnalyze::Analyze(QByteArray Data){
    data_analyze AnalyzeData;
    AnalyzeData.length.sprintf("%02x", xpgTool->GetWord(Data, 2));
    AnalyzeData.cmd.sprintf("%02x",(int)Data.at(4));
    AnalyzeData.sn.sprintf("%02x", Data.at(5));
    AnalyzeData.checksum.sprintf("%02x",xpgTool->CheckSum(Data, 2, 1));
    unsigned char cmd = Data.at(4);
    QString tmp = "cmd/";
    switch((int)cmd){
    case 1:
        tmp += "01";
        break;
    case 2:
        tmp+= "02";
        AnalyzeData.product_key = xpgTool->ReadIni("product_key/key");
        break;
    case 3:
        tmp += "03";
        if(Data.at(8) == 0x01)
            tmp += "01";
        else if(Data.at(8) == 0x02)
            tmp += "02";
        break;
    case 4:
        tmp += "04";
        if(xpgTool->GetWord(Data, 2) == 5)
            tmp += "04";
        else
            tmp += "03";
        break;
    case 5:
        tmp += "05";
        break;
    case 6:
        tmp += "06";
        break;
    case 7:
        tmp += "07";
        break;
    case 8:
        tmp += "08";
        break;
    case 9:
        tmp += "09";
        if(Data.at(8) == 0x01)
            AnalyzeData.setmode = xpgTool->ReadIni("setmode/01");
        else if(Data.at(8) == 0x02)
            AnalyzeData.setmode = xpgTool->ReadIni("setmode/02");
        break;
    case 10:
        tmp += "0A";
        break;
    case 11:
        tmp += "0B";
        break;
    case 12:
        tmp += "0C";
        break;
    case 13:{
        tmp += "0D";
        QString connect, state, hasconnect;
        state.sprintf("wifistate/%02X", (int)(Data.at(9)&0x0f));
        connect.sprintf("connect/%02X", (int)Data.at(9)>>4);
        AnalyzeData.wifistate = xpgTool->ReadIni(state);
        AnalyzeData.connect = xpgTool->ReadIni(connect);
        AnalyzeData.signal.sprintf("%02X", (int)(Data.at(8)&0x07));
        hasconnect.sprintf("hasconnect/%02X",(int)((Data.at(8)&0x08)>>3));
        AnalyzeData.hasconnect = xpgTool->ReadIni(hasconnect);
        break;
    }
    case 14:
        tmp += "0E";
        break;
    case 15:
        tmp += "0F";
        break;
    case 16:
        tmp += "10";
        break;
    case 17:
        tmp += "11";
        break;
    case 18:
        tmp += "12";
        break;
    case 19:
        tmp += "13";
        break;
    case 20:
        tmp += "14";
        break;
    case 21:
        tmp += "15";
        break;
    case 22:
        tmp += "16";
        break;
    case 23:
        tmp += "17";
        break;
    case 24:{
        tmp += "18";
        unsigned int Year = xpgTool->GetWord(Data, 8);
        unsigned int Month = xpgTool->GetByte(Data, 10);
        unsigned int Day = xpgTool->GetByte(Data, 11);
        unsigned int Hour = xpgTool->GetByte(Data, 12);
        unsigned int Min = xpgTool->GetByte(Data, 13);
        unsigned int Sec = xpgTool->GetByte(Data, 14);
        AnalyzeData.time.sprintf("%d年%d月%d日%d时%d分%d秒",Year,Month,Day,Hour,Min,Sec);
        break;}
    case 25:
        tmp += "19";
        break;
    case 26:
        tmp += "1A";
        break;
    case 27:
        tmp += "1B";
        break;
    case 28:
        tmp += "1C";
        break;
    case 29:
        tmp += "1D";
        break;
    case 30:
        tmp += "1E";
        break;
    case 31:
        tmp += "1F";
        break;
    case 32:
        tmp += "20";
        break;
    }
    AnalyzeData.text = xpgTool->ReadIni(tmp);

    return AnalyzeData;
}
