#ifndef TOOLWINDOW_H
#define TOOLWINDOW_H

#include <QWidget>

#define DWORD unsigned long

namespace Ui {
class ToolWindow;
}

class ToolWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ToolWindow(QWidget *parent = 0);
    ~ToolWindow();

private slots:
    void on_pushButton_run_clicked();

private:
    Ui::ToolWindow *ui;
};

#endif // TOOLWINDOW_H
