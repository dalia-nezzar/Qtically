#include "qticallymainwindow.h"
#include "ui_qticallymainwindow.h"
#include "settingsdialog.h"
#include <QMediaPlayer>
#include <QFileDialog>
#include <QTime>
#include <QRandomGenerator>
#include <QContextMenuEvent>

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

    timeElapsedLabel = ui->timeElapsedLabel;
    totalTimeLabel = ui->totalTimeLabel;

    repeatButton = ui->pushButton_repeat;
    shuffleButton = ui->pushButton_shuffle;

    repeatEnabled = false;
    shuffleEnabled = false;

    connect(repeatButton, &QPushButton::clicked, this, &QticallyMainWindow::toggleRepeat);
    connect(shuffleButton, &QPushButton::clicked, this, &QticallyMainWindow::toggleShuffle);

    musicNameLabel = ui->musicNameLabel;

    connect(ui->pushButton_edit, &QPushButton::clicked, this, &QticallyMainWindow::showSettingsDialog);

    connect(ui->pushButton_import_playlist, &QPushButton::clicked, this, &QticallyMainWindow::importPlaylist);

    contextMenu = new QMenu(this);
    contextMenu->addAction("Lire", this, &QticallyMainWindow::playSelectedMusic);
    contextMenu->addAction("Supprimer", this, &QticallyMainWindow::deleteSelectedMusic);

    musicList->installEventFilter(this);



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

void QticallyMainWindow::showSettingsDialog()
{
    SettingsDialog dialog(selectedMusicName, selectedMusicImage, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newMusicName = dialog.getMusicName();
        QPixmap newImage = dialog.getImage();

        updateMusicName(selectedMusicName, newMusicName);
        updateMusicImage(newMusicName, newImage);

        selectedMusicName = newMusicName;
        selectedMusicImage = newImage;


        musicList->currentItem()->setText(newMusicName);
        selectedMusicName = newMusicName;
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
        QString musicName = QFileInfo(fileName).baseName();
        QListWidgetItem* item = new QListWidgetItem(musicName);
        item->setData(Qt::UserRole, fileName);
        musicList->addItem(item);
        musicMap.insert(fileName, fileName);
        QPixmap defaultImage(":/images/images/OIP.jpeg");
        musicImageMap.insert(fileName, defaultImage);

        musicImageLabel->setPixmap(defaultImage);

        selectedMusicName = musicName;
        selectedMusicImage = defaultImage;
    }
}



void QticallyMainWindow::playSelectedMusic()
{
    if (musicList->currentItem())
    {
        QString filePath = musicList->currentItem()->data(Qt::UserRole).toString();
        QFileInfo fileInfo(filePath);
        QString musicName = fileInfo.fileName();
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->play();
        musicNameLabel->setText(QFileInfo(musicName).completeBaseName());

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

    QMap<QString, QString>::iterator it = musicMap.begin();
    while (it != musicMap.end()) {
        if (it.value().endsWith(oldName)) {
            QString newFilePath = it.value();
            newFilePath.replace(oldName, newName);
            it.value() = newFilePath;
            ++it;
        } else {
            ++it;
        }
    }

    QMap<QString, QPixmap>::iterator it2 = musicImageMap.begin();
    while (it2 != musicImageMap.end()) {
        if (it2.key().endsWith(oldName)) {
            QString newFilePath = it2.key();
            newFilePath.replace(oldName, newName);
            it2.value() = musicImageMap.value(newFilePath);
            ++it2;
        } else {
            ++it2;
        }
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
                        QString musicName = fileInfo.baseName();
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
