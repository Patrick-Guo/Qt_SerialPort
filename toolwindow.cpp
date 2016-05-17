#include "toolwindow.h"
#include "ui_toolwindow.h"

ToolWindow::ToolWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolWindow)
{
    ui->setupUi(this);
    QString background_url = qApp->applicationDirPath()+ "/image/background.png";
    ui->label_background->setStyleSheet("border-image:url(" + background_url + ")");
}

ToolWindow::~ToolWindow()
{
    delete ui;
}

void ToolWindow::on_pushButton_run_clicked()
{
    QString s = ui->textEdit_checksum->toPlainText();
    QByteArray temp = QByteArray::fromHex(s.toLatin1());
    DWORD checksum=0;
    for(int i=0; i<temp.size(); i++){
        checksum+=temp.at(i);
    }
    checksum&=0x000000FF;
    QString str;
    str.sprintf("%02X",checksum);
    ui->label_result->setText(str);
}
