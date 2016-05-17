#ifndef MCUSETTINGWINDOW_H
#define MCUSETTINGWINDOW_H

#include <QWidget>
#include <xpgtool.h>

namespace Ui {
class McuSettingWindow;
}

class McuSettingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit McuSettingWindow(XpgTool *xpgtool, QWidget *parent = 0);
    ~McuSettingWindow();
    void initData();

private slots:
    void on_pushButton_save_clicked();

private:
    Ui::McuSettingWindow *ui;
    XpgTool *xpgTool;
};

#endif // MCUSETTINGWINDOW_H
