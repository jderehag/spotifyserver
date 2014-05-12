#include "AlbumEntry.h"
#include "ui_AlbumEntry.h"
#include <QMenu>

AlbumEntry::AlbumEntry(const LibSpotify::Album& album_, MediaInterface& m_, QWidget *parent) :
    album(album_),
    m(m_),
    QWidget(parent),
    ui(new Ui::AlbumEntry)
{
    ui->setupUi(this);
    m_.getAlbum( album_.getLink(), this, NULL );
    m_.getImage( album_.getLink(), this, NULL );

    ui->titleLabel->setText( album_.getName().c_str() );
    ui->yearLabel->setText( QString::number( album_.getYear() ) );
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
            if ( (*it).type == TrackListModel::ContextMenuItem::BROWSE_ARTIST )
                continue;
            if ( (*it).type == TrackListModel::ContextMenuItem::BROWSE_ALBUM ) //todo!
                continue;

            QAction* act = new QAction(this);
            switch( (*it).type )
            {
            case TrackListModel::ContextMenuItem::ENQUEUE:
                connect( act, SIGNAL(triggered()), this, SLOT(enqueue()));
                break;
            }

            act->setText((*it).text);
            act->setData(QVariant(QString( (*it).arg.c_str() )));
            menu->addAction(act);
        }
        menu->popup(ui->albumTracksTable->viewport()->mapToGlobal(pos));
    }
}

void AlbumEntry::enqueue()
{
    QAction* origin = (QAction*)sender();
    QString data = origin->data().toString();
    m.enqueue(data.toStdString(), this, NULL);
}

void AlbumEntry::on_albumTracksTable_doubleClicked(const QModelIndex &index)
{
    Track* t = static_cast<Track*>(index.internalPointer());
    m.play( album.getLink(), t->getIndex(), this, NULL );
}