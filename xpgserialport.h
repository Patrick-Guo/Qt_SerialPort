#ifndef XPGSERIALPORT_H
#define XPGSERIALPORT_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMutex>

class XpgSerialPort : public QSerialPort, QSerialPortInfo
{
    Q_OBJECT

public:
    XpgSerialPort();
    ~XpgSerialPort();
    void InitData();

    QList<QString> ComList;
    bool OpenCom(QString COM, QString RATE, int DATA, int STOP, int CHECK);
    void CloseCom();
    QByteArray Data;

private:
    QTimer timerComList;

private slots:
    void getComList();
    void readComData();

signals:
    void ComListChange(QList<QString>);
    void DataReady(QByteArray);
};

#endif // XPGSERIALPORT_H
