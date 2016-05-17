#ifndef XPGANALYZE_H
#define XPGANALYZE_H

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <xpgframer.h>
#include <xpgtool.h>
#include <QFile>

typedef struct
{
    QString text;
    QString length;
    QString cmd;
    QString sn;
    QString checksum;
    QString setmode;
    QString connect;
    QString wifistate;
    QString product_key;
    QString signal;
    QString hasconnect;
    QString time;
}data_analyze;

class XpgAnalyze : public QObject
{
    Q_OBJECT
public:
    explicit XpgAnalyze(XpgFramer *xpgFramer, XpgTool *xpgTool ,QObject *parent = 0);
    ~XpgAnalyze();
    void InitData();
    bool SaveProductKey(QString key);
//    QString GetIniData(QString local);
//    void WriteIni(QString local, QByteArray value);
    data_analyze Analyze(QByteArray Data);
//    bool changEnvironment(QString str);
    QByteArray MD5Len, MD5;

signals:
    void RespondReady(QByteArray);
    void SendUploadPackage(bool IsFirstPackage);
    void StopUpload();

private:
    XpgFramer *xpgFramer;
    XpgTool *xpgTool;
    QSettings *setting;
    QFile *binFile, *hexFile;
    QString appLocal;
    unsigned int package_sn;

private slots:
    void FindRespond(QByteArray Data);
};

#endif // XPGANALYZE_H
