#ifndef ALBUMENTRY_H
#define ALBUMENTRY_H

#include "MediaContainers/Album.h"
#include "MediaInterface/MediaInterface.h"
#include "TrackListModel.h"
#include <QWidget>

namespace Ui {
class AlbumEntry;
}

class AlbumEntry : public QWidget, IMediaInterfaceCallbackSubscriber
{
    Q_OBJECT

public:
    explicit AlbumEntry(const LibSpotify::Album& album_, MediaInterface& m_, QWidget *parent = 0);
    ~AlbumEntry();
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
    TrackListModel tracksModel;

};

#endif // ALBUMENTRY_H
