#ifndef ALBUMENTRY_H
#define ALBUMENTRY_H

#include "MediaContainers/Album.h"
#include "MediaInterface/MediaInterface.h"
#include "TrackListModel.h"
#include "GlobalActionInterface.h"
#include <QWidget>

namespace Ui {
class AlbumEntry;
}

class AlbumEntry : public QWidget, IMediaInterfaceCallbackSubscriber
{
    Q_OBJECT

public:
    explicit AlbumEntry(const LibSpotify::Album& album_, MediaInterface& m_, GlobalActionSlots& actions_, QWidget *parent = 0);
    ~AlbumEntry();

protected:
    virtual void paintEvent(QPaintEvent *event );

public slots:
    void updateAlbum();
    void enqueue();

private slots:
    void on_albumTracksTable_customContextMenuRequested(const QPoint &pos);
    void on_albumTracksTable_doubleClicked(const QModelIndex &index);

private:
    Ui::AlbumEntry *ui;

    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getAlbumResponse( const Album& album, void* userData );

    Album album;
    MediaInterface& m;
    GlobalActionSlots& actions;
    TrackListModel tracksModel;
    bool loaded;

};

#endif // ALBUMENTRY_H
