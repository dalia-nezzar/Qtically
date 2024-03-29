#ifndef QTICALLYMAINWINDOW_H
#define QTICALLYMAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QFileDialog>
#include <QListWidget>
#include <QLabel>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class QticallyMainWindow; }
QT_END_NAMESPACE

class QticallyMainWindow : public QMainWindow
{
    Q_OBJECT

public :
    QticallyMainWindow(QWidget *parent = nullptr);
    ~QticallyMainWindow();

public slots :
    void addMusic();
    void playSelectedMusic();
    void updateMusicName(const QString &oldName, const QString &newName);
    void updateMusicImage(const QString &musicName, const QPixmap &newImage);
    void deleteSelectedMusic();
    void loadState(const QString &filename);
    void saveState(const QString &filename);
    void save();
    void open();




private:
    Ui::QticallyMainWindow *ui;
    QMediaPlayer *player;
    QListWidget *musicList;
    QSlider *musicSlider;
    QLabel *musicImageLabel;
    QLabel *timeElapsedLabel;
    QLabel *totalTimeLabel;
    QPushButton *repeatButton;
    QPushButton *shuffleButton;
    bool repeatEnabled;
    bool shuffleEnabled;
    QLabel *musicNameLabel;
    QMap<QString, QString> musicMap;
    QMap<QString, QPixmap> musicImageMap;
    QMap<QString, QPixmap> customMusicImageMap;
    QString selectedMusicName;
    QPixmap selectedMusicImage;
    int prevIndex;
    bool isPlaying;
    QMenu *contextMenu;
    QString selectedMusicImagePath;
    QPixmap defaultImage;
    QLineEdit *searchBar;
    QSystemTrayIcon *trayIcon;






private slots:
    void playMusic();
    void nextMusic();
    void previousMusic();
    void updateSliderPosition();
    void updateTimeLabels();
    void toggleRepeat();
    void toggleShuffle();
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void showSettingsDialog();
    void importPlaylist();
    void keyPressEvent(QKeyEvent *event) override;
    void sliderPressed();
    void filterMusicList();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);




protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

};


#endif // QTICALLYMAINWINDOW_H
