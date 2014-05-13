#include "ArtistPage.h"
#include "AlbumEntry.h"
#include "ui_ArtistPage.h"

ArtistPage::ArtistPage( const LibSpotify::MediaBaseInfo& artist_, MediaInterface &m_, GlobalActionSlots &actions_, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::ArtistPage),
    artist(artist_.getName(), artist_.getLink()),
    m(m_),
    actions(actions_)
{
    ui->setupUi(this);

    ui->artistPageTitle->setText(QString::fromStdString( artist.getName() ) );

    m.getArtist( artist.getLink(), this, NULL);
    m.getImage( artist.getLink(), this, NULL );
}

ArtistPage::~ArtistPage()
{
    delete ui;
}

void ArtistPage::getImageResponse( const void* data, size_t dataSize, void* userData )
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
    ui->artistPageImage->setPixmap( pm );
}
void ArtistPage::getArtistResponse( const Artist& artist, void* userData )
{
    this->artist = artist;
    QMetaObject::invokeMethod( this, "updateArtist", Qt::QueuedConnection );
}

void ArtistPage::updateArtist()
{
    ui->artistPageTitle->setText(QString::fromStdString( artist.getName() ) );

    AlbumContainer::const_iterator it = artist.getAlbums().begin();
    for ( ; it != artist.getAlbums().end(); it++ )
    {
        ui->artistAlbumsContainerLayout->addWidget(new AlbumEntry((*it), artist, m, actions));
    }
}
