#include <iomanip>
#include <sstream>

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

#include "ui/mainwindow.hpp"
#include "engine/plotdata.hpp"
#include "version.hpp"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->actionSave->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton));

    QValidator * doubleValidator = new QDoubleValidator(this);
    ui->reminLineEdit->setValidator(doubleValidator);
    ui->remaxLineEdit->setValidator(doubleValidator);
    ui->imminLineEdit->setValidator(doubleValidator);
    ui->immaxLineEdit->setValidator(doubleValidator);
    ui->colorSlopeLineEdit->setValidator(doubleValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_drawButton_clicked()
{
    ui->actionSave->setEnabled(false);
    ui->statusBar->clearMessage();

    // read the data
    PlotData plotData;
    plotData.formula = ui->formulaLineEdit->text().toStdString();
    plotData.reMin = ui->reminLineEdit->text().toDouble();
    plotData.reMax = ui->remaxLineEdit->text().toDouble();
    plotData.imMin = ui->imminLineEdit->text().toDouble();
    plotData.imMax = ui->immaxLineEdit->text().toDouble();
    plotData.imageWidth = ui->imageWidthSpinBox->value();
    plotData.imageHeight = ui->imageHeightSpinBox->value();
    plotData.coloringMethod = ui->coloringMethodComboBox->currentIndex();
    plotData.colorSlope = ui->colorSlopeLineEdit->text().toDouble();

    // process
    RedrawInfo info;
    try
    {
        ui->plotWidget->draw(plotData, info);
    }
    catch (std::invalid_argument const & e)
    {
        std::string error_msg = std::string("Formula error: ") + e.what() + ".";
        QMessageBox::warning(this, QString("Formula error"), QString::fromStdString(error_msg));
        return;
    }


    // create info
    std::stringstream message;
    message << std::fixed << std::setprecision(2)
            << "Parsing: " << info.parsingDuration.count()
            << "s; Computing: " << info.computingDuration.count()
            << "s; Coloring: " << info.coloringDuration.count() << "s.";

    ui->statusBar->showMessage(QString::fromStdString(message.str()));
    ui->actionSave->setEnabled(true);
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
