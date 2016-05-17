#ifndef XPGJSON_H
#define XPGJSON_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonParseError>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork>
#include <QDebug>
#include <QFile>
#include <xpgtool.h>
#include <stdint.h>

#define Head_len 9

typedef struct
{
    QString display_name;
    QString name;
    QString data_type;
    QString unit;
    int len;
    int byte_offset;
    int bit_offset;
    QString type;
    int id;
    QString desc;
    QList<QString> enum_list;
    int mininum;
    int maxnum;
}json_struct;

class XpgJson : public QObject
{
    Q_OBJECT
public:
    typedef enum _XpgDirection
    {
        FORWARD,
        BACKWARD,
    } XpgDirection;

    explicit XpgJson(QObject *parent = 0);
    ~XpgJson();
    void get_json(QString key, QString environment);
    QByteArray GetUpLoadData();
    QByteArray InsertValueToData(QByteArray Data, QString DataName, int DataValue);
    QByteArray InsertValueToData(QByteArray Data, QString DataName, QByteArray DataValue);
    QByteArray intToByte(int Number, int NumSize);
    json_struct init_struct(json_struct temp_struct);
    void json2list(QString key);
    uint16_t GetBits(QByteArray data, uint byteOffset, uint bitOffset, uint bitLen, XpgDirection direction);
    QMap<QString, QString> analyze_json(QByteArray);

    QList<json_struct> json_list;//存放各个解析后数据点的定义。
    int writable;//可写数据点个数
    int bitWriteAble;//可写数据点bit总数
    int bitReadOnly;//只读数据点bit总数
    int bitAlert;//警报数据点bit总数
    int bitFault;//故障数据点bit总数
    XpgTool *xpgTool;

private:
    QString FileName;

signals:
    void json_stat(bool);
    void ListOk(QList<json_struct>);

public slots:
    void replyFinished(QNetworkReply*);
};

#endif // XPGJSON_H
