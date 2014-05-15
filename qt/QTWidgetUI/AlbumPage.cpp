#include "AlbumPage.h"
#include "ui_AlbumPage.h"
#include <QMenu>

AlbumPage::AlbumPage( const LibSpotify::MediaBaseInfo& album_, MediaInterface& m_, GlobalActionSlots &actions_, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::AlbumPage),
    album(album_.getName(), album_.getLink()),
    m(m_),
    actions(actions_)
{
    ui->setupUi(this);

    ui->albumPageTitle->setText( QString::fromStdString( album.getName() ) );

    ui->albumPageLabelsLayout->setAlignment( Qt::AlignTop );

    m.getAlbum( album.getLink(), this, NULL);
    m.getImage( album.getLink(), this, ui->albumPageImage );
}

AlbumPage::~AlbumPage()
{
    delete ui;
}

void AlbumPage::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    ui->albumPageImage->setPixmap( QPixmap::fromImage(img) );
}
void AlbumPage::getAlbumResponse( const Album& album, void* userData )
{
    this->album = album;
    QMetaObject::invokeMethod( this, "updateAlbum", Qt::QueuedConnection );
}

void AlbumPage::updateAlbum()
{
    ui->albumPageTitle->setText( QString::fromStdString( album.getName() ) );
    ui->albumPageArtist->setText( QString::fromStdString( album.getArtist().getName() ) );
    ui->albumPageYear->setText( QString::number( album.getYear() ) );

    albumTracksModel.setTrackList( album.getTracks() );
    ui->albumTracksTable->setModel( &albumTracksModel );
    ui->albumTracksTable->hideColumn( TrackListModel::COLUMN_ALBUM );
}

void AlbumPage::on_albumTracksTable_doubleClicked(const QModelIndex &index)
{
    Track* t = static_cast<Track*>(index.internalPointer());
    m.play( t->getAlbumLink(), t->getIndex(), this, NULL );
}

void AlbumPage::on_albumTracksTable_customContextMenuRequested(const QPoint &pos)
{
    QModelIndexList list = ui->albumTracksTable->selectionModel()->selectedIndexes();

    const std::deque<const Track> items = albumTracksModel.getTracks( list );

    if ( items.size() > 0 )
    {
        QMenu menu;

        {
            QAction* act = menu.addAction( "Enqueue", &actions, SLOT(enqueueTracks()) );
            act->setData( QVariant::fromValue( items ) );
        }

        QMenu* addMenu = menu.addMenu( "Add to" );
        actions.populateAddTracksMenu( addMenu, items );

        /* only show browse artist if it's a single selection */
        if ( items.size() == 1 )
        {
            const Track& t = items.front();

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

        menu.exec(ui->albumTracksTable->viewport()->mapToGlobal(pos));
    }
}

