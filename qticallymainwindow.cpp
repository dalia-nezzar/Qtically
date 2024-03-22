#include "qticallymainwindow.h"
#include "ui_qticallymainwindow.h"
#include <QMediaPlayer>
#include <QFileDialog>
#include <QTime>
#include <QRandomGenerator>

QticallyMainWindow::QticallyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QticallyMainWindow)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);
    musicList = ui->listWidget;

    connect(ui->pushButton_add_music, &QPushButton::clicked, this, &QticallyMainWindow::addMusic);
    connect(musicList, &QListWidget::itemClicked, this, &QticallyMainWindow::playSelectedMusic);

    connect(ui->pushButton_play, &QPushButton::clicked, this, &QticallyMainWindow::playMusic);
    connect(ui->pushButton_pause, &QPushButton::clicked, this, &QticallyMainWindow::pauseMusic);
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


    connect(ui->pushButton_changeImage, &QPushButton::clicked, this, &QticallyMainWindow::changeMusicImage);

    timeElapsedLabel = ui->timeElapsedLabel;
    totalTimeLabel = ui->totalTimeLabel;

    repeatButton = ui->pushButton_repeat;
    shuffleButton = ui->pushButton_shuffle;

    repeatEnabled = false;
    shuffleEnabled = false;

    connect(repeatButton, &QPushButton::clicked, this, &QticallyMainWindow::toggleRepeat);
    connect(shuffleButton, &QPushButton::clicked, this, &QticallyMainWindow::toggleShuffle);

    musicNameLabel = ui->musicNameLabel;


}

QticallyMainWindow::~QticallyMainWindow()
{

    delete ui;
}


void QticallyMainWindow::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
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
        musicList->addItem(musicName);
        QMap<QString, QString>::iterator it = musicMap.find(musicName);
        if (it == musicMap.end()) {
            musicMap.insert(musicName, fileName);
        }
        musicImageLabel->setPixmap(QPixmap(":/images/default_image.png"));
    }
}

void QticallyMainWindow::playSelectedMusic()
{
    if (musicList->currentItem())
    {
        QString musicName = musicList->currentItem()->text();
        QString filePath = musicMap.value(musicName);
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->play();
        musicNameLabel->setText(musicName);
    }
}


void QticallyMainWindow::playMusic()
{
    if (player->state() != QMediaPlayer::PlayingState) {
        player->play();
    }
}

void QticallyMainWindow::pauseMusic()
{
    if (player->state() == QMediaPlayer::PlayingState) {
        player->pause();
    }
}

void QticallyMainWindow::nextMusic()
{
    int nextIndex;

    if (shuffleEnabled) {
        nextIndex = QRandomGenerator::global()->bounded(musicList->count());
    } else {
        nextIndex = musicList->currentRow() + 1;
    }

    if (nextIndex < musicList->count()) {
        musicList->setCurrentRow(nextIndex);
        playSelectedMusic();
    }
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




void QticallyMainWindow::changeMusicImage()
{
    QString imagePath = QFileDialog::getOpenFileName(this, tr("Choose Image"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        musicImageLabel->setPixmap(pixmap.scaled(musicImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
