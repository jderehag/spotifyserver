#include "AlbumPage.h"
#include "ui_AlbumPage.h"
#include <QMenu>

AlbumPage::AlbumPage(const std::string& link_, MediaInterface& m_, GlobalActionSlots &actions_, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::AlbumPage),
    album(NULL),
    m(m_),
    actions(actions_)
{
    ui->setupUi(this);

    ui->albumPageLabelsLayout->setAlignment( Qt::AlignTop );
    ui->albumTracksTable->verticalHeader()->hide();

    m.getAlbum( link_, this, NULL);
    m.getImage( link_, this, ui->albumPageImage );
}

AlbumPage::~AlbumPage()
{
    delete ui;
    if ( album ) delete album;
}

void AlbumPage::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    ui->albumPageImage->setPixmap( QPixmap::fromImage(img) );
}
void AlbumPage::getAlbumResponse( const Album& album, void* userData )
{
    this->album = new Album(album);
    QMetaObject::invokeMethod( this, "updateAlbum", Qt::QueuedConnection );
}

void AlbumPage::updateAlbum()
{
    ui->albumPageTitle->setText( QString::fromStdString( album->getName() ) );
    ui->albumPageArtist->setText( QString::fromStdString( album->getArtist().getName() ) );
    ui->albumPageYear->setText( QString::number( album->getYear() ) );

    albumTracksModel.setTrackList( album->getTracks() );
    ui->albumTracksTable->setModel( &albumTracksModel );
}

void AlbumPage::on_albumTracksTable_doubleClicked(const QModelIndex &index)
{
    Track* t = static_cast<Track*>(index.internalPointer());
    m.play( t->getAlbumLink(), t->getIndex(), this, NULL );
}

void AlbumPage::on_albumTracksTable_customContextMenuRequested(const QPoint &pos)
{
    std::deque<TrackListModel::ContextMenuItem> items = albumTracksModel.constructContextMenu( ui->albumTracksTable->indexAt( pos ) );
    if ( items.size() > 0 )
    {
        QMenu* menu = new QMenu();
        std::deque<TrackListModel::ContextMenuItem>::iterator it = items.begin();
        for ( ; it != items.end(); it++ )
        {
            if ( (*it).type == TrackListModel::ContextMenuItem::BROWSE_ALBUM )
                continue;

            QAction* act = new QAction(this);
            switch( (*it).type )
            {
            case TrackListModel::ContextMenuItem::ENQUEUE:
                connect( act, SIGNAL(triggered()), &actions, SLOT(enqueueTrack()));
                break;
            case TrackListModel::ContextMenuItem::BROWSE_ARTIST:
                connect( act, SIGNAL(triggered()), &actions, SLOT(browseArtist()));
                break;
            }

            act->setText((*it).text);
            act->setData(QVariant(QString( (*it).arg.c_str() )));
            menu->addAction(act);
        }
        menu->popup(ui->albumTracksTable->viewport()->mapToGlobal(pos));
    }
}

