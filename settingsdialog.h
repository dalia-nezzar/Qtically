#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QString musicName, QPixmap image, QWidget *parent = nullptr);
    ~SettingsDialog();

    QString getMusicName() const;
    QPixmap getImage() const;

private slots:
    void changeImage();

private:
    Ui::SettingsDialog *ui;
    QLineEdit *musicNameLineEdit;
    QLabel *musicImageLabel;
    QPushButton *changeImageButton;
    QPixmap currentImage;
};

#endif // SETTINGSDIALOG_H
