#ifndef XPGFRAMER_H
#define XPGFRAMER_H

#include <QObject>
#include <stdint.h>
#include <xpgserialport.h>
#include <xpgframer.h>
#include <xpgtool.h>

class XpgFramer : public QObject
{
    Q_OBJECT
public:
    explicit XpgFramer(XpgSerialPort * xpgSerialPort, XpgTool *xpgTool, QObject *parent = 0);
    ~XpgFramer();

    void InitData();
    QByteArray LineData;

signals:
    void LineDataReady(QByteArray);

private:
    XpgSerialPort *xpgSerialPort;
    XpgTool *xpgTool;
    bool Flag;
    int tmp;

private slots:
    void dataToV4(QByteArray Data);
};

#endif // XPGFRAMER_H
