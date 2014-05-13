#ifndef ALBUMPAGE_H
#define ALBUMPAGE_H

#include <QWidget>
#include "TrackListModel.h"
#include "GlobalActionInterface.h"
#include "MediaInterface/MediaInterface.h"


namespace Ui {
class AlbumPage;
}

class AlbumPage : public QWidget, IMediaInterfaceCallbackSubscriber
{
    Q_OBJECT

public:
    explicit AlbumPage( const LibSpotify::MediaBaseInfo& album_, MediaInterface& m_, GlobalActionSlots& actions_, QWidget *parent = 0 );
    ~AlbumPage();

public slots:
    void updateAlbum();

private slots:
    void on_albumTracksTable_customContextMenuRequested(const QPoint &pos);
    void on_albumTracksTable_doubleClicked(const QModelIndex &index);

private:
    Ui::AlbumPage *ui;

    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getAlbumResponse( const Album& album, void* userData );

    LibSpotify::Album album;
    MediaInterface& m;
    GlobalActionSlots& actions;
    TrackListModel albumTracksModel;
};

#endif // ALBUMPAGE_H
