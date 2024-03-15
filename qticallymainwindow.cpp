#include "qticallymainwindow.h"
#include "ui_qticallymainwindow.h"

QticallyMainWindow::QticallyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QticallyMainWindow)
{
    ui->setupUi(this);
}

QticallyMainWindow::~QticallyMainWindow()
{
    delete ui;
}

