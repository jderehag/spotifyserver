#ifndef ARTISTPAGE_H
#define ARTISTPAGE_H

#include "MediaInterface/MediaInterface.h"
#include "GlobalActionInterface.h"
#include <QWidget>

namespace Ui {
class ArtistPage;
}

class ArtistPage : public QWidget, IMediaInterfaceCallbackSubscriber
{
    Q_OBJECT

public:
    explicit ArtistPage( const std::string& link_, MediaInterface& m_, GlobalActionSlots& actions_, QWidget *parent = 0);
    ~ArtistPage();

public slots:
    void updateArtist();

private:
    Ui::ArtistPage *ui;

    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getArtistResponse( const Artist& artist, void* userData );

    LibSpotify::Artist* artist;
    MediaInterface& m;
    GlobalActionSlots& actions;

};

#endif // ARTISTPAGE_H
