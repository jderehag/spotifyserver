#ifndef TRACKLISTMODEL_H
#define TRACKLISTMODEL_H

#include <QAbstractItemModel>
#include "MediaContainers/Track.h"
#include <deque>

class TrackListModel : public QAbstractItemModel
{
private:
    std::deque<LibSpotify::Track> tracks;
public:
    enum
    {
        COLUMN_NAME,
        COLUMN_ARTIST,
        COLUMN_ALBUM,
        NCOLUMNS
    };

    TrackListModel();
    void setTrackList( const std::deque<LibSpotify::Track>& tracks_ );
    const std::deque<const LibSpotify::Track> getTracks( const QModelIndexList & index ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

};

#endif // TRACKLISTMODEL_H
