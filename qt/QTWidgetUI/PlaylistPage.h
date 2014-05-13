#ifndef PLAYLISTPAGE_H
#define PLAYLISTPAGE_H

#include "MediaInterface/MediaInterface.h"
#include "GlobalActionInterface.h"
#include "TrackListModel.h"
#include <QWidget>

namespace Ui {
class PlaylistPage;
}

class PlaylistPage : public QWidget, IMediaInterfaceCallbackSubscriber
{
    Q_OBJECT

public:
    explicit PlaylistPage( const LibSpotify::MediaBaseInfo& playlist, MediaInterface& m_, GlobalActionSlots& actions_, QWidget *parent = 0);
    ~PlaylistPage();

private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_tableView_customContextMenuRequested(const QPoint &pos);

private:
    Ui::PlaylistPage *ui;

    /* implaments IMediaInterfaceCallbackSubscriber */
    virtual void playlistUpdatedInd( const std::string& link );
    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getTracksResponse( const std::deque<Track>& tracks, void* userData );

    std::string link;
    TrackListModel tracksModel;
    MediaInterface& m;
    GlobalActionSlots& actions;
};

#endif // PLAYLISTPAGE_H
