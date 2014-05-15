#include "PlaylistPage.h"
#include "ui_PlaylistPage.h"
#include <QMenu>

PlaylistPage::PlaylistPage( const LibSpotify::MediaBaseInfo& playlist, MediaInterface& m_, GlobalActionSlots& actions_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaylistPage),
    link(playlist.getLink()),
    m(m_),
    actions(actions_)
{
    ui->setupUi(this);

    ui->tableView->setModel( &tracksModel );
    ui->tableView->verticalHeader()->hide();

    ui->playlistTitle->setText( QString::fromStdString(playlist.getName()));

    m.registerForCallbacks( *this );

    m.getTracks( playlist.getLink(), this, NULL );
    m.getImage( link, this, NULL );    
}

PlaylistPage::~PlaylistPage()
{
    m.unRegisterForCallbacks( *this );
    delete ui;
}

void PlaylistPage::removeTracks()
{
    QAction* origin = (QAction*)sender();
    QVariant v = origin->data();
    if ( v.canConvert<std::deque<const LibSpotify::Track>>() )
    {
        std::deque<const LibSpotify::Track> tracks = v.value<std::deque<const LibSpotify::Track>>();
        //todo validate that the tracks are still at the same indexes
        std::set<int> indexes;
        std::deque<const LibSpotify::Track>::iterator it = tracks.begin();
        for( ; it != tracks.end(); it++ )
        {
            int index = (*it).getIndex();
            if ( index >= 0 )
                indexes.insert( index );
        }

        m.playlistRemoveTracks( link, indexes, this, NULL );
    }
}


void PlaylistPage::playlistUpdatedInd( const std::string& link )
{
    if ( this->link.compare( link ) == 0 )
    {
        m.getTracks( link, this, NULL );
    }
}
void PlaylistPage::getTracksResponse( const std::deque<Track>& tracks, void* userData )
{
    tracksModel.setTrackList(tracks);
}

void PlaylistPage::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    QPixmap pm;
    if ( img.width() != img.height() )
    {
        /* the height/width ratio can be anything so we crop it into square first */
        int s = img.width() < img.height() ? img.width() : img.height();
        int x = (img.width()-s)/2;
        int y = (img.height()-s)/2;
        pm = QPixmap::fromImage(img.copy( x, y, s, s ) );
    }
    else
    {
        /* image is already square */
        pm = QPixmap::fromImage(img);
    }
    // and now apply it to the image, it has the scale property set so it will resize the image
    ui->playlistImage->setPixmap( pm );
}

void PlaylistPage::on_tableView_doubleClicked(const QModelIndex &index)
{
    Track* t = static_cast<Track*>(index.internalPointer());
    m.play( link, t->getIndex(), this, NULL );
}

void PlaylistPage::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndexList list = ui->tableView->selectionModel()->selectedIndexes();

    const std::deque<const Track> items = tracksModel.getTracks( list );

    if ( items.size() > 0 )
    {
        QMenu menu;

        {
            QAction* act = menu.addAction( "Enqueue", &actions, SLOT(enqueueTracks()) );
            act->setData( QVariant::fromValue( items ) );
        }

        QMenu* addMenu = menu.addMenu( "Add to" );
        actions.populateAddTracksMenu( addMenu, items );

        {
            QAction* act = menu.addAction( "Remove", this, SLOT(removeTracks()) );
            act->setData( QVariant::fromValue( items ) );
        }

        /* only show browse artist/album if it's a single selection */
        if ( items.size() == 1 )
        {
            const Track& t = items.front();

            {
                QAction* act = menu.addAction( "Browse Album", &actions, SLOT(browseAlbum()) );
                act->setData( QVariant::fromValue( LibSpotify::MediaBaseInfo(t.getAlbum(),t.getAlbumLink())) );
            }

            int nartists = t.getArtists().size();
            if ( nartists == 1 )
            {
                QAction* act = menu.addAction( "Browse Artist", &actions, SLOT(browseArtist()) );
                act->setData( QVariant::fromValue( t.getArtists().front() ) );
            }
            else if ( nartists > 1 )
            {
                QMenu* submenu = menu.addMenu("Browse Artist");
                std::vector<LibSpotify::MediaBaseInfo>::const_iterator it = t.getArtists().begin();
                for ( ; it != t.getArtists().end(); it++ )
                {
                    QAction* act = submenu->addAction((*it).getName().c_str(), &actions, SLOT(browseArtist()));
                    act->setData( QVariant::fromValue( *it ) );
                }
            }
        }

        menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    }
}
