#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QEvent>
#include <doublewindow.h>
#include <mcuwindow.h>
#include <toolwindow.h>
#include <about.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DoubleWindow *doubleWindow;
    McuWindow *mcuWindow;
    ToolWindow *toolWindow;
    About *about;

protected:
    void keyPressEvent(QKeyEvent *);
private slots:
    void on_tabWidget_currentChanged(int index);
};

#endif // MAINWINDOW_H
