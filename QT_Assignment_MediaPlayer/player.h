#ifndef PLAYER_H
#define PLAYER_H

//#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
//#include <QSqlDatabase>
#include <QSqlQueryModel>

#include <QtWidgets>
#include <QSqlQuery>


QT_BEGIN_NAMESPACE
class QAbstractItemView;
class QLabel;
class QMediaPlayer;
class QModelIndex;
class QPushButton;
class QSlider;
class QStatusBar;
class QVideoProbe;
class QVideoWidget;
class QAudioProbe;
QT_END_NAMESPACE

class PlaylistModel;

class Player : public QWidget
{
    Q_OBJECT

public:
    explicit Player(QWidget *parent = nullptr);
    ~Player();
    QSqlDatabase mydb;
    void connClose()
    {
        mydb.close();
        mydb.removeDatabase(QSqlDatabase::defaultConnection);
        qDebug()<<"connection closed";
    }
    bool connOpen()
    {
        mydb = QSqlDatabase::addDatabase("QSQLITE");
        mydb.setDatabaseName("C:/Users/ezhil/Desktop/db/mydb.db");

        if(!mydb.open()){
            qDebug()<<"Failed to open database!";
            return false;
        }
        else{
            qDebug()<<"Opened db successfully...";
            return true;
        }
    }

    bool isPlayerAvailable() const;
    void addToPlaylist(const QList<QUrl> &urls);

signals:
    void fullScreenChanged(bool fullScreen);

private slots:
    void open();
    void removefromPlaylist();
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void seekbackwardsClicked();
    void seekforwardsClicked();
    void previousClicked();
    void seek(int seconds);
    void jump(const QModelIndex &index);
    void jumptoFav(const QModelIndex &index);
    void playlistPositionChanged(int);
    void stateChanged(QMediaPlayer::State state);
    void videoAvailableChanged(bool available);
    void addtoFav();
    void removefromFav();

private:  

    void updateDurationInfo(qint64 currentInfo);

    QMediaPlayer *m_player = nullptr;
    QMediaPlaylist *m_playlist = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QLabel *m_coverLabel = nullptr;
    QSlider *m_slider = nullptr;
    QLabel *m_labelDuration = nullptr;
    QPushButton *m_fullScreenButton = nullptr;
    QPushButton *m_favButton = nullptr;
    QPushButton *m_dislikeButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QStatusBar *m_statusBar = nullptr;
    PlaylistModel *m_playlistModel = nullptr;
    QSqlQueryModel *model =nullptr;
    QAbstractItemView *m_playlistView = nullptr;
    QAbstractItemView *m_favlistView = nullptr;
    QString m_trackInfo;
    QString m_statusInfo;
    qint64 m_duration;
};

#endif // PLAYER_H
