#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFileDialog>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QString musicName, QPixmap image, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    musicNameLineEdit = ui->musicNameLineEdit;
    musicImageLabel = ui->musicImageLabel;
    changeImageButton = ui->changeImageButton;

    musicNameLineEdit->setText(musicName);
    musicImageLabel->setPixmap(image);
    currentImage = image;

    connect(changeImageButton, &QPushButton::clicked, this, &SettingsDialog::changeImage);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

QString SettingsDialog::getMusicName() const
{
    return musicNameLineEdit->text();
}

QPixmap SettingsDialog::getImage() const
{
    return currentImage;
}

void SettingsDialog::changeImage()
{
    QString imagePath = QFileDialog::getOpenFileName(this, tr("Choose Image"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (!imagePath.isEmpty()) {
        if (!imagePath.isEmpty()) {
            QPixmap pixmap(imagePath);
            if (!pixmap.isNull()) {
                currentImage = pixmap;
                musicImageLabel->setPixmap(pixmap.scaled(musicImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                QMessageBox::warning(this, "Error", "Failed to load the image.");
            }

        } else {
            currentImage = QPixmap();
        }
    }
}
