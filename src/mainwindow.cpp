#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "plotdata.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
    // read the data
    PlotData plotData;
    plotData.formula = ui->formulaLineEdit->text();
    plotData.reMin = ui->reminLineEdit->text().toDouble();
    plotData.reMax = ui->remaxLineEdit->text().toDouble();
    plotData.imMin = ui->imminLineEdit->text().toDouble();
    plotData.imMax = ui->immaxLineEdit->text().toDouble();
    plotData.imageWidth = ui->imageWidthSpinBox->value();
    plotData.imageHeight = ui->imageHeightSpinBox->value();
    plotData.coloringMethod = ui->coloringMethodComboBox->currentIndex();
    plotData.colorSlope = ui->colorSlopeLineEdit->text().toDouble();

    // process
    ui->plotWidget->setFixedSize(plotData.imageWidth, plotData.imageHeight);
}
