#include <QMessageBox>
#include <QTime>
#include "mcuwindow.h"
#include "ui_mcuwindow.h"

McuWindow::McuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::McuWindow)
{
    ui->setupUi(this);
    InitUi();
    InitData();
}

McuWindow::~McuWindow()
{
    delete ui;
}

void McuWindow::InitUi(){
    QString background_url = qApp->applicationDirPath()+ "/image/background.png";
    ui->label_background2->setStyleSheet("border-image:url(" + background_url + ")");
}

void McuWindow::InitData(){
    DataList.clear();
    xpgTool = new XpgTool();
    com1 = new XpgSerialPort();
    connect(com1, SIGNAL(ComListChange(QList<QString>)), this, SLOT(UpdateUi(QList<QString>)));
    xpgFramer = new XpgFramer(com1, xpgTool);
    connect(xpgFramer, SIGNAL(LineDataReady(QByteArray)), this, SLOT(fromFramer(QByteArray)));
    analyze = new XpgAnalyze(xpgFramer, xpgTool);
    connect(analyze, SIGNAL(RespondReady(QByteArray)), this, SLOT(fromRespond(QByteArray)));
    connect(analyze, SIGNAL(SendUploadPackage(bool)), this, SLOT(sendUploadPackage(bool)));
    connect(analyze, SIGNAL(StopUpload()), this, SLOT(stopUpload()));
    xpgJson = new XpgJson();
    connect(xpgJson, SIGNAL(json_stat(bool)), this, SLOT(jsonState(bool)));
    connect(xpgJson, SIGNAL(ListOk(QList<json_struct>)), this, SLOT(updateDataPointWindow(QList<json_struct>)));
    xpgJson->json2list(xpgTool->ReadIni("product_key/key"));
    upLoadTimer = new QTimer();
    connect(upLoadTimer, SIGNAL(timeout()), this, SLOT(autoUpLoad()));
    openFlag = false;
    upLoadFlag = false;
    mcuCmdWindow = new McuCmdWindow();
    connect(mcuCmdWindow, SIGNAL(SENDCMD(CMD)), this, SLOT(SendCMD(CMD)));
    mcuSettingWindow = new McuSettingWindow(xpgTool);
    uploadData = new UploadData();
}

void McuWindow::UpdateUi(QList<QString> ComList){
    QString tmp_com;
    ui->com_3->clear();
    int i = 0;
    foreach(QString tmp, ComList){
        ui->com_3->insertItem(i, tmp);
        if(tmp_com == tmp)
            ui->com_3->setCurrentText(tmp);
        i++;
    }
}

void McuWindow::SendData(QString s){
    QByteArray Data = QByteArray::fromHex(s.toLatin1());
    com1->write(Data);
}

//APP数据下发时更新界面
void McuWindow::DataPointSetValue(QByteArray Data){
    for(int i=0; i<xpgJson->writable; i++){
        ui->datapoint_table->item(i, 0)->setBackgroundColor(QColor(255,255,255));
    }

    QMap<QString, QString> map = xpgJson->analyze_json(Data);
    QMapIterator<QString, QString> j(map);
    while(j.hasNext()){
        j.next();
        for(int i=0; i<ui->datapoint_table->rowCount(); i++){
            QString TableName = ui->datapoint_table->item(i,0)->text();
            if(j.key() == TableName){
                QWidget * widget = ui->datapoint_table->cellWidget(i, 1);
                QString data_value = j.value();
                if(widget->inherits("QSpinBox")){
                        ui->datapoint_table->item(i, 0)->setBackgroundColor(QColor(252,222,156));
                        QSpinBox * spinBox = (QSpinBox *)ui->datapoint_table->cellWidget(i, 1);
                        spinBox->setValue(data_value.toInt());
                }else if(widget->inherits("QComboBox")){
                        ui->datapoint_table->item(i, 0)->setBackgroundColor(QColor(252,222,156));
                        QComboBox * comBox = (QComboBox *)ui->datapoint_table->cellWidget(i, 1);
                        comBox->setCurrentText(data_value);
                }else if(widget->inherits("QLineEdit")){
                        ui->datapoint_table->item(i, 0)->setBackgroundColor(QColor(252,222,156));
                        QLineEdit * lineEdit = (QLineEdit *)ui->datapoint_table->cellWidget(i, 1);
                        lineEdit->setText(data_value);
                }
            }
        }
    }
}

void McuWindow::CleanRec(){
    ui->listWidget->clear();
}

void McuWindow::ResetWindow(){
    if(com1->isOpen()){
        com1->close();
        xpgFramer->LineData.clear();
    }
    ui->pushButton_open3->setText("打开串口");
}

QByteArray McuWindow::GetUpLoadData(){
    QByteArray Data = xpgJson->GetUpLoadData();
    for(int row=0; row < ui->datapoint_table->rowCount(); row++){
        QString DataName = ui->datapoint_table->item(row, 0)->text();
        int DataValue = 0;
        QWidget * widget = ui->datapoint_table->cellWidget(row, 1);
        if(widget->inherits("QSpinBox")){
                QSpinBox * spinBox = (QSpinBox *)ui->datapoint_table->cellWidget(row, 1);
                DataValue = spinBox->value();
                Data = xpgJson->InsertValueToData(Data, DataName, DataValue);
        }else if(widget->inherits("QComboBox")){
                QComboBox * comBox = (QComboBox *)ui->datapoint_table->cellWidget(row, 1);
                DataValue = comBox->currentIndex();
                Data = xpgJson->InsertValueToData(Data, DataName, DataValue);
        }else if(widget->inherits("QLineEdit")){
                QLineEdit * lineEdit = (QLineEdit *)ui->datapoint_table->cellWidget(row, 1);
                QString s = lineEdit->text();
                int editLen = lineEdit->maxLength();
                for(int i=0; s.size()<editLen; i++){
                    s.append("0");
                }
                QByteArray temp = QByteArray::fromHex(s.toLatin1());
                Data = xpgJson->InsertValueToData(Data, DataName, temp);
        }
    }
    unsigned int len = xpgTool->GetWord(Data, 2);
    unsigned int len_tmp = Data.mid(4).length();
    for(unsigned int i = len_tmp; i<len; i++){
        Data.append(0xFF);
    }

    qDebug()<<Data.toHex();
    unsigned char checksum = xpgTool->CheckSum(Data, 2, 1);
    checksum &= 0x000000FF;
    Data[Data.length()-1] = checksum;
    return Data;
}

void McuWindow::CheckKey(){
    QString key = xpgTool->ReadIni("product_key/key");
    if(key == "01234567891011121314151617181920"){
        QMessageBox::about(this,"提示","第一次使用请输入product_key！");
    }
}

//------槽函数------//
void McuWindow::on_pushButton_open3_clicked()
{
    if(!com1->isOpen()){
        QString COM = ui->com_3->currentText();
        QString RATE = ui->rate_3->currentText();
        int DATA = ui->data_3->currentIndex();
        int STOP = ui->stop_3->currentIndex();
        int CHECK = ui->check_3->currentIndex();
        bool IsOpen = com1->OpenCom(COM, RATE, DATA, STOP, CHECK);
        if(IsOpen){
            ui->pushButton_open3->setText("关闭串口");
            }else{
            QMessageBox::about(this, "错误", "打开失败！");
        }
    }else{
        com1->CloseCom();
        upLoadTimer->stop();
        ui->pushButton_autoupload->setText("定时上报开");
        openFlag = false;
        ui->pushButton_open3->setText("打开串口");
        xpgFramer->LineData.clear();
    }
}

//显示来自wifi模块的数据
void McuWindow::fromFramer(QByteArray Data){
    if(Data.at(4) == 0x03 && Data.at(8) == 0x01){
        DataPointSetValue(Data);
    }
    Data = xpgTool->InsertSpace(Data);
    ui->listWidget->addItem("WIFI:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
    ui->listWidget->addItem(Data);
    ui->listWidget->scrollToBottom();
}

//显示MCU回复的数据
void McuWindow::fromRespond(QByteArray Data){
    QByteArray tmp = Data;
    if(Data.at(4)==0x04 && Data.at(8)==0x03){
        unsigned char sn = Data.at(5);
        Data.clear();
        Data = GetUpLoadData();
        Data[4]=0x04;
        Data[5]=sn;
        Data[8]=0x03;
        unsigned char checksum = xpgTool->CheckSum(Data, 2, 1);
        checksum &= 0x000000FF;
        Data[Data.length()-1] = checksum;
    }

    Data = xpgTool->Insert55(Data);

    com1->write(Data);
    Data = xpgTool->InsertSpace(Data);
    ui->listWidget->addItem("MCU:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
    ui->listWidget->addItem(Data);
    ui->listWidget->scrollToBottom();

    //当cmd == 0x1C时开始发送大数据分片
    if(tmp.at(4) == 0x1C){
        uploadData->calPieceSum();
        upLoadFlag = true;
        sendUploadPackage(true);
    }
}

void McuWindow::on_pushButton_send3_clicked()
{
    QString tmp = ui->textEdit_send3->toPlainText();
    if(com1->isOpen()){   
        SendData(tmp);
        QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
        Data = xpgTool->InsertSpace(Data);
        ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
        ui->listWidget->addItem(Data);
        ui->listWidget->scrollToBottom();
    }
}

//保存product_key
void McuWindow::on_pushButton_productkey_clicked()
{
    QString key = ui->lineEdit_productkey->text();
    if(!analyze->SaveProductKey(key)){
        QMessageBox::about(this, "错误", "输入格式错误");
        return;
    }
    key = xpgTool->ReadIni("product_key/key");
    QString envirnment = xpgTool->ReadIni("environment/environment");
    xpgJson->get_json(key, envirnment);
}

void McuWindow::jsonState(bool state){
    if(state){
        QMessageBox::about(this, "成功", "获取数据点列表成功！");
        QString key = xpgTool->ReadIni("product_key/key");
        xpgJson->json2list(key);
    }else
        QMessageBox::about(this, "失败", "获取数据点列表失败，请检查product_key是否正确！");
}

//更新json
void McuWindow::on_pushButton_updatejson_clicked()
{
    QString key = xpgTool->ReadIni("product_key/key");
    QString envirnment = xpgTool->ReadIni("environment/environment");
    xpgJson->get_json(key, envirnment);
}

//单击指令列表进行解析
void McuWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    if(item->text().contains("WIFI") || item->text().contains("MCU") || item->text().contains("SEND"))
        return;
    QByteArray Data = QByteArray::fromHex(item->text().toLatin1());
    data_analyze myData = analyze->Analyze(Data);
    ui->label_analyze->setText(myData.text);
    ui->label_length->setText(myData.length);
    ui->label_cmd->setText(myData.cmd);
    ui->label_sn->setText(myData.sn);
    ui->label_checksum_2->setText(myData.checksum);
    ui->label_setmode->setText(myData.setmode);
    ui->label_connect->setText(myData.connect);
    ui->label_wifistate->setText(myData.wifistate);
    ui->label_signal->setText(myData.signal);
    ui->label_hasconnect->setText(myData.hasconnect);
    ui->label_productkey->setText(myData.product_key);
    ui->label_time->setText(myData.time);
}

//更新数据点界面
void McuWindow::updateDataPointWindow(QList<json_struct> jsonList){
    ui->datapoint_table->clear();
    ui->datapoint_table->setRowCount(jsonList.count());
    ui->datapoint_table->setColumnCount(2);
    ui->datapoint_table->setColumnWidth(0,220);
    int row = 0;
    foreach(json_struct temp, jsonList){
        ui->datapoint_table->setItem(row, 0, new QTableWidgetItem(temp.display_name));
        if(temp.data_type == "bool"){
            QSpinBox *box = new QSpinBox(this);
            box->setMaximum(1);
            box->setMinimum(0);
            //box->setEnabled(false);
            ui->datapoint_table->setItem(row, 1, new QTableWidgetItem());
            ui->datapoint_table->setCellWidget(row, 1, box);
        }else if(temp.data_type == "enum"){
            QComboBox *box = new QComboBox(this);
            for(int i=0; i<temp.enum_list.count(); i++){
                QString s = temp.enum_list.at(i);
                box->insertItem(i, s);
            }
            ui->datapoint_table->setItem(row, 1, new QTableWidgetItem());
            ui->datapoint_table->setCellWidget(row, 1, box);
        }else if(temp.data_type == "uint8" || temp.data_type == "uint16" || temp.data_type == "uint32"){
            QSpinBox *box = new QSpinBox(this);
            int maxnum = temp.maxnum;
            int minnum = temp.mininum;
            box->setMaximum(maxnum);
            box->setMinimum(minnum);
            ui->datapoint_table->setItem(row, 1, new QTableWidgetItem());
            ui->datapoint_table->setCellWidget(row, 1, box);
        }else if(temp.data_type == "binary"){
            int len = temp.len;
            QLineEdit *line = new QLineEdit(this);
            QRegExp regx("[a-fA-F0-9]+$");
            QValidator *validator = new QRegExpValidator(regx, line);
            line->setValidator(validator);
            line->setMaxLength(len*2);
            ui->datapoint_table->setItem(row, 1, new QTableWidgetItem());
            ui->datapoint_table->setCellWidget(row, 1, line);
        }
        if(temp.type=="status_readonly" ||temp.type=="alert" || temp.type=="fault")
            ui->datapoint_table->item(row, 0)->setBackgroundColor(QColor(221,221,221));
        row++;
    }
}

//上报数据
void McuWindow::on_pushButton_upload_clicked()
{
    if(com1->isOpen()){
        QByteArray Data;
        Data = GetUpLoadData();
        Data = xpgTool->Insert55(Data);
        com1->write(Data);
        Data = xpgTool->InsertSpace(Data);
        ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
        ui->listWidget->addItem(Data);
        ui->listWidget->scrollToBottom();
    }
}

//自动上报数据
void McuWindow::on_pushButton_autoupload_clicked()
{
    if(openFlag){
        upLoadTimer->stop();
        ui->pushButton_autoupload->setText("定时上报开");
        openFlag = false;
    }else{
        if(com1->isOpen())
        {
            upLoadTimer->start(xpgTool->ReadIni("setting/uploadtime").toInt()*1000);
            ui->pushButton_autoupload->setText("定时上报关");
            openFlag = true;
        }
    }
}

void McuWindow::autoUpLoad(){
    if(com1->isOpen()){
        QByteArray Data;
        Data = GetUpLoadData();
        Data = xpgTool->Insert55(Data);
        com1->write(Data);
        Data = xpgTool->InsertSpace(Data);
        ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
        ui->listWidget->addItem(Data);
        ui->listWidget->scrollToBottom();
    }
}

void McuWindow::SendCMD(CMD cmd){
    if(com1->isOpen()){
        switch(cmd){
        case AirLink:
        {
            QString tmp = xpgTool->ReadIni("Defaule/492");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case SoftAp:
        {
            QString tmp = xpgTool->ReadIni("Defaule/491");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case Reset:
        {
            QString tmp = xpgTool->ReadIni("Defaule/4B");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case GetTime:
        {
            QString tmp = xpgTool->ReadIni("Defaule/417");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case OnBording:
        {
            QString tmp = xpgTool->ReadIni("Defaule/415");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case StartOTA:
        {
            QByteArray Data;
            int otaPiece = xpgTool->ReadIni("setting/otapiece").toInt();
            QString otaType = xpgTool->ReadIni("setting/otatype");
            QByteArray perSize;
            perSize = xpgTool->IntToByte(otaPiece, 2);
            QString s = xpgTool->ReadIni("Defaule/41B");
            Data = QByteArray::fromHex(s.toLatin1());
            Data = Data.append(analyze->MD5Len);
            Data = Data.append(analyze->MD5);
            Data = Data.append(perSize);
            int a = Data.length()-3;
            Data.replace(2,2,xpgTool->IntToByte(a,2));
            if(otaType == "hex"){
                QString type = "01";
                Data.replace(7, 1, QByteArray::fromHex(type.toLatin1()));
            }
            unsigned char checksum = xpgTool->CheckSum(Data,2,0);
            checksum &= 0x000000FF;
            Data[Data.length()] = checksum;
            for(int i=Data.length(); i>2; i--){
                if((unsigned char)Data[i] == 0xff){
                    Data.insert(i+1, 0x55);
                }
            }
            com1->write(Data);
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("MCU:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case Test:
        {
            QString tmp = xpgTool->ReadIni("Defaule/413");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case Upload:
        {
            int i = uploadData->GetFileData();
            if(i != 0)
                break;

            QByteArray MD5 = uploadData->GetMD5();
            QByteArray dataLen = uploadData->GetUploadDataLen();
            QByteArray MD5Len = uploadData->GetMD5Len();

            QString tmp = xpgTool->ReadIni("Defaule/419");
            QByteArray Data;
            Data = QByteArray::fromHex(tmp.toLatin1());
            Data.append(dataLen);
            Data.append(MD5Len);
            Data.append(MD5);

            int dataSize = Data.length()-3;
            QByteArray DataSize = xpgTool->IntToByte(dataSize, 2);
            Data.replace(2, 2, DataSize);

            unsigned char checksum = xpgTool->CheckSum(Data, 2, 0);
            checksum &= 0x000000FF;
            Data.append(checksum);

            for(int i=Data.length(); i>2; i--){
                if((unsigned char)Data[i] == 0xff){
                    Data.insert(i+1, 0x55);
                }
            }
            com1->write(Data);
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("MCU:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;
        }
        case Restart:{
            QString tmp = xpgTool->ReadIni("Defaule/429");
            SendData(tmp);
            QByteArray Data = QByteArray::fromHex(tmp.toLatin1());
            Data = xpgTool->InsertSpace(Data);
            ui->listWidget->addItem("SEND:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
            ui->listWidget->addItem(Data);
            ui->listWidget->scrollToBottom();
            break;}
        case StopUpload:
            stopUpload();
            break;
        }
    }
}

void McuWindow::on_pushButton_showcmd_clicked()
{
    if(mcuCmdWindow->isHidden())
        mcuCmdWindow->show();
    else
        mcuCmdWindow->hide();
}

void McuWindow::on_pushButton_setting_clicked()
{
    if(mcuSettingWindow->isHidden()){
        mcuSettingWindow->initData();
        mcuSettingWindow->show();
    }else{
        mcuSettingWindow->hide();
    }
}

void McuWindow::SendFirstUploadData(){

//    if(!upLoadFlag)
//        return;

//    QString tmp = xpgTool->ReadIni("Defaule/41D");
//    QByteArray Data;
//    Data = QByteArray::fromHex(tmp.toLatin1());
//    Data = uploadData->GetOnePackage(Data, true);

//    int dataSize = Data.length()-3;
//    QByteArray DataSize = xpgTool->IntToByte(dataSize, 2);
//    Data.replace(2, 2, DataSize);

//    unsigned char checksum = xpgTool->CheckSum(Data, 2, 0);
//    checksum &= 0x000000FF;
//    Data.append(checksum);

//    for(int i=Data.length(); i>2; i--){
//        if((unsigned char)Data[i] == 0xff){
//            Data.insert(i+1, 0x55);
//        }
//    }
//    com1->write(Data);
//    Data = xpgTool->InsertSpace(Data);
//    ui->listWidget->addItem("MCU:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
//    ui->listWidget->addItem(Data);
//    ui->listWidget->scrollToBottom();
}

void McuWindow::sendUploadPackage(bool IsFirstPackage){
    qDebug()<<upLoadFlag;
    if(!upLoadFlag)
        return;

    if(uploadData->currentPiece == uploadData->pieceSum)
        return;
    QString tmp = xpgTool->ReadIni("Defaule/41D");
    QByteArray Data;
    Data = QByteArray::fromHex(tmp.toLatin1());
    Data = uploadData->GetOnePackage(Data, IsFirstPackage);

    int dataSize = Data.length()-3;
    QByteArray DataSize = xpgTool->IntToByte(dataSize, 2);
    Data.replace(2, 2, DataSize);

    unsigned char checksum = xpgTool->CheckSum(Data, 2, 0);
    checksum &= 0x000000FF;
    Data.append(checksum);

    for(int i=Data.length(); i>2; i--){
        if((unsigned char)Data[i] == 0xff){
            Data.insert(i+1, 0x55);
        }
    }
    com1->write(Data);
    Data = xpgTool->InsertSpace(Data);
    ui->listWidget->addItem("MCU:"+QTime::currentTime().toString("hh:mm:ss.zzz"));
    ui->listWidget->addItem(Data);
    ui->listWidget->scrollToBottom();
}

void McuWindow::stopUpload(){
    qDebug()<<"in";
    upLoadFlag = false;
}
