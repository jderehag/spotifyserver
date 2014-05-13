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
    m.getTracks( playlist.getLink(), this, NULL );
    m.getImage( link, this, NULL );
}

PlaylistPage::~PlaylistPage()
{
    delete ui;
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
    std::deque<TrackListModel::ContextMenuItem> items = tracksModel.constructContextMenu( ui->tableView->indexAt( pos ) );
    if ( items.size() > 0 )
    {
        QMenu* menu = new QMenu();
        std::deque<TrackListModel::ContextMenuItem>::iterator it = items.begin();
        for ( ; it != items.end(); it++ )
        {
            TrackListModel::ContextMenuItem& item = (*it);
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

            act->setText( item.text );
            act->setData( QVariant::fromValue( item.arg ) );
            menu->addAction(act);
        }
        menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
    }
}
