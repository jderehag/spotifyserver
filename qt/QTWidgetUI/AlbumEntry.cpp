#include "AlbumEntry.h"
#include "ui_AlbumEntry.h"
#include <QMenu>

AlbumEntry::AlbumEntry(const LibSpotify::Album& album_, const LibSpotify::MediaBaseInfo& owner_, MediaInterface& m_, GlobalActionSlots& actions_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlbumEntry),
    album(album_),
    owner(owner_),
    m(m_),
    actions(actions_),
    loaded(false)
{
    ui->setupUi(this);

    ui->titleLabel->setText( album_.getName().c_str() );
    ui->yearLabel->setText( QString::number( album_.getYear() ) );
}

void AlbumEntry::paintEvent( QPaintEvent  * event )
{
    QWidget::paintEvent ( event );

    if ( loaded == false )
    {
        loaded = true;
        m.getAlbum( album.getLink(), this, NULL );
        m.getImage( album.getLink(), this, NULL );
    }
}

AlbumEntry::~AlbumEntry()
{
    delete ui;
}

void AlbumEntry::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    ui->albumImage->setPixmap( QPixmap::fromImage(img) );
}
void AlbumEntry::getAlbumResponse( const Album& album, void* userData )
{
    this->album = album;
    QMetaObject::invokeMethod( this, "updateAlbum", Qt::QueuedConnection );
}

void AlbumEntry::updateAlbum()
{
    tracksModel.setTrackList( album.getTracks() );
    ui->albumTracksTable->setModel( &tracksModel );
    ui->albumTracksTable->hideColumn( TrackListModel::COLUMN_ALBUM );
}

void AlbumEntry::on_albumTracksTable_customContextMenuRequested(const QPoint &pos)
{
    QModelIndexList list = ui->albumTracksTable->selectionModel()->selectedIndexes();

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

        /* only show browse artist/album if it's a single selection */
        if ( items.size() == 1 )
        {
            const Track& t = items.front();

            {
                QAction* act = menu.addAction( "Browse Album", &actions, SLOT(browseAlbum()) );
                act->setData( QVariant::fromValue( LibSpotify::MediaBaseInfo(t.getAlbum(),t.getAlbumLink())) );
            }

            std::vector<MediaBaseInfo> artists = t.getArtists();
            std::vector<MediaBaseInfo>::iterator it = artists.begin();
            while (  it != artists.end() )
            {
                if ( (*it) == owner )
                    it = artists.erase(it);
                else
                    it++;
            }
            int nartists = artists.size();
            if ( nartists == 1 )
            {
                QAction* act = menu.addAction( QString("Browse ").append(artists.front().getName().c_str()), &actions, SLOT(browseArtist()) );
                act->setData( QVariant::fromValue( artists.front() ) );
            }
            else if ( nartists > 1 )
            {
                QMenu* submenu = menu.addMenu("Browse Artist");
                std::vector<LibSpotify::MediaBaseInfo>::const_iterator it = artists.begin();
                for ( ; it != artists.end(); it++ )
                {
                    QAction* act = submenu->addAction((*it).getName().c_str(), &actions, SLOT(browseArtist()));
                    act->setData( QVariant::fromValue( *it ) );
                }
            }
        }

        menu.exec(ui->albumTracksTable->viewport()->mapToGlobal(pos));
    }
}

void AlbumEntry::on_albumTracksTable_doubleClicked(const QModelIndex &index)
{
    Track* t = static_cast<Track*>(index.internalPointer());
    m.play( album.getLink(), t->getIndex(), this, NULL );
}
