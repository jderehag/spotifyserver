
#include "GlobalActionInterface.h"

GlobalActionSlots::GlobalActionSlots( IGlobalActionCallbacks& cb_ ) : cb(cb_), playlistsRoot(NULL)
{}

GlobalActionSlots::~GlobalActionSlots()
{

}

void GlobalActionSlots::playlistsUpdated(const QTreeWidgetItem *newRoot )
{
    /* todo protect this */
    playlistsRoot = newRoot;
}

void GlobalActionSlots::populateAddTracksMenu(QMenu* parent, const QTreeWidgetItem* folder, const std::deque<const LibSpotify::Track>& tracks  )
{
    for( int i = 0; i < folder->childCount(); i++ )
    {
        PlaylistsModelItem* item = static_cast<PlaylistsModelItem*>(folder->child(i));
        if ( item->childCount() > 0 )
        {
            QMenu* submenu = parent->addMenu( item->m.getName().c_str() );
            populateAddTracksMenu( submenu, item, tracks );
        }
        else
        {
            QAction* act = parent->addAction( item->m.getName().c_str(), this, SLOT(addTracks()) );
            act->setData( QVariant::fromValue(PlaylistActionData(item->getLink(), tracks)));
        }
    }
}

void GlobalActionSlots::populateAddTracksMenu( QMenu* parent, const std::deque<const LibSpotify::Track>& tracks  )
{
    populateAddTracksMenu( parent, playlistsRoot, tracks );
}

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

void GlobalActionSlots::enqueueTracks()
{
    QAction* origin = (QAction*)sender();
    QVariant v = origin->data();
    if ( v.canConvert<std::deque<const LibSpotify::Track>>() )
    {
        std::deque<const LibSpotify::Track> tracks = v.value<std::deque<const LibSpotify::Track>>();
        std::deque<const LibSpotify::Track>::iterator it = tracks.begin();
        for ( ; it != tracks.end(); it++ )
            cb.enqueueTrack( (*it).getLink() );
    }
}

void GlobalActionSlots::addTracks()
{
    QAction* origin = (QAction*)sender();
    QVariant v = origin->data();
    if ( v.canConvert<PlaylistActionData>() )
    {
        PlaylistActionData d = v.value<PlaylistActionData>();
        cb.addTracks( d.playlist, d.tracks );
    }
}
