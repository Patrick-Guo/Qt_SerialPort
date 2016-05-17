#ifndef XPGTOOL_H
#define XPGTOOL_H

#include <QObject>
#include <stdint.h>
#include <QSettings>
#include <QApplication>

class XpgTool : public QObject
{
    Q_OBJECT

public:
    XpgTool();
    ~XpgTool();
    void InitData();
    unsigned int GetByte(QByteArray Data, int position);
    unsigned int GetWord(QByteArray Data, int position);
    unsigned char CheckSum(QByteArray Data, int position, int end);
    QByteArray InsertSpace(QByteArray Data);
    QByteArray IntToByte(int Number, int NumSize);
    QByteArray Insert55(QByteArray Data);
    QSettings *setting;
    void WriteIni(QString local, QByteArray value);
    void WriteIni(QString local, QString value);
    QString ReadIni(QString local);

private:
    QByteArray LineData;
};

#endif // XPGTOOL_H
