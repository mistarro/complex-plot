#ifndef COMPLEXPLOT_MAINWINDOW_H
#define COMPLEXPLOT_MAINWINDOW_H

#include <future>

#include <QMainWindow>

#include "engine/plotdata.hpp"

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
    void on_engineThreadExited_triggered();
    void on_plotWidget_mouseMoved(QMouseEvent * event);
    void on_plotWidget_mouseLeft();

private:
    Ui::MainWindow * ui;

    enum class State { READY, BUSY };

    State state;
    std::atomic_bool cancellationToken;

    PlotData plotData;
    std::future<RedrawInfo> engineFuture;

    void readPlotData();
    void draw();
    void cancel();
};

#endif // COMPLEXPLOT_MAINWINDOW_H
