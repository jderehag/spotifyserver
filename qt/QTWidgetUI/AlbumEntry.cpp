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
    std::deque<TrackListModel::ContextMenuItem> items = tracksModel.constructContextMenu( ui->albumTracksTable->indexAt( pos ) );
    if ( items.size() > 0 )
    {
        QMenu* menu = new QMenu();
        std::deque<TrackListModel::ContextMenuItem>::iterator it = items.begin();
        for ( ; it != items.end(); it++ )
        {
            TrackListModel::ContextMenuItem& item = (*it);
            if ( item.type == TrackListModel::ContextMenuItem::BROWSE_ARTIST && item.arg == owner )
                continue;

            QAction* act = new QAction(this);
            switch( item.type )
            {
            case TrackListModel::ContextMenuItem::ENQUEUE:
                connect( act, SIGNAL(triggered()), &actions, SLOT(enqueueTrack()));
                break;
            case TrackListModel::ContextMenuItem::BROWSE_ALBUM:
                connect( act, SIGNAL(triggered()), &actions, SLOT(browseAlbum()));
                break;
            case TrackListModel::ContextMenuItem::BROWSE_ARTIST:
                connect( act, SIGNAL(triggered()), &actions, SLOT(browseArtist()));
                break;
            }

            act->setText(item.text);
            act->setData( QVariant::fromValue( item.arg ) );
            menu->addAction(act);
        }
        menu->popup(ui->albumTracksTable->viewport()->mapToGlobal(pos));
    }
}

void AlbumEntry::on_albumTracksTable_doubleClicked(const QModelIndex &index)
{
    Track* t = static_cast<Track*>(index.internalPointer());
    m.play( album.getLink(), t->getIndex(), this, NULL );
}
