#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "PlaylistPage.h"
#include "ArtistPage.h"
#include "AlbumPage.h"
#include "EndpointsPage.h"
#include "PlaylistModel.h"
#include "Logger/applog.h"

MainWindow::MainWindow( QString& title, MediaInterface& m, EndpointCtrlInterface& epMgr, QWidget *parent ) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        m_(m), epMgr_(epMgr),
        actions(*this),
        progress_(0), isPlaying(false)
{
    Q_INIT_RESOURCE(QTResources);

    ui->setupUi(this);
    setWindowTitle( title );
    ui->playlistsTree->setColumnCount( 1 );

    // the splitter freaks out when it doesn't have any pages, manually set width
    QList<int> sizes;
    sizes.append(1); //this will force left frame to its minimum size
    sizes.append(1000); //and whatever the rest is to right frame
    ui->splitter->setSizes(sizes);

    ui->playButton->setToolTip(tr("Play"));
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    ui->nextButton->setToolTip(tr("Next"));
    ui->nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    ui->prevButton->setToolTip(tr("Previous"));
    ui->prevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    ui->actionBack->setEnabled(false);
    ui->actionBack->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->actionForward->setEnabled(false);
    ui->actionForward->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));

    connect(&progressTimer, SIGNAL(timeout()), this, SLOT(progressUpdate()));

    m_.registerForCallbacks( *this );
}

MainWindow::~MainWindow()
{
    m_.unRegisterForCallbacks( *this );
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

void MainWindow::newPage( QWidget* page )
{
    int currentIndex = ui->stackedWidget->currentIndex();
    int lastIndex = ui->stackedWidget->count() - 1;
    // if current page isn't last in list, discard all pages after current
    if ( currentIndex != -1 && currentIndex != lastIndex )
    {

        while( currentIndex+1 < ui->stackedWidget->count() )
        {
            QWidget* p = ui->stackedWidget->widget(currentIndex+1);
            ui->stackedWidget->removeWidget(p);
            delete p;
        }
    }

    //keep maximum 30 pages
    if ( ui->stackedWidget->count() >= 30 )
    {
        QWidget* p = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(p);
        delete p;
    }

    //and finally insert new page
    int index = ui->stackedWidget->addWidget( page );
    ui->stackedWidget->setCurrentIndex( index );

    ui->actionForward->setEnabled(false);
    if ( index > 0 )
        ui->actionBack->setEnabled(true);
}

void MainWindow::on_playlistsTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if ( current )
    {
        PlaylistsModelItem* pitem = (PlaylistsModelItem*)current;
        const std::string& link = pitem->getLink();
        if ( !link.empty() )
        {
            newPage( new PlaylistPage( pitem->m, m_, actions, this ) );
        }
    }
}


void MainWindow::on_playlistsTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    PlaylistsModelItem* pitem = (PlaylistsModelItem*)item;
    const std::string& link = pitem->getLink();
    if ( !link.empty() )
        m_.play( link, this, NULL );
}

void MainWindow::on_actionShowEndpoints_triggered()
{
    newPage( new EndpointsPage(epMgr_, this) );
}

void MainWindow::on_actionBack_triggered()
{
    if ( ui->stackedWidget->currentIndex() > 0 )
    {
        int newIndex = ui->stackedWidget->currentIndex() - 1;
        ui->stackedWidget->setCurrentIndex( newIndex);
        ui->actionForward->setEnabled(true);
        if ( newIndex == 0 )
            ui->actionBack->setEnabled(false);

    }
}

void MainWindow::on_actionForward_triggered()
{
    if ( ui->stackedWidget->currentIndex() < ui->stackedWidget->count() - 1 )
    {
        int newIndex = ui->stackedWidget->currentIndex() + 1;
        ui->stackedWidget->setCurrentIndex( ui->stackedWidget->currentIndex()+1);
        ui->actionBack->setEnabled(true);
        if ( newIndex == ui->stackedWidget->count() - 1 )
            ui->actionForward->setEnabled(false);
    }
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

void MainWindow::browseArtist(const MediaBaseInfo& artist )
{
    newPage(  new ArtistPage( artist, m_, actions, this ) );
}

void MainWindow::browseAlbum( const MediaBaseInfo& album )
{
    newPage( new AlbumPage( album, m_, actions, this ) );
}

void MainWindow::enqueueTrack( const std::string& link )
{
    m_.enqueue(link, this, NULL);
}

void MainWindow::addTracks( const std::string& playlist, const std::deque<const LibSpotify::Track>& tracks )
{
    std::list<std::string> tracklinks;

    std::deque<const LibSpotify::Track>::const_iterator it = tracks.begin();
    for( ; it != tracks.end(); it++ )
        tracklinks.push_back( (*it).getLink() );
    m_.playlistAddTracks( playlist, tracklinks, -1, this, NULL );
}

void MainWindow::rootFolderUpdatedInd()
{
    m_.getPlaylists( this, NULL );
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
    FolderItemContainer::const_iterator fit = folder.getItems().begin();
    for ( ; fit != folder.getItems().end(); fit++ )
    {
        PlaylistsModelItem* item;
        if ( (*fit)->isFolder )
        {
            const Folder* f = dynamic_cast<const Folder*>(*fit);
            item = new PlaylistsModelItem(*f);
            addFolder( item, *f );
        }
        else
        {
            const Playlist* p = dynamic_cast<const Playlist*>(*fit);
            item = new PlaylistsModelItem(*p);
        }
        parent->addChild( item );
    }
}

void MainWindow::getPlaylistsResponse( const Folder& rootfolder, void* userData )
{
    /* todo this isn't safe*/
    ui->playlistsTree->clear();
    QTreeWidgetItem* parentItem = ui->playlistsTree->invisibleRootItem();
    addFolder( parentItem, rootfolder );

    actions.playlistsUpdated( parentItem );
}

void MainWindow::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    QLabel* origin = (QLabel*)userData;
    int s = img.width() < img.height() ? img.width() : img.height();
    int x = (img.width()-s)/2;
    int y = (img.height()-s)/2;
    QPixmap pm = QPixmap::fromImage(img.copy( x, y, s, s ) );
    origin->setPixmap( pm );
}

void MainWindow::getArtistResponse( const Artist& artist, void* userData )
{
}
void MainWindow::genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData )
{
}

void MainWindow::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress )
{
    m_.getImage( currentTrack.getAlbumLink(), this, ui->label );
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



