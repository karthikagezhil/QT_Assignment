#include "player.h"
#include "playercontrols.h"
#include "playlistmodel.h"
#include "videowidget.h"

Player::Player(QWidget *parent)
    : QWidget(parent)
{
//! [create-objs]

    m_player = new QMediaPlayer(this);
    // owned by PlaylistModel
    m_playlist = new QMediaPlaylist();
    m_player->setPlaylist(m_playlist);
//! [create-objs]

    connect(m_player, &QMediaPlayer::durationChanged, this, &Player::durationChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    connect(m_playlist, &QMediaPlaylist::currentIndexChanged, this, &Player::playlistPositionChanged);
    connect(m_player, &QMediaPlayer::videoAvailableChanged, this, &Player::videoAvailableChanged);//full screen
    connect(m_player, &QMediaPlayer::stateChanged, this, &Player::stateChanged);

//! [2]
    m_videoWidget = new VideoWidget(this);
    m_player->setVideoOutput(m_videoWidget);
    m_videoWidget->setToolTip("video widget");

    m_playlistModel = new PlaylistModel(this);
    m_playlistModel->setPlaylist(m_playlist);
//! [2]

    m_playlistView = new QListView(this);
    m_playlistView->setModel(m_playlistModel);
    m_playlistView->setCurrentIndex(m_playlistModel->index(m_playlist->currentIndex(), 0));
    m_playlistView->setToolTip("playlist");

//Database playlist
    m_favlistView = new QListView(this);
    m_favlistView->setToolTip("user preference stored in Database, select any to add to playlist");
    model = new QSqlQueryModel();
    if(!connOpen())
        connOpen();
/*
        QSqlQuery *create = new QSqlQuery(mydb);
        bool run = create->prepare("CREATE TABLE IF NOT EXISTS C:/Users/ezhil/Desktop/db/mydb.db.m_player (media TEXT PRIMARY KEY)");
        if(!run)
        qDebug()<<"Table already there or some error";

        else
        qDebug()<<"Table mediaplayer created";
*/
        //create a table named favorites in SQLITE3, change database path specified in player.h
        //CREATE TABLE IF NOT EXISTS mediaplayer (media TEXT PRIMARY KEY);
    QSqlQuery *qry = new QSqlQuery(mydb);
    qry->prepare("select * from mediaplayer");
    bool run = qry->exec();
    if(!run)
        qDebug()<<"query failed";
    else{
        qDebug()<<"query run successfully";
        model->setQuery(*qry);
        m_favlistView->setModel(model);
    }
    connClose();

    connect(m_playlistView, &QAbstractItemView::activated, this, &Player::jump);
    connect(m_favlistView, &QAbstractItemView::activated, this, &Player::jumptoFav);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, m_player->duration() / 1000);
    m_slider->setToolTip("position media");

    m_labelDuration = new QLabel(this);
    connect(m_slider, &QSlider::sliderMoved, this, &Player::seek);

    QPushButton *openButton = new QPushButton(tr("Open"), this);
    openButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    openButton->setToolTip("open files to put in playlist");

    connect(openButton, &QPushButton::clicked, this, &Player::open);

    QPushButton *remButton = new QPushButton(tr("Remove"), this);
    remButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    remButton->setToolTip("remove from playlist");

    connect(remButton, &QPushButton::clicked, this, &Player::removefromPlaylist);


    PlayerControls *controls = new PlayerControls(this);
    controls->setState(m_player->state());
    controls->setVolume(m_player->volume());
    controls->setMuted(controls->isMuted());

    connect(controls, &PlayerControls::play, m_player, &QMediaPlayer::play);
    connect(controls, &PlayerControls::pause, m_player, &QMediaPlayer::pause);
    connect(controls, &PlayerControls::stop, m_player, &QMediaPlayer::stop);
    connect(controls, &PlayerControls::next, m_playlist, &QMediaPlaylist::next);
    connect(controls, &PlayerControls::previous, this, &Player::previousClicked);
    connect(controls, &PlayerControls::seekbackwards, this, &Player::seekbackwardsClicked);
    connect(controls, &PlayerControls::seekforwards, this, &Player::seekforwardsClicked);
    connect(controls, &PlayerControls::changeVolume, m_player, &QMediaPlayer::setVolume);
    connect(controls, &PlayerControls::changeMuting, m_player, &QMediaPlayer::setMuted);
    connect(controls, &PlayerControls::changeRate, m_player, &QMediaPlayer::setPlaybackRate);
    connect(controls, &PlayerControls::stop, m_videoWidget, QOverload<>::of(&QVideoWidget::update));

    connect(m_player, &QMediaPlayer::stateChanged, controls, &PlayerControls::setState);
    connect(m_player, &QMediaPlayer::volumeChanged, controls, &PlayerControls::setVolume);
    connect(m_player, &QMediaPlayer::mutedChanged, controls, &PlayerControls::setMuted);

    m_fullScreenButton = new QPushButton(tr("FullScreen"), this);
    m_fullScreenButton->setCheckable(true);
    m_fullScreenButton->setToolTip("full screen only when media is playing");

    m_favButton = new QPushButton(tr("Like"), this);
    m_favButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_favButton->setToolTip("save to Database");

    connect(m_favButton, &QPushButton::clicked, this, &Player::addtoFav);

    m_dislikeButton = new QPushButton(tr("Dislike"), this);
    m_dislikeButton->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
    m_dislikeButton->setToolTip("remove from Database");

    connect(m_dislikeButton, &QPushButton::clicked, this, &Player::removefromFav);

    QBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(m_videoWidget, 2);
    displayLayout->addWidget(m_playlistView);
    displayLayout->addWidget(m_favlistView);//for database
    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(remButton);
    controlLayout->addStretch(1);
    controlLayout->addWidget(controls);
    controlLayout->addStretch(1);
    controlLayout->addWidget(m_fullScreenButton);
    controlLayout->addWidget(m_favButton);
    controlLayout->addWidget(m_dislikeButton);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(displayLayout);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_slider);
    hLayout->addWidget(m_labelDuration);
    layout->addLayout(hLayout);
    layout->addLayout(controlLayout);

    setLayout(layout);

    if (!isPlayerAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        m_playlistView->setEnabled(false);
        openButton->setEnabled(false);
        remButton->setEnabled(false);
        m_fullScreenButton->setEnabled(false);
        m_favButton->setEnabled(false);
        m_dislikeButton->setEnabled(false);
    }
}

Player::~Player()
{
    connClose();
}

bool Player::isPlayerAvailable() const
{
    return m_player->isAvailable();
}

void Player::open()
{

    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Files"));
    QStringList supportedMimeTypes = m_player->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty()) {
        supportedMimeTypes.append("audio/x-m3u"); // MP3 playlists
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        addToPlaylist(fileDialog.selectedUrls());
}
void Player::removefromPlaylist()
{
    m_playlist->removeMedia(m_playlistView->currentIndex().row());
}
static bool isPlaylist(const QUrl &url) // Check for ".m3u" playlists.
{
    if (!url.isLocalFile())
        return false;
    const QFileInfo fileInfo(url.toLocalFile());

    return fileInfo.exists() && !fileInfo.suffix().compare(QLatin1String("m3u"), Qt::CaseInsensitive);
}

void Player::addToPlaylist(const QList<QUrl> &urls)
{
    for (auto &url: urls) {
        if (isPlaylist(url))
            m_playlist->load(url);
        else{
            m_playlist->addMedia(url);
        }
    }
}

void Player::durationChanged(qint64 duration)
{
    m_duration = duration / 1000;
    m_slider->setMaximum(m_duration);
}

void Player::positionChanged(qint64 progress)
{
    if (!m_slider->isSliderDown())
        m_slider->setValue(progress / 1000);

    updateDurationInfo(progress / 1000);
}

void Player::previousClicked()
{
    // Go to previous track if we are within the first 5 seconds of playback
    // Otherwise, seek to the beginning.
    if (m_player->position() <= 5000)
        m_playlist->previous();
    else
        m_player->setPosition(0);
}

void Player::seekbackwardsClicked()
{
    m_player->setPosition(m_player->position()-10000);
}

void Player::seekforwardsClicked()
{
    m_player->setPosition(m_player->position()+10000);
}

void Player::jump(const QModelIndex &index)
{
    qDebug()<<"media count"<<m_playlist->mediaCount();

    if (index.isValid()) {
        m_playlist->setCurrentIndex(index.row());
        m_player->play();
    }
}


void Player::jumptoFav(const QModelIndex &index)//data in database view
{
    QString filename;
    if (index.isValid()){
        filename = QVariant(index.data(2)).toString();
        qDebug()<<"filename"<<filename;
        m_playlist->addMedia(QUrl::fromLocalFile(filename));
    }
}

void Player::playlistPositionChanged(int currentItem)
{
    m_playlistView->setCurrentIndex(m_playlistModel->index(currentItem, 0));
}

void Player::seek(int seconds)
{
    m_player->setPosition(seconds * 1000);
}

void Player::stateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState)
        qDebug()<<"";
}

void Player::videoAvailableChanged(bool available)
{
    if (!available) {
        disconnect(m_fullScreenButton, &QPushButton::clicked, m_videoWidget, &QVideoWidget::setFullScreen);
        disconnect(m_videoWidget, &QVideoWidget::fullScreenChanged, m_fullScreenButton, &QPushButton::setChecked);
        m_videoWidget->setFullScreen(false);
    } else {
        connect(m_fullScreenButton, &QPushButton::clicked, m_videoWidget, &QVideoWidget::setFullScreen);
        connect(m_videoWidget, &QVideoWidget::fullScreenChanged, m_fullScreenButton, &QPushButton::setChecked);

        if (m_fullScreenButton->isChecked())
            m_videoWidget->setFullScreen(true);
    }
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || m_duration) {

        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
            currentInfo % 60, (currentInfo * 1000) % 1000);
        QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60,
            m_duration % 60, (m_duration * 1000) % 1000);
        QString format = "mm:ss";
        if (m_duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    m_labelDuration->setText(tStr);
}

void Player::addtoFav()
{

    qDebug()<<"m_playlist->currentIndex()"<<m_playlist->currentIndex();
    qDebug()<<"m_playlistView->currentIndex()"<<m_playlistView->currentIndex();
    qDebug()<<m_playlistView->currentIndex().data();
    QUrl location = m_playlist->currentMedia().canonicalUrl();
    qDebug()<<"location "<< location;
    QString filename;
    connOpen();
    QSqlQuery qry;
    filename = location.toLocalFile();
    qDebug()<<"file name taken to save ****"<<filename;
    qry.prepare("insert into mediaplayer (media) values ('"+filename+"')");
    if(qry.exec())
    {
        qDebug()<<"SAVED into DB";
    }
    else{
        qDebug()<<"SAVE failed";
    }
    connClose();
}

void Player::removefromFav()
{
    qDebug()<<"m_playlist->currentIndex()"<<m_playlist->currentIndex();
    qDebug()<<"m_playlistView->currentIndex()"<<m_playlistView->currentIndex();
    qDebug()<<m_playlistView->currentIndex().data();
    QUrl location = m_playlist->currentMedia().canonicalUrl();
    qDebug()<<"location "<< location;
    QString filename;
    connOpen();
    QSqlQuery qry;
    filename = location.toLocalFile();
    qDebug()<<"file name taken to save ****"<<filename;
    qry.prepare("delete from mediaplayer where media = '"+filename+"'");
    if(qry.exec())
    {
        qDebug()<<"DELETED from DB";
    }
    else{
        qDebug()<<"DELETE failed, maybe it was not even there in DB";
    }
    connClose();
}
