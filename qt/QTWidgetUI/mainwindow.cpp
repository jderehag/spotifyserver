#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/applog.h"


class PlaylistsModelItem : public QStandardItem
{
private:
    MediaBaseInfo m;
public:
    PlaylistsModelItem( const MediaBaseInfo& m_ ) : QStandardItem(QString::fromStdString(m_.getName())), m(m_) {}
    const std::string& getLink() { return m.getLink(); }
};


MainWindow::MainWindow( MediaInterface& m, EndpointCtrlInterface& epMgr, QWidget *parent ) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_(m), epMgr_(epMgr), isPlaying(false)
{
    ui->setupUi(this);
    ui->treeView->setModel( &model );
    ui->tableView->setModel( &tracksModel );
    ui->label->setScaledContents(true);

    ui->playButton->setToolTip(tr("Play"));
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    ui->nextButton->setToolTip(tr("Next"));
    ui->nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    ui->prevButton->setToolTip(tr("Previous"));
    ui->prevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    m_.registerForCallbacks( *this );

}

MainWindow::~MainWindow()
{
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


void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    PlaylistsModelItem* item = (PlaylistsModelItem*)model.itemFromIndex(index);
    const std::string& link = item->getLink();
    if ( !link.empty() )
        m_.getTracks( link, this, NULL );
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    PlaylistsModelItem* item = (PlaylistsModelItem*)model.itemFromIndex(ui->treeView->currentIndex());
    Track* t = static_cast<Track*>(index.internalPointer());
    m_.play( item->getLink(), t->getIndex(), this, NULL );
}

void MainWindow::updateGui()
{
    if ( isPlaying )
    {
        ui->playButton->setToolTip(tr("Pause"));
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
    else
    {
        ui->playButton->setToolTip(tr("Play"));
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}


QVariant toString ( const Track& t, int column )
{
    switch ( column )
    {
    case 0:
        return QVariant( t.getName().c_str() );
    case 1:
        return QVariant( t.getArtists().front().getName().c_str() );
    }
}



void MainWindow::rootFolderUpdatedInd()
{}
void MainWindow::connectionState( bool up )
{
    if ( up )
    {
        m_.getPlaylists( this, NULL );
        m_.getStatus( this, NULL );
    }
}

static void addFolder( QStandardItem* parent, const Folder& folder )
{
    FolderContainer::const_iterator fit = folder.getFolders().begin();
    for ( ; fit != folder.getFolders().end(); fit++ )
    {
        PlaylistsModelItem *item = new PlaylistsModelItem(*fit);
        addFolder( item, *fit );
        parent->appendRow( item );
    }

    PlaylistContainer::const_iterator pit = folder.getPlaylists().begin();
    for ( ; pit != folder.getPlaylists().end(); pit++ )
    {
        PlaylistsModelItem *item = new PlaylistsModelItem(*pit);
        parent->appendRow( item );
    }
}

void MainWindow::getPlaylistsResponse( const Folder& rootfolder, void* userData )
{
    model.clear();
    QStandardItem *parentItem = model.invisibleRootItem();
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
    statusUpdateInd(state, repeatStatus, shuffleStatus, volume );
}
void MainWindow::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume )
{
    isPlaying = (state == PLAYBACK_PLAYING);

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
}

void MainWindow::audioEndpointsUpdatedNtf()
{
}


