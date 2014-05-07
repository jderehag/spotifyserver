#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/applog.h"
#include <QCheckBox>

class PlaylistsModelItem : public QTreeWidgetItem
{
private:
    MediaBaseInfo m;
public:
    PlaylistsModelItem( const MediaBaseInfo& m_ ) : m(m_) {}
    const std::string& getLink() { return m.getLink(); }
    virtual QVariant data ( int column, int role ) const { if ( role != Qt::DisplayRole || column != 0 ) return QVariant(); else return QVariant( m.getName().c_str() ); }
};


MainWindow::MainWindow( QString& title, MediaInterface& m, EndpointCtrlInterface& epMgr, QWidget *parent ) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        m_(m), epMgr_(epMgr),
        progress_(0), isPlaying(false)
{
    ui->setupUi(this);
    setWindowTitle( title );
    ui->playlistsTree->setColumnCount( 1 );
    ui->playlistsTree->header()->hide();
    tracksModel.setHeaderData( 0, Qt::Horizontal, QVariant(QString("Name")));
    ui->tableView->setModel( &tracksModel );

    ui->endpointsScrollAreaLayout->setAlignment( Qt::AlignTop );

    ui->tableView->verticalHeader()->hide();
    ui->label->setScaledContents( true );

    ui->playButton->setToolTip(tr("Play"));
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    ui->nextButton->setToolTip(tr("Next"));
    ui->nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    ui->prevButton->setToolTip(tr("Previous"));
    ui->prevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    connect(&progressTimer, SIGNAL(timeout()), this, SLOT(progressUpdate()));

    m_.registerForCallbacks( *this );
    epMgr_.registerForCallbacks( *this );

}

MainWindow::~MainWindow()
{
    m_.unRegisterForCallbacks( *this );
    epMgr_.unRegisterForCallbacks( *this );
    delete ui;
}


void MainWindow::on_playButton_clicked()
{
    if ( isPlaying )
        m_.pause();
    else
        m_.resume();
}

void MainWindow::on_prevButton_clicked()
{
    m_.previous();
}

void MainWindow::on_nextButton_clicked()
{
    m_.next();
}

void MainWindow::on_playlistsTree_itemClicked(QTreeWidgetItem *item, int column)
{
    PlaylistsModelItem* pitem = (PlaylistsModelItem*)item;
    const std::string& link = pitem->getLink();
    if ( !link.empty() )
        m_.getTracks( link, this, NULL );

    ui->stackedWidget->setCurrentWidget( ui->playlistPage );
}


void MainWindow::on_playlistsTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    PlaylistsModelItem* pitem = (PlaylistsModelItem*)item;
    const std::string& link = pitem->getLink();
    if ( !link.empty() )
        m_.play( link, this, NULL );

    ui->stackedWidget->setCurrentWidget( ui->playlistPage );
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    PlaylistsModelItem* item = (PlaylistsModelItem*)ui->playlistsTree->currentItem();
    Track* t = static_cast<Track*>(index.internalPointer());
    m_.play( item->getLink(), t->getIndex(), this, NULL );
}

void MainWindow::on_actionShowEndpoints_triggered()
{
    ui->stackedWidget->setCurrentWidget( ui->endpointsPage );
    epMgr_.getAudioEndpoints( this, NULL );
}

void MainWindow::endpointCheckbox_clicked(bool checked)
{
    QCheckBox* cb = (QCheckBox*) QObject::sender();
    if ( checked )
        epMgr_.addAudioEndpoint( cb->text().toStdString(), this, NULL );
    else
        epMgr_.removeAudioEndpoint( cb->text().toStdString(), this, NULL );
}

void MainWindow::on_repeatButton_clicked(bool checked)
{
    m_.setRepeat( checked );
}

void MainWindow::on_shuffleButton_clicked(bool checked)
{
    m_.setShuffle( checked );
}

void MainWindow::updateGui()
{
    if ( isPlaying )
    {
        ui->playButton->setToolTip(tr("Pause"));
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        progressTimer.start( 1000 );
    }
    else
    {
        ui->playButton->setToolTip(tr("Play"));
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        progressTimer.stop();
    }
}

void MainWindow::updateEndpointsGui()
{

    QLayoutItem *child;
    while ((child = ui->endpointsScrollAreaLayout->takeAt(0)) != 0)
    {
        delete child->widget();
        delete child;
    }

    AudioEndpointInfoList::const_iterator it = endpoints_.begin();
    for ( ; it != endpoints_.end(); it++ )
    {
        QCheckBox* ep = new QCheckBox();
        ep->setText( QString::fromStdString( (*it).id ) );
        ep->setChecked( (*it).active );
        connect( ep, SIGNAL(clicked(bool)), this, SLOT(endpointCheckbox_clicked(bool)));
        ui->endpointsScrollAreaLayout->addWidget( ep );
    }
}

QString toTextLabel( unsigned int sec )
{
    unsigned int minutes = sec / 60;
    unsigned int seconds = sec % 60;
    std::stringstream out;
    out << (minutes < 10 ? " " : "") << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
    return QString( out.str().c_str() );
}

void MainWindow::progressUpdate()
{
    progress_++;
    ui->progressLabel->setText( toTextLabel(progress_) );
    ui->progressBar->setValue( progress_ );
}


void MainWindow::rootFolderUpdatedInd()
{
    m_.getPlaylists( this, NULL );
}
void MainWindow::playlistUpdatedInd( const std::string& link )
{
    PlaylistsModelItem* item = (PlaylistsModelItem*)ui->playlistsTree->currentItem();
    if ( item && item->getLink().compare( link ) == 0 )
    {
        m_.getTracks( link, this, NULL );
    }
}

void MainWindow::connectionState( bool up )
{
    if ( up )
    {
        m_.getPlaylists( this, NULL );
        m_.getStatus( this, NULL );
    }
}

static void addFolder( QTreeWidgetItem* parent, const Folder& folder )
{
    FolderContainer::const_iterator fit = folder.getFolders().begin();
    for ( ; fit != folder.getFolders().end(); fit++ )
    {
        PlaylistsModelItem *item = new PlaylistsModelItem(*fit);
        addFolder( item, *fit );
        parent->addChild( item );
    }

    PlaylistContainer::const_iterator pit = folder.getPlaylists().begin();
    for ( ; pit != folder.getPlaylists().end(); pit++ )
    {
        PlaylistsModelItem *item = new PlaylistsModelItem(*pit);
        parent->addChild( item );
    }
}

void MainWindow::getPlaylistsResponse( const Folder& rootfolder, void* userData )
{
    ui->playlistsTree->clear();
    QTreeWidgetItem *parentItem = ui->playlistsTree->invisibleRootItem();
    addFolder( parentItem, rootfolder );
}
void MainWindow::getTracksResponse( const std::deque<Track>& tracks, void* userData )
{
    tracksModel.setTrackList(tracks);

}
void MainWindow::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    ui->label->setPixmap( QPixmap::fromImage(img) );
}
void MainWindow::getAlbumResponse( const Album& album, void* userData )
{
}
void MainWindow::getArtistResponse( const Artist& artist, void* userData )
{
}
void MainWindow::genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData )
{
}

void MainWindow::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress )
{
    m_.getImage( currentTrack.getAlbumLink(), this, NULL );
    ui->currentTrackLabel->setText( QString( currentTrack.getName().c_str() ) );
    ui->currentArtistLabel->setText( QString( currentTrack.getArtists().front().getName().c_str() ) );
    progress_ = progress / 1000;
    ui->durationLabel->setText( toTextLabel(currentTrack.getDurationMillisecs()/1000) );
    ui->progressLabel->setText( toTextLabel(progress_) );
    ui->progressBar->setMaximum( currentTrack.getDurationMillisecs()/1000 );
    ui->progressBar->setValue( progress/1000 );
    statusUpdateInd(state, repeatStatus, shuffleStatus, volume );
}

void MainWindow::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume )
{
    isPlaying = (state == PLAYBACK_PLAYING);

    ui->shuffleButton->setChecked( shuffleStatus );
    ui->repeatButton->setChecked( repeatStatus );
    QMetaObject::invokeMethod( this, "updateGui", Qt::QueuedConnection );

}
void MainWindow::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus, volume, currentTrack, progress );
}
void MainWindow::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus, volume );
}


void MainWindow::getCurrentAudioEndpointsResponse( const std::set<std::string>& endpoints, void* userData )
{
}

void MainWindow::renameEndpointResponse( void* userData ) {}
void MainWindow::getEndpointsResponse( const EndpointInfoList& endpoints, void* userData )
{
}

void MainWindow::getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData )
{
    endpoints_ = endpoints;
    QMetaObject::invokeMethod( this, "updateEndpointsGui", Qt::QueuedConnection );
}

void MainWindow::audioEndpointsUpdatedNtf()
{
    if ( ui->stackedWidget->currentWidget() == ui->endpointsPage )
    {
        epMgr_.getAudioEndpoints( this, NULL );
    }
}

