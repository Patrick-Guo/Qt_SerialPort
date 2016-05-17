#ifndef UPLOADDATA_H
#define UPLOADDATA_H

#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <xpgtool.h>
#include <QCryptographicHash>

class UploadData : public QWidget
{
    Q_OBJECT
public:
    explicit UploadData(QWidget *parent = 0);

    QByteArray fileData;
    QByteArray MD5;
    QByteArray pieceCount;
    int currentPiece;
    int pieceSum;
    int perPieceSize;

    int GetFileData();
    QByteArray GetUploadDataLen();
    QByteArray GetMD5();
    QByteArray GetMD5Len();
    void calPieceSum();
    QByteArray GetOnePackage(QByteArray, bool);

private:
    XpgTool *xpgTool;

signals:

public slots:
};

#endif // UPLOADDATA_H
