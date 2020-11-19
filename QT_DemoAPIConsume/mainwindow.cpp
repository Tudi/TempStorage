#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialoglogin.h"
#include "dialogmemberarea.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //show the login dialog
    DialogLogin *dl = new DialogLogin();
    dl->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

