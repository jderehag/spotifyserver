#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QTreeWidgetItem>
#include "MediaContainers/MediaBaseInfo.h"

class PlaylistsModelItem : public QTreeWidgetItem
{
public:
    LibSpotify::MediaBaseInfo m;
    PlaylistsModelItem( const LibSpotify::MediaBaseInfo& m_ ) : m(m_) {}
    const std::string& getLink() { return m.getLink(); }
    virtual QVariant data ( int column, int role ) const { if ( role != Qt::DisplayRole || column != 0 ) return QVariant(); else return QVariant( m.getName().c_str() ); }
};

#endif // PLAYLISTMODEL_H
