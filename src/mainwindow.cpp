#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QValidator * doubleValidator = new QDoubleValidator(this);
    ui->reminLineEdit->setValidator(doubleValidator);
    ui->remaxLineEdit->setValidator(doubleValidator);
    ui->imminLineEdit->setValidator(doubleValidator);
    ui->immaxLineEdit->setValidator(doubleValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_drawButton_clicked()
{
    int width = ui->imageWidthSpinBox->value();
    int height = ui->imageHeightSpinBox->value();
    ui->imageWidget->setFixedSize(width, height);
}
