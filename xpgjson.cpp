#include "xpgjson.h"
#include <QApplication>

XpgJson::XpgJson(QObject *parent) : QObject(parent)
{
    xpgTool = new XpgTool();
    writable = 0;
    bitWriteAble = 0;
    bitReadOnly = 0;
    bitAlert = 0;
    bitFault = 0;
}

XpgJson::~XpgJson()
{

}

void XpgJson::get_json(QString key, QString environment){
    FileName = qApp->applicationDirPath()+"/json/"+key+".json";
    QString json_url = "http://";
    qDebug()<<"111"<<xpgTool->ReadIni("environment/environment");
    json_url += xpgTool->ReadIni("environment/environment");
    json_url += "/v2/datapoint?product_key=";
    json_url += key;
    json_url += "&format=json";
    qDebug()<<json_url;
    QNetworkRequest request;
    request.setUrl(QUrl(json_url));
    QNetworkAccessManager *manager;
    manager = new QNetworkAccessManager(this);
    manager->get(request);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    qDebug()<<"in_getjson";
}

void XpgJson::replyFinished(QNetworkReply *reply){
    QByteArray data = reply->readAll();
    qDebug()<<"in_reply";
    QJsonParseError json_error;
    QJsonDocument JsonDocument = QJsonDocument::fromJson(data, &json_error);

    if(json_error.error == QJsonParseError::NoError)
    {
        if(JsonDocument.isObject())
        {
            QJsonObject obj = JsonDocument.object();
            //判断获取回来的json数据是否有效
            if(obj.contains("errors")){
                emit json_stat(false);
            }else{
                QFile file(FileName);
                if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate)){
                    return ;
                }
                file.write(data);
                file.close();
                emit json_stat(true);
            }
        }
    }

}

//初始化json_struct
json_struct XpgJson::init_struct(json_struct temp_struct){
    temp_struct.bit_offset = 0;
    temp_struct.byte_offset = 0;
    temp_struct.data_type = "";
    temp_struct.desc = "";
    temp_struct.display_name = "";
    temp_struct.enum_list.clear();
    temp_struct.id = 0;
    temp_struct.len = 0;
    temp_struct.maxnum = 0;
    temp_struct.mininum = 0;
    temp_struct.name = "";
    temp_struct.type = "";
    temp_struct.unit = "";
    return temp_struct;
}

//把json转换为数据点列表
void XpgJson::json2list(QString key){
    json_list.clear();
    writable = 0;
    bitWriteAble = 0;
    bitReadOnly = 0;
    bitAlert = 0;
    bitFault = 0;
    //获取对应json文件中的数据
    FileName = qApp->applicationDirPath()+"/json/"+key+".json";
    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly)){
        return ;
    }
    QByteArray json_value = file.readAll();
    file.close();

    //把json数据转成可分析类型
    QJsonParseError json_error;
    QJsonDocument JsonDocument = QJsonDocument::fromJson(json_value, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(JsonDocument.isObject())
        {
            QJsonObject obj = JsonDocument.object();
            QJsonArray entities = obj.take("entities").toArray();
            QJsonObject entity = entities[0].toObject();
            QJsonArray attrs = entity.take("attrs").toArray();
            //分析每个数据点类型和相应操作
            for(int i=0; i<attrs.count(); i++){
                QJsonObject temp_obj = attrs.at(i).toObject();
                //把当前数据点的参数放入结构体中
                json_struct temp_struct;
                temp_struct = init_struct(temp_struct);

                temp_struct.display_name = temp_obj.take("display_name").toString();
                temp_struct.name         = temp_obj.take("name").toString();
                temp_struct.data_type    = temp_obj.take("data_type").toString();
                temp_struct.type         = temp_obj.take("type").toString();
                temp_struct.id           = temp_obj.take("id").toInt();
                temp_struct.desc         = temp_obj.take("desc").toString();

                QJsonObject position_obj = temp_obj.take("position").toObject();
                temp_struct.unit         = position_obj.take("unit").toString();
                temp_struct.len          = position_obj.take("len").toInt();
                temp_struct.byte_offset  = position_obj.take("byte_offset").toInt();
                temp_struct.bit_offset   = position_obj.take("bit_offset").toInt();
                //计算可写数据点个数
                if(temp_struct.type == "status_writable")
                    writable++;
                //计算只读数据点bit总数
                if(temp_struct.type == "status_writable" && temp_struct.unit == "bit")
                    bitWriteAble += temp_struct.len;
                //计算警报数据点bit总数
                if(temp_struct.type == "status_readonly" && temp_struct.unit == "bit")
                    bitReadOnly += temp_struct.len;
                //计算故障数据点bit总数
                if(temp_struct.type == "alert" && temp_struct.unit == "bit")
                    bitAlert += temp_struct.len;
                //计算可写数据点bit总数
                if(temp_struct.type == "fault" && temp_struct.unit == "bit")
                    bitFault += temp_struct.len;
                //获取枚举型数值和uint的最大最小值
                if(temp_struct.data_type == "bool"){

                }else if(temp_struct.data_type == "enum"){
                    QJsonArray arr = temp_obj.take("enum").toArray();
                    for(int i=0; i<arr.count(); i++){
                        temp_struct.enum_list.append(arr.at(i).toString());
                    }
                }else if(temp_struct.data_type == "uint8" || temp_struct.data_type == "uint16" || temp_struct.data_type == "uint32"){
                    QJsonObject uint_spec_obj = temp_obj.take("uint_spec").toObject();
                    temp_struct.maxnum = uint_spec_obj.take("max").toInt();
                    temp_struct.mininum = uint_spec_obj.take("min").toInt();
                }else if(temp_struct.data_type == "binary"){

                }
                //把结构体插入列表中
                //if(temp_struct.type == "status_writable")
                    json_list.append(temp_struct);
            }
        }
        emit ListOk(json_list);
    }
}


#define BITS_PER_BYTE (8)
#define BITS_PER_WORD (16)
#define GET_BIT(theByte, theBit) (((theByte) & (0x0001<<(theBit))) >> (theBit))
#define SET_BIT(theByte, theBit) ((theByte) = (theByte) | (0x0001<<(theBit)))
uint16_t XpgJson::GetBits(QByteArray data, uint byteOffset, uint bitOffset, uint bitLen, XpgDirection direction)
{
    QByteArray output;
    uint16_t wordBitsIn = 0;
    uint16_t wordBitsOut = 0;
    bool flag = false;
    int i = 0;
    int byteLen = 0;

    // 修正 byte offset, bit offset 到 8 位对齐
    if (FORWARD == direction)
        byteOffset += bitOffset / 8;
    else
        byteOffset -= bitOffset / 8;
    bitOffset = bitOffset % 8;


    // 求 bit 所占的 byte 区域大小
    byteLen = bitLen / 8;
    byteLen += (0 == bitLen % 8) ? 0 : 1;

    if((bitOffset + bitLen) > 8 && bitLen < 8){
        byteLen+=1;
        flag = true;
    }
    // 输入检查
    if (0 == bitLen)
    {
        qDebug() << __func__ << "(), WARN: bitLen = 0";
        return 0;
    }
    if (byteOffset + byteLen > data.length())
    {
        qDebug() << __func__ << "(), WARN: byteOffset+(bitOffset+bitLen) overflows";
        return 0;
    }

    // TODO: 下标越界
    if (direction == FORWARD)
    {
        output = data.mid(byteOffset, byteLen);
        unsigned char * pData = (unsigned char *)output.data();
        // HACK: 对于大于16位的情况，会有精度丢失。
        if (2 == byteLen)
        {
            wordBitsOut = pData[0];
            wordBitsOut = wordBitsOut << BITS_PER_BYTE;
            wordBitsOut |= pData[1];
        }
        else
            wordBitsOut = *pData;
    }
    else
    {
        output = data.mid(byteOffset - byteLen, byteLen);
        unsigned char * pData = (unsigned char *)output.data();
        // HACK: 对于大于16位的情况，会有精度丢失。
        if (2 == byteLen)
        {
            wordBitsIn = pData[0];
            wordBitsIn = wordBitsIn << BITS_PER_BYTE;
            wordBitsIn |= pData[1];
        }
        else
            wordBitsIn = *pData;
        // 取字节或者字内容的逆序位布局。e.g. 0011 0101 -> 1010 1100
        wordBitsOut = 0;
        for (i=0; i<byteLen*BITS_PER_BYTE; i++)
        { // 0-15
            if (GET_BIT(wordBitsIn, i))
            {
                SET_BIT(wordBitsOut, (byteLen*BITS_PER_BYTE - 1) - i);
            }
        }
    }

    if (bitLen > BITS_PER_BYTE || flag)
    { // e.g. 0x3132 = 0011 0001 0011 0010,
        // then << 3 = 1000 1001 1001 0000
        // then >> (16 - 2) = 0000 0000 0000 0010
        wordBitsOut = wordBitsOut << bitOffset;
        wordBitsOut = wordBitsOut >> (BITS_PER_WORD - bitLen);
    }
    else
    {
        wordBitsOut = wordBitsOut << bitOffset;
        wordBitsOut = wordBitsOut & 0x00FF;
        wordBitsOut = wordBitsOut >> (BITS_PER_BYTE - bitLen);
    }
    return wordBitsOut;
}

//根据json和下发指令解析具体数据
QMap<QString, QString> XpgJson::analyze_json(QByteArray data){
    QMap<QString, QString> map;//用于存放key和value返回给界面。
    //attr_flag所占字节数
    int attr_flag_len = 0;
    if(writable == 0){
        attr_flag_len = 0;
    }else if(writable%8==0){
        attr_flag_len = writable/8;
    }else{
        attr_flag_len = writable/8+1;
    }
    //bool和enum所占字节数
    int bit_len = 0;
    if(bitWriteAble == 0){
        bit_len = 0;
    }else if(bitWriteAble%8==0){
        bit_len = bitWriteAble/8;
    }else{
        bit_len = bitWriteAble/8+1;
    }
    QByteArray attr_flag = data.mid(9, attr_flag_len);
    //根据attr_flag读取数据
    int temp_writable = writable;
    int bit_num = 0;
    //从attr_flag的最右一位开始遍历
    for(int byte=attr_flag_len-1; byte>=0; byte--){
        if(temp_writable>8)
            bit_num = 8;
        else
            bit_num = temp_writable;
        for(int bit=0; bit<bit_num; bit++){
            if(attr_flag.at(byte)&(0x01<<bit)){
                int tmp = -(byte - attr_flag_len + 1);//把attr_flag转为json_list中相对应的位置
                int offset = tmp*8+bit;
                QString name = json_list.value(offset).display_name;
                int byte_offset = json_list.value(offset).byte_offset;
                int bit_offset = json_list.value(offset).bit_offset;
                int len = json_list.value(offset).len;
                QString data_type = json_list.value(offset).data_type;
                if(data_type == "bool"){
                    int i = Head_len + attr_flag_len + bit_len;
                    uint16_t value = GetBits(data, i, bit_offset, len, BACKWARD);
                    QString s = QString::number(value, 10);
                    map.insert(name, s);
                }else if(data_type == "enum"){
                    int i = Head_len + attr_flag_len + bit_len;
                    uint16_t valueIn = GetBits(data, i, bit_offset, len, BACKWARD);
                    uint16_t value = 0;
                    for (i=0; i<BITS_PER_WORD; i++)
                    { // 0-15
                        if (GET_BIT(valueIn, i))
                        {
                            SET_BIT(value, (BITS_PER_WORD - 1) - i);
                        }
                    }
                    value = value >> (BITS_PER_WORD - len);
                    QString enum_value = json_list.value(offset).enum_list.at(value);
                    map.insert(name, enum_value);
                }else if(data_type == "uint8"){
                    int i = Head_len + attr_flag_len + byte_offset;
                    uint8_t value = data.at(i);
                    QString s = QString::number(value, 10);
                    map.insert(name, s);
                }else if(data_type == "uint16"){
                    int i = Head_len + attr_flag_len + byte_offset;
                    uint16_t temp1 = data.at(i);
                    uint16_t temp2 = data.at(i+1);
                    temp1 = (temp1&0x00ff)<<8;
                    temp2 = (temp2&0x00ff);
                    uint16_t value = temp1|temp2;
                    QString s = QString::number(value, 10);
                    map.insert(name, s);
                }else if(data_type == "uint32"){
                    int i = Head_len + attr_flag_len + byte_offset;
                    uint32_t temp1 = data.at(i);
                    uint32_t temp2 = data.at(i+1);
                    uint32_t temp3 = data.at(i+2);
                    uint32_t temp4 = data.at(i+3);
                    temp1 = (temp1&0x000000ff)<<24;
                    temp2 = (temp2&0x000000ff)<<16;
                    temp3 = (temp3&0x000000ff)<<8;
                    temp4 = (temp4&0x000000ff);
                    uint32_t value = temp1|temp2|temp3|temp4;
                    QString s = QString::number(value, 10);
                    map.insert(name, s);
                }else if(data_type == "binary"){
                    int i = Head_len + attr_flag_len + byte_offset;
                    QByteArray value = data.mid(i, len);
                    QString s = value.toHex();
                    map.insert(name, s);
                }
            }
        }
        temp_writable-=8;
    }
    return map;
}

QByteArray XpgJson::GetUpLoadData(){
    int bit_len = 0;
    int byte_len = 0;

    bit_len = bitWriteAble / 8;
    bit_len += (0 == bitWriteAble % 8) ? 0 : 1;
    byte_len += bit_len;

    bit_len = bitReadOnly / 8;
    bit_len += (0 == bitReadOnly % 8) ? 0 : 1;
    byte_len += bit_len;

    bit_len = bitAlert / 8;
    bit_len += (0 == bitAlert % 8) ? 0 : 1;
    byte_len += bit_len;

    bit_len = bitFault / 8;
    bit_len += (0 == bitFault % 8) ? 0 : 1;
    byte_len += bit_len;

    foreach(json_struct temp, json_list){
        if(temp.unit != "bit")
            byte_len += temp.len;
    }

    QByteArray Data;
    for(int i; i<byte_len+10; i++){
        Data[i] = 0x00;
    }
    Data[0] = 0xFF;
    Data[1] = 0xFF;
    Data[2] = ((byte_len + 6) & 0xff00)>>8;
    Data[3] = (byte_len + 6) & 0x00ff;
    Data[4] = 0x05;
    Data[8] = 0x04;
    return Data;
}

QByteArray XpgJson::InsertValueToData(QByteArray Data, QString DataName, int DataValue){
    int bitLen = 0;
    json_struct DataPoint;
    foreach(json_struct temp, json_list){
        if(DataName == temp.display_name){
            DataPoint = temp;
            break;
        }
    }
    //int byteOffset = DataPoint.byte_offset;
    int byteOffset =  0;
    int bitOffset = DataPoint.bit_offset;

    if(DataPoint.type == "status_writable"){
        bitLen = bitWriteAble / 8;
        bitLen += (0 == bitWriteAble % 8) ? 0 : 1;
    }else if(DataPoint.type == "status_readonly"){
        bitLen = bitReadOnly / 8;
        bitLen += (0 == bitReadOnly % 8) ? 0 : 1;
    }else if(DataPoint.type == "alert"){
        bitLen = bitAlert / 8;
        bitLen += (0 == bitAlert % 8) ? 0 : 1;
    }else if(DataPoint.type == "fault"){
        bitLen = bitFault / 8;
        bitLen += (0 == bitFault % 8) ? 0 : 1;
    }

    // memo: for bit points, points order is desc, bit order is asc
    // FF FF ?? ?? 05 ?? 00 00 04 [writable bit points] [writable byte points] [readonly bit points] [readonly byte points] [alert bit points] [alert byte points] [fault bit points] [fault byte points]
    // example: 9 fault points, 2 bits each, value 0b10 each, segment byte offset in json: 18
    // FF FF ?? ?? 05 ?? 00 00 04 [writable bit points] [writable byte points] [readonly bit points] [readonly byte points] [alert bit points] [alert byte points] <byte@(9+18)> 0b[00 00 00 XX XX XX XX XX XX XX XX XX] [fault byte points]
    // result 1: point 1, byte offset = header length + segment byte offset + segment byte len - (point bit offset / 8 + 1)
    //      = (9 + 18 + 3 - 1)
    // result 2: point 1, bit offset = () = 8 - 2

    // asc order first, then reverse
    // step #1 -> 0b[XX XX XX XX XX XX XX XX XX 00 00 00]
    // step #1 (point #1): 0b10 -> 0b01
    // step #2 (point #1): 0b[01 00 00 00]
    // step #3 (point #2): 0b10 -> 0b01
    // step #4 (point #2): 0b[01 01 00 00]
    // ...
    // step #17 (point #9): 0b10 -> 0b01
    // step #18 (point #9): 0b[01 01 01 01 01 01 01 01 01 00 00 00]
    // step #19 (reverse): 0b[00 00 00 10 10 10 10 10 10 10 10 10]
    if(DataPoint.unit == "bit"){
        byteOffset += ((bitOffset / 8)+1);
        //byteOffset += (0 == bitOffset % 8) ? 0 : 1;
        bitOffset = bitOffset % 8;
        if((bitOffset + DataPoint.len)>8){
            uint16_t temp = DataValue << bitOffset;
            Data[Head_len + DataPoint.byte_offset + bitLen - byteOffset] = (Data[Head_len + DataPoint.byte_offset + bitLen - byteOffset] | (temp & 0x00ff));
            Data[Head_len + DataPoint.byte_offset + bitLen - byteOffset - 1] = (Data[Head_len + DataPoint.byte_offset + bitLen - byteOffset - 1] | ((temp & 0xff00)>>8));
        }else{
            uint8_t dataValue = DataValue << bitOffset;
            Data[Head_len + DataPoint.byte_offset + bitLen - byteOffset] = (Data[Head_len + DataPoint.byte_offset + bitLen - byteOffset] | dataValue);
        }
    }else{
        QByteArray dataValue = intToByte(DataValue, DataPoint.len);
        Data.replace(Head_len + DataPoint.byte_offset, DataPoint.len, dataValue);
    }
    return Data;
}

QByteArray XpgJson::InsertValueToData(QByteArray Data, QString DataName, QByteArray DataValue){
    int byteOffset = 0;
    int DataLen = 0;
    foreach(json_struct temp, json_list){
        if(DataName == temp.display_name){
            byteOffset = temp.byte_offset;
            DataLen = temp.len;
            break;
        }
    }
    Data.replace(byteOffset+9, DataLen, DataValue);
    return Data;
}

QByteArray  XpgJson::intToByte(int Number, int NumSize)
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
