#include "TrackListModel.h"

TrackListModel::TrackListModel()
{
}

void TrackListModel::setTrackList( const std::deque<LibSpotify::Track>& tracks_ )
{
    beginResetModel();
    tracks = tracks_;
    endResetModel();
}

int TrackListModel::columnCount( const QModelIndex & parent ) const
{
    return NCOLUMNS;
}

int TrackListModel::rowCount( const QModelIndex & parent ) const
{
    return tracks.size();
}

QVariant TrackListModel::data( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() || index.row() >= tracks.size() || index.column() >= NCOLUMNS || role != Qt::DisplayRole )
        return QVariant();
    else
    {
        switch ( index.column() )
        {
        case COLUMN_NAME:
            return QVariant( tracks[index.row()].getName().c_str() );
        case COLUMN_ARTIST:
            return QVariant( tracks[index.row()].getArtists().front().getName().c_str() );
        case COLUMN_ALBUM:
            return QVariant( tracks[index.row()].getAlbum().c_str() );
        }
    }
}

QModelIndex TrackListModel::index( int row, int column, const QModelIndex & parent ) const
{
    if( column >= NCOLUMNS || row >= tracks.size())
        return QModelIndex();
    else
        return createIndex(row, column, (void*)&tracks[row]);
}

QModelIndex TrackListModel::parent( const QModelIndex & index ) const
{
    return QModelIndex();
}

QVariant TrackListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( section >= NCOLUMNS || orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    else
    {
        switch (section)
        {
        case COLUMN_NAME:
            return QVariant(QString("Name"));
        case COLUMN_ARTIST:
            return QVariant(QString("Artist"));
        case COLUMN_ALBUM:
            return QVariant(QString("Album"));
        }
    }
}
