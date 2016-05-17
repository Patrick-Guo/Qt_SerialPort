#include "uploaddata.h"
#include <QDebug>

UploadData::UploadData(QWidget *parent) : QWidget(parent)
{
    xpgTool = new XpgTool();
    perPieceSize = 128;
    currentPiece = 0;
}

int UploadData::GetFileData(){
    QFile *file = new QFile();
    QString s = QFileDialog::getOpenFileName(NULL, tr("选取文件"), qApp->applicationDirPath(), tr("bin (*.bin)"));
    if(s == "")
        return -1;
    file->setFileName(s);
    if(!file->open(QIODevice::ReadOnly))
        return -1;
    fileData.clear();
    fileData = file->readAll();
    file->close();
    return 0;
}

QByteArray UploadData::GetUploadDataLen(){
    return xpgTool->IntToByte(fileData.size(), 4);
}

QByteArray UploadData::GetMD5(){
    QCryptographicHash md(QCryptographicHash::Md5);
    md.addData(fileData);
    MD5 = md.result();
    return MD5;
}

QByteArray UploadData::GetMD5Len(){
    return xpgTool->IntToByte(MD5.size(), 2);
}

void UploadData::calPieceSum(){
    pieceSum = fileData.size() / perPieceSize;
    if(fileData.size() % perPieceSize)
        pieceSum+=1;
    pieceCount = xpgTool->IntToByte(pieceSum, 2);
}

QByteArray UploadData::GetOnePackage(QByteArray Data, bool isFirst){
    if(isFirst)
        currentPiece = 0;
    currentPiece++;
    Data.append(xpgTool->IntToByte(currentPiece,2));
    Data.append(pieceCount);

    QByteArray tmp;
    tmp.resize(perPieceSize);
    int index = perPieceSize * (currentPiece-1);
    tmp = fileData.mid(index, perPieceSize);
    Data.append(tmp);

    if(currentPiece == pieceSum){
        QByteArray a;
        a.resize(1);
        a[0]=0x01;
        Data.replace(7, 1, a);
    }

    return Data;
}
