#ifndef GLOBALACTIONINTERFACE_H
#define GLOBALACTIONINTERFACE_H

#include <QAction>
#include <QMenu>
#include "PlaylistModel.h"
#include "MediaContainers/MediaBaseInfo.h"
#include "MediaContainers/Track.h"
#include <deque>

struct PlaylistActionData
{
    std::string playlist;
    std::deque<const LibSpotify::Track> tracks;
    PlaylistActionData(const std::string& playlist_, const std::deque<const LibSpotify::Track>& tracks_ ) : playlist(playlist_), tracks(tracks_) {}
    PlaylistActionData() {}
};

Q_DECLARE_METATYPE(LibSpotify::MediaBaseInfo)
Q_DECLARE_METATYPE(std::deque<const LibSpotify::Track>)
Q_DECLARE_METATYPE(PlaylistActionData)

class IGlobalActionCallbacks
{
public:
    virtual void browseArtist( const LibSpotify::MediaBaseInfo& artist ) = 0;
    virtual void browseAlbum( const LibSpotify::MediaBaseInfo& album ) = 0;
    virtual void enqueueTrack( const std::string& link ) = 0;
    virtual void addTracks( const std::string& playlist, const std::deque<const LibSpotify::Track>& tracks ) = 0;
};

class GlobalActionSlots : public QObject
{
    Q_OBJECT
public:
    GlobalActionSlots( IGlobalActionCallbacks& cb_ );
    ~GlobalActionSlots();
    void playlistsUpdated( const QTreeWidgetItem* newRoot );
    void populateAddTracksMenu(QMenu* parent, const std::deque<const LibSpotify::Track>& tracks );
public slots:
    void browseArtist();
    void browseAlbum();
    void enqueueTracks();
    void addTracks();
private:
    void populateAddTracksMenu(QMenu* parent, const QTreeWidgetItem* folder, const std::deque<const LibSpotify::Track>& tracks );
    IGlobalActionCallbacks& cb;
    const QTreeWidgetItem* playlistsRoot;
};

#endif // GLOBALACTIONINTERFACE_H
