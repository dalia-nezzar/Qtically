#include "qticallymainwindow.h"
#include "ui_qticallymainwindow.h"
#include "settingsdialog.h"
#include <QMediaPlayer>
#include <QFileDialog>
#include <QTime>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include <QBuffer>
#include <QSystemTrayIcon>


QticallyMainWindow::QticallyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QticallyMainWindow)
    , prevIndex(-1)
    , isPlaying(false)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);
    musicList = ui->listWidget;

    connect(ui->pushButton_add_music, &QPushButton::clicked, this, &QticallyMainWindow::addMusic);
    connect(musicList, &QListWidget::itemClicked, this, &QticallyMainWindow::playSelectedMusic);

    connect(ui->pushButton_play, &QPushButton::clicked, this, &QticallyMainWindow::playMusic);
    connect(ui->pushButton_pnext, &QPushButton::clicked, this, &QticallyMainWindow::nextMusic);
    connect(ui->pushButton_previous, &QPushButton::clicked, this, &QticallyMainWindow::previousMusic);

    connect(player, &QMediaPlayer::mediaStatusChanged, this, &QticallyMainWindow::handleMediaStatusChanged);


    musicSlider = ui->slider;
    musicImageLabel = ui->musicImageLabel;
    musicSlider->setRange(0, 0);
    musicSlider->setSliderDown(false);

    connect(player, &QMediaPlayer::positionChanged, this, &QticallyMainWindow::updateSliderPosition);
    connect(player, &QMediaPlayer::durationChanged, this, &QticallyMainWindow::updateSliderPosition);
    connect(musicSlider, &QSlider::sliderMoved, this, [=](int position) {
        player->setPosition(position);
    });
    connect(musicSlider, &QSlider::sliderPressed, this, &QticallyMainWindow::sliderPressed);

    timeElapsedLabel = ui->timeElapsedLabel;
    totalTimeLabel = ui->totalTimeLabel;

    repeatButton = ui->pushButton_repeat;
    shuffleButton = ui->pushButton_shuffle;

    repeatEnabled = false;
    shuffleEnabled = false;

    connect(repeatButton, &QPushButton::clicked, this, &QticallyMainWindow::toggleRepeat);
    connect(shuffleButton, &QPushButton::clicked, this, &QticallyMainWindow::toggleShuffle);

    musicNameLabel = ui->musicNameLabel;

    defaultImage = QPixmap(":/images/images/OIP.jpeg");

    connect(ui->pushButton_edit, &QPushButton::clicked, this, &QticallyMainWindow::showSettingsDialog);

    connect(ui->pushButton_import_playlist, &QPushButton::clicked, this, &QticallyMainWindow::importPlaylist);

    contextMenu = new QMenu(this);
    contextMenu->addAction("Lire", this, &QticallyMainWindow::playSelectedMusic);
    contextMenu->addAction("Supprimer", this, &QticallyMainWindow::deleteSelectedMusic);

    musicList->installEventFilter(this);

    ui->menuParametres->addAction("Sauvegarder", this, &QticallyMainWindow::save);
    ui->menuParametres->addAction("Ouvrir", this, &QticallyMainWindow::open);

    searchBar = ui->searchBar;
    connect(searchBar, &QLineEdit::textChanged, this, &QticallyMainWindow::filterMusicList);


    // Préparation des notifications

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/images/OIP.jpeg"));
    trayIcon->setToolTip("Qtically");
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &QticallyMainWindow::iconActivated);


}

QticallyMainWindow::~QticallyMainWindow()
{

    delete ui;
}


void QticallyMainWindow::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        isPlaying = false;
        ui->pushButton_play->setIcon(QIcon(":/images/images/play.png"));
        if (repeatEnabled) {
            if (shuffleEnabled) {
                int nextIndex = QRandomGenerator::global()->bounded(musicList->count());
                musicList->setCurrentRow(nextIndex);
            }
            playSelectedMusic();
        } else {
            nextMusic();
        }
    }
}

void QticallyMainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        break;
    default:
        ;
    }
}

void QticallyMainWindow::showSettingsDialog()
{
    SettingsDialog dialog(selectedMusicName, selectedMusicImage, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newMusicName = dialog.getMusicName();
        QPixmap newImage = dialog.getImage();
        updateMusicImage(newMusicName, newImage);
        selectedMusicImage = newImage;

        updateMusicName(selectedMusicName, newMusicName);
        selectedMusicName = newMusicName;

        musicList->currentItem()->setText(newMusicName);
        musicNameLabel->setText(newMusicName);
    }
}




void QticallyMainWindow::toggleRepeat()
{
    repeatEnabled = !repeatEnabled;
}

void QticallyMainWindow::toggleShuffle()
{
    shuffleEnabled = !shuffleEnabled;
}


void QticallyMainWindow::addMusic()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Music"), "", tr("Music Files (*.mp3 *.wav)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        QString musicName = fileInfo.completeBaseName();
        QListWidgetItem* item = new QListWidgetItem(musicName);
        item->setData(Qt::UserRole, fileName);
        musicList->addItem(item);
        musicMap.insert(fileName, fileName);
        QPixmap defaultImage(":/images/images/OIP.jpeg");
        musicImageMap.insert(fileName, defaultImage);

        musicImageLabel->setPixmap(defaultImage);

        selectedMusicName = musicName;
        selectedMusicImage = defaultImage;

        qDebug() << "Music added: " << musicName;
    }
}


void QticallyMainWindow::playSelectedMusic()
{
    if (musicList->currentItem())
    {
        QString musicName = musicList->currentItem()->text();
        QString filePath = musicList->currentItem()->data(Qt::UserRole).toString();
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->play();
        musicNameLabel->setText(musicName);

        QPixmap image;
        if (customMusicImageMap.contains(musicName)) {
            image = customMusicImageMap.value(musicName);
        } else {
            image = musicImageMap.value(filePath);
        }
        musicImageLabel->setPixmap(image.scaled(musicImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        selectedMusicName = musicName;
        selectedMusicImage = image;

        isPlaying = true;
        ui->pushButton_play->setIcon(QIcon(":/images/images/pause.png"));

        ui->pushButton_edit->setEnabled(true);

        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
        trayIcon->showMessage("Qtically", "En train de jouer : " + musicName, icon, 5000);
    }
}





void QticallyMainWindow::playMusic()
{
    if (!isPlaying) {
        player->play();
        isPlaying = true;
        ui->pushButton_play->setIcon(QIcon(":/images/images/pause.png"));
    } else {
        player->pause();
        isPlaying = false;
        ui->pushButton_play->setIcon(QIcon(":/images/images/play.png"));
    }
    setFocus();
}

void QticallyMainWindow::nextMusic()
{
    int nextIndex;

    if (shuffleEnabled) {
        do {
            nextIndex = QRandomGenerator::global()->bounded(musicList->count());
        } while (nextIndex == prevIndex);
    } else {
        nextIndex = musicList->currentRow() + 1;
    }

    if (nextIndex < musicList->count()) {
        musicList->setCurrentRow(nextIndex);
        playSelectedMusic();
    }

    prevIndex = nextIndex;
}

void QticallyMainWindow::previousMusic()
{
    int prevIndex = musicList->currentRow() - 1;

    if (prevIndex >= 0) {
        musicList->setCurrentRow(prevIndex);
        playSelectedMusic();
    }
}

void QticallyMainWindow::updateTimeLabels()
{
    int currentPosition = player->position();
    int totalDuration = player->duration();

    QTime currentTime(0, 0, 0, 0);
    currentTime = currentTime.addMSecs(currentPosition);
    QString timeElapsed = currentTime.toString("mm:ss");

    QTime totalTime(0, 0, 0, 0);
    totalTime = totalTime.addMSecs(totalDuration);
    QString totalDurationStr = totalTime.toString("mm:ss");

    timeElapsedLabel->setText(timeElapsed);
    totalTimeLabel->setText(totalDurationStr);
}


void QticallyMainWindow::updateSliderPosition()
{
    if (player->duration() != 0) {
        musicSlider->setRange(0, player->duration());
    }

    if (!musicSlider->isSliderDown()) {
        musicSlider->setValue(player->position());
    }

    updateTimeLabels();
}


void QticallyMainWindow::updateMusicName(const QString &oldName, const QString &newName)
{
    for (int i = 0; i < musicList->count(); ++i) {
        QListWidgetItem *item = musicList->item(i);
        if (item->text() == oldName) {
            item->setText(newName);
            break;
        }
    }

    if (musicNameLabel->text() == oldName) {
        musicNameLabel->setText(newName);
    }

    QMap<QString, QString>::iterator it = musicMap.find(oldName);
    if (it != musicMap.end()) {
        it.value() = newName;
    }

    if (selectedMusicName == oldName) {
        selectedMusicName = newName;
    }

    if (customMusicImageMap.contains(oldName)) {
        QPixmap image = customMusicImageMap.value(oldName);
        customMusicImageMap.remove(oldName);
        customMusicImageMap.insert(newName, image);
    }

    QMap<QString, QPixmap>::iterator it2 = musicImageMap.find(oldName);
    if (it2 != musicImageMap.end()) {
        it2.value() = newName;
    }

}





void QticallyMainWindow::updateMusicImage(const QString &musicName, const QPixmap &newImage)
{
    customMusicImageMap.insert(musicName, newImage);
    musicImageLabel->setPixmap(newImage.scaled(musicImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}




void QticallyMainWindow::importPlaylist()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Playlist"), "", tr("Playlist Files (*.m3u)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            QFileInfo fileInfo(fileName);
            QString playlistDir = fileInfo.absoluteDir().absolutePath();

            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.startsWith('#'))
                {
                    QFileInfo fileInfo(line);
                    if (fileInfo.isRelative()) {
                        fileInfo.setFile(playlistDir + '/' + line);
                    }

                    if (fileInfo.exists())
                    {
                        QString musicName = fileInfo.completeBaseName();
                        QListWidgetItem* item = new QListWidgetItem(musicName);
                        item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
                        musicList->addItem(item);
                        musicMap.insert(fileInfo.absoluteFilePath(), fileInfo.absoluteFilePath());
                        QPixmap defaultImage(":/images/images/OIP.jpeg");
                        musicImageMap.insert(fileInfo.absoluteFilePath(), defaultImage);
                    }
                }
            }
            file.close();
        }
    }
}

void QticallyMainWindow::deleteSelectedMusic()
{
    if (musicList->currentItem())
    {
        QString filePath = musicList->currentItem()->data(Qt::UserRole).toString();
        player->stop();
        isPlaying = false;
        ui->pushButton_play->setIcon(QIcon(":/images/images/play.png"));
        musicNameLabel->clear();
        musicImageLabel->clear();
        ui->pushButton_edit->setEnabled(false);

        musicList->takeItem(musicList->currentRow());
        musicMap.remove(filePath);
        musicImageMap.remove(filePath);
        customMusicImageMap.remove(QFileInfo(filePath).fileName());
    }
}

bool QticallyMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == musicList && event->type() == QEvent::ContextMenu)
    {
        if (QContextMenuEvent *contextMenuEvent = dynamic_cast<QContextMenuEvent *>(event))
        {
            const QPoint &pos = contextMenuEvent->pos();
            QListWidgetItem *item = musicList->itemAt(pos);

            if (item)
            {
                musicList->setCurrentItem(item);
                contextMenu->exec(QCursor::pos());
            }

            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}


void QticallyMainWindow::saveState(const QString &filename)
{
    QJsonArray musicArray;

    for (int i = 0; i < musicList->count(); ++i)
    {
        QListWidgetItem *item = musicList->item(i);
        QJsonObject musicObject;
        musicObject["name"] = item->text();
        musicObject["filePath"] = item->data(Qt::UserRole).toString();

        // Vérifier si l'image actuelle est la même que l'image par défaut
        if (customMusicImageMap.contains(item->text()) && customMusicImageMap[item->text()] != defaultImage)
        {
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            customMusicImageMap[item->text()].save(&buffer, "PNG");
            musicObject["image"] = QJsonValue(QString(buffer.data().toBase64()));
            buffer.close();
        }

        musicArray.append(musicObject);
    }

    QJsonObject settingsObject;
    settingsObject["repeatEnabled"] = repeatEnabled;
    settingsObject["shuffleEnabled"] = shuffleEnabled;

    QJsonObject stateObject;
    stateObject["musicArray"] = musicArray;
    stateObject["settings"] = settingsObject;

    QJsonDocument jsonDoc(stateObject);

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(jsonDoc.toJson());
        file.close();
    }
    else
    {
        QMessageBox::warning(this, "Erreur", "Impossible d'enregistrer la sauvegarde.");
    }
}



void QticallyMainWindow::loadState(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (!jsonDoc.isNull() && jsonDoc.isObject())
        {
            QJsonObject stateObject = jsonDoc.object();

            QJsonArray musicArray = stateObject["musicArray"].toArray();
            musicList->clear();
            for (const QJsonValue &musicValue : musicArray)
            {
                if (musicValue.isObject())
                {
                    QJsonObject musicObject = musicValue.toObject();

                    QString musicName = musicObject["name"].toString();
                    QString filePath = musicObject["filePath"].toString();

                    QListWidgetItem *item = new QListWidgetItem(musicName);
                    item->setData(Qt::UserRole, filePath);
                    musicList->addItem(item);

                    QPixmap image;
                    if (musicObject.contains("image"))
                    {
                        QByteArray imageData = QByteArray::fromBase64(musicObject["image"].toString().toLatin1());
                        image.loadFromData(imageData);
                    }
                    else
                    {
                        image = defaultImage;
                    }
                    customMusicImageMap.insert(musicName, image);
                    musicImageMap.insert(filePath, image);
                }
            }

            QJsonObject settingsObject = stateObject["settings"].toObject();
            repeatEnabled = settingsObject["repeatEnabled"].toBool();
            shuffleEnabled = settingsObject["shuffleEnabled"].toBool();

            ui->pushButton_repeat->setChecked(repeatEnabled);
            ui->pushButton_shuffle->setChecked(shuffleEnabled);
        }
        else
        {
            QMessageBox::warning(this, "Erreur", "Le fichier de sauvegarde est invalide.");
        }
    }
    else
    {
        QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier de sauvegarde.");
    }
}

void QticallyMainWindow::save()
{
    QString defaultFileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder", defaultFileName, "Fichiers de sauvegarde (*.json)");

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
            fileName += ".json";
        }
        saveState(fileName);
    }
}

void QticallyMainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Ouvrir", "", "Fichiers de sauvegarde (*.json)");
    if (!fileName.isEmpty())
    {
        loadState(fileName);
    }
}

void QticallyMainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
        playMusic();
    } else if (event->key() == Qt::Key_Left) {
        int currentPosition = player->position();
        int newPosition = currentPosition - 10000;
        if (newPosition >= 0) {
            player->setPosition(newPosition);
        }
    } else if (event->key() == Qt::Key_Right) {
        int currentPosition = player->position();
        int newPosition = currentPosition + 10000;
        if (newPosition <= player->duration()) {
            player->setPosition(newPosition);
        }
    }
    QMainWindow::keyPressEvent(event);
}


void QticallyMainWindow::sliderPressed()
{
    int position = musicSlider->value();
    player->setPosition(position);
    updateTimeLabels();
}


void QticallyMainWindow::filterMusicList()
{
    QString filter = searchBar->text();
    for (int i = 0; i < musicList->count(); ++i)
    {
        QListWidgetItem *item = musicList->item(i);
        if (item->text().contains(filter, Qt::CaseInsensitive))
        {
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }
    }
}

