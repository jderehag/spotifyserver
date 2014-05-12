#include "ArtistPage.h"
#include "AlbumEntry.h"
#include "ui_ArtistPage.h"

ArtistPage::ArtistPage( const std::string& link_, MediaInterface &m_, GlobalActionSlots &actions_, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::ArtistPage),
    artist(NULL),
    m(m_),
    actions(actions_)
{
    ui->setupUi(this);

    m.getArtist( link_, this, NULL);
    m.getImage( link_, this, NULL );
}

ArtistPage::~ArtistPage()
{
    delete ui;
}

void ArtistPage::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    QImage img = QImage::fromData( (const uchar*)data, (int)dataSize );
    // the size/width ratio can be anything so we crop it into square first
    int s = img.width() < img.height() ? img.width() : img.height();
    int x = (img.width()-s)/2;
    int y = (img.height()-s)/2;
    QPixmap pm = QPixmap::fromImage(img.copy( x, y, s, s ) );
    ui->artistPageImage->setPixmap( pm );
}
void ArtistPage::getArtistResponse( const Artist& artist, void* userData )
{
    this->artist = new Artist(artist);
    QMetaObject::invokeMethod( this, "updateArtist", Qt::QueuedConnection );
}

void ArtistPage::updateArtist()
{
    ui->artistPageTitle->setText(QString::fromStdString( artist->getName() ) );

    AlbumContainer::const_iterator it = artist->getAlbums().begin();
    for ( ; it != artist->getAlbums().end(); it++ )
    {
        ui->artistAlbumsContainerLayout->addWidget(new AlbumEntry((*it), m, actions));
    }
}
