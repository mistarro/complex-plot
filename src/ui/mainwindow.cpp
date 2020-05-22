#include <future>
#include <iomanip>
#include <sstream>

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>

#include "ui/mainwindow.hpp"
#include "version.hpp"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    state(State::READY),
    cancellationToken(false)
{
    ui->setupUi(this);

    ui->actionSave->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton));

    QValidator * doubleValidator = new QDoubleValidator(this);
    ui->reminLineEdit->setValidator(doubleValidator);
    ui->remaxLineEdit->setValidator(doubleValidator);
    ui->imminLineEdit->setValidator(doubleValidator);
    ui->immaxLineEdit->setValidator(doubleValidator);
    ui->colorSlopeLineEdit->setValidator(doubleValidator);

    connect(ui->plotWidget, &PlotWidget::engineThreadExited, this, &MainWindow::on_engineThreadExited_triggered);
    connect(ui->plotWidget, &PlotWidget::mouseMove, this, &MainWindow::on_plotWidget_mouseMoved);
    connect(ui->plotWidget, &PlotWidget::mouseUp, this, &MainWindow::on_plotWidget_mouseUp);
    connect(ui->plotWidget, &PlotWidget::mouseLeave, this, &MainWindow::on_plotWidget_mouseLeft);

    readPlotData();
    ui->plotWidget->clear(plotData);

    ui->statusBar->showMessage("Ready");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_drawButton_clicked()
{
    switch (state)
    {
    case State::READY:
        draw();
        break;
    case State::BUSY:
        cancel();
        break;
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, QString(), QString(), QString("Images (*.png)"));
    if (path.isNull())
        return;
    if (ui->plotWidget->saveImage(path))
        return;
    QMessageBox::warning(this, QString("Warning"), QString("Image has not been saved (wrong extension)."));
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, QString("About complex-plot"), QString("complex-plot " VERSION_STRING));
}

void MainWindow::on_engineThreadExited_triggered()
{
    ui->drawButton->setText("Draw");
    state = State::READY;
    RedrawInfo info = engineFuture.get();
    if (info.status == RedrawInfo::Status::FINISHED)
    {
        ui->plotWidget->repaint();

        std::stringstream message;
        message << std::fixed << std::setprecision(2)
                << "Parsing: " << info.parsingDuration.count()
                << "s; Computing: " << info.computingDuration.count()
                << "s; Coloring: " << info.coloringDuration.count() << "s.";

        ui->actionSave->setEnabled(true);
        ui->statusBar->showMessage(QString::fromStdString(message.str()));

        return;
    }

    ui->plotWidget->clear(plotData);

    if (info.status == RedrawInfo::Status::ERROR)
    {
        QMessageBox::warning(this, QString("Error"), QString::fromStdString(info.message));
        ui->statusBar->showMessage("Ready");
        return;
    }

    if (info.status == RedrawInfo::Status::CANCELLED)
        ui->statusBar->showMessage("Cancelled");
}

void MainWindow::readPlotData()
{
    plotData.formula = ui->formulaLineEdit->text().toStdString();
    plotData.reMin = ui->reminLineEdit->text().toDouble();
    plotData.reMax = ui->remaxLineEdit->text().toDouble();
    plotData.imMin = ui->imminLineEdit->text().toDouble();
    plotData.imMax = ui->immaxLineEdit->text().toDouble();
    plotData.reSeed = ui->reSeedLineEdit->text().toDouble();
    plotData.imSeed = ui->imSeedLineEdit->text().toDouble();
    plotData.imageWidth = ui->imageWidthSpinBox->value();
    plotData.imageHeight = ui->imageHeightSpinBox->value();
    plotData.coloringMethod = ui->coloringMethodComboBox->currentIndex();
    plotData.colorSlope = ui->colorSlopeLineEdit->text().toDouble();
}

void MainWindow::draw()
{
    cancellationToken = false;
    ui->actionSave->setEnabled(false);
    ui->statusBar->showMessage("Drawing...");
    ui->drawButton->setText("Cancel");
    state = State::BUSY;

    readPlotData();

    // process
    engineFuture = ui->plotWidget->draw(plotData, cancellationToken);
}

void MainWindow::cancel()
{
    cancellationToken = true;
    ui->statusBar->showMessage("Cancelling...");
}

void MainWindow::on_plotWidget_mouseMoved(QMouseEvent * event)
{
    double re, im;
    plotData.image2complex(event->x(), event->y(), re, im);
    std::stringstream message;
    message << "Mouse: (" << re << ", " << im << ")";

    ui->statusBar->showMessage(QString::fromStdString(message.str()));
}

void MainWindow::on_plotWidget_mouseUp(QMouseEvent * event)
{
    double re, im;
    plotData.image2complex(event->x(), event->y(), re, im);
    ui->reSeedLineEdit->setText(QString::number(re));
    ui->imSeedLineEdit->setText(QString::number(im));
}

void MainWindow::on_plotWidget_mouseLeft()
{
    ui->statusBar->showMessage("Ready");
}
