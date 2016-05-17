#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    InitUi();
}

About::~About()
{
    delete ui;
}

void About::InitUi(){
    QString background_url = qApp->applicationDirPath()+ "/image/background.png";
    ui->label_background->setStyleSheet("border-image:url(" + background_url + ")");
    QString logo_url = qApp->applicationDirPath()+ "/image/logo.png";
    ui->label_logo->setStyleSheet("border-image:url(" + logo_url + ")");
}
