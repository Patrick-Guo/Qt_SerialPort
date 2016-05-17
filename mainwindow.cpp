#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::setWindowTitle("机智云串口调试助手V2.1.2");
    MainWindow::setFixedSize(600,600);
    ui->tabWidget->resize(600,600);

    doubleWindow = new DoubleWindow(ui->doubleWindow);
    mcuWindow = new McuWindow(ui->mcuWindow);
    toolWindow = new ToolWindow(ui->toolWindow);
    about = new About(ui->aboutWindow);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//按键操作
void MainWindow::keyPressEvent(QKeyEvent *event){
    switch(event->key()){
    case Qt::Key_F5:
        doubleWindow->CleanRec();
        mcuWindow->CleanRec();
        break;
    case Qt::Key_Escape:
        close();
        break;
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 1){
        MainWindow::setFixedSize(950,600);
        ui->tabWidget->resize(950, 600);
        doubleWindow->ResetWindow();
        mcuWindow->CheckKey();
    }else{
        MainWindow::setFixedSize(600,600);
        ui->tabWidget->resize(600, 600);
        mcuWindow->ResetWindow();
    }
}
