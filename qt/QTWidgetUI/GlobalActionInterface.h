#ifndef GLOBALACTIONINTERFACE_H
#define GLOBALACTIONINTERFACE_H

#include <QAction>
#include "MediaContainers/Album.h"
#include "MediaContainers/Artist.h"

Q_DECLARE_METATYPE(LibSpotify::MediaBaseInfo)

class IGlobalActionCallbacks
{
public:
    virtual void browseArtist( const LibSpotify::MediaBaseInfo& artist ) = 0;
    virtual void browseAlbum( const LibSpotify::MediaBaseInfo& album ) = 0;
    virtual void enqueueTrack( std::string link ) = 0;
};

class GlobalActionSlots : public QObject
{
    Q_OBJECT
public:
    GlobalActionSlots( IGlobalActionCallbacks& cb_ );
public slots:
    void browseArtist();
    void browseAlbum();
    void enqueueTrack();

private:
    IGlobalActionCallbacks& cb;
};

#endif // GLOBALACTIONINTERFACE_H
