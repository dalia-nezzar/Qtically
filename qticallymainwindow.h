#ifndef QTICALLYMAINWINDOW_H
#define QTICALLYMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class QticallyMainWindow; }
QT_END_NAMESPACE

class QticallyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QticallyMainWindow(QWidget *parent = nullptr);
    ~QticallyMainWindow();

private:
    Ui::QticallyMainWindow *ui;
};
#endif // QTICALLYMAINWINDOW_H
