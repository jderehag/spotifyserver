
#include "GlobalActionInterface.h"

GlobalActionSlots::GlobalActionSlots( IGlobalActionCallbacks& cb_ ) : cb(cb_)
{}


void GlobalActionSlots::browseArtist()
{
    QAction* origin = (QAction*)sender();
    QVariant v = origin->data();
    if ( v.canConvert<LibSpotify::MediaBaseInfo>() )
    {
        LibSpotify::MediaBaseInfo m = v.value<LibSpotify::MediaBaseInfo>();
        cb.browseArtist( m );
    }

}
void GlobalActionSlots::browseAlbum()
{
    QAction* origin = (QAction*)sender();
    QVariant v = origin->data();
    if ( v.canConvert<LibSpotify::MediaBaseInfo>() )
    {
        LibSpotify::MediaBaseInfo m = v.value<LibSpotify::MediaBaseInfo>();
        cb.browseAlbum( m );
    }
}

void GlobalActionSlots::enqueueTrack()
{
    QAction* origin = (QAction*)sender();
    QVariant v = origin->data();
    if ( v.canConvert<LibSpotify::MediaBaseInfo>() )
    {
        LibSpotify::MediaBaseInfo m = v.value<LibSpotify::MediaBaseInfo>();
        cb.enqueueTrack( m.getLink() );
    }
}

