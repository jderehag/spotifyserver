
#include "GlobalActionInterface.h"

GlobalActionSlots::GlobalActionSlots( IGlobalActionCallbacks& cb_ ) : cb(cb_)
{}


void GlobalActionSlots::browseArtist()
{
    QAction* origin = (QAction*)sender();
    cb.browseArtist( origin->data().toString().toStdString() );

}
void GlobalActionSlots::browseAlbum()
{
    QAction* origin = (QAction*)sender();
    cb.browseAlbum( origin->data().toString().toStdString() );
}

void GlobalActionSlots::enqueueTrack()
{
    QAction* origin = (QAction*)sender();
    cb.enqueueTrack( origin->data().toString().toStdString() );
}

