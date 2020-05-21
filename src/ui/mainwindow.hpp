#ifndef COMPLEXPLOT_MAINWINDOW_H
#define COMPLEXPLOT_MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_drawButton_clicked();
    void on_actionSave_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();

private:
    Ui::MainWindow * ui;
};

#endif // COMPLEXPLOT_MAINWINDOW_H
