#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStandardItemModel>
#include <QMainWindow>
#include "MediaInterface/MediaInterface.h"
#include "EndpointManager/EndpointManagerCtrlInterface.h"

namespace Ui {
class MainWindow;
}

QVariant toString ( const Track& t, int column );

class TrackListModel : public QAbstractItemModel
{
private:
    std::deque<Track> tracks;
public:
    TrackListModel() {}
    void setTrackList( const std::deque<Track>& tracks_ ) { beginResetModel(); tracks = tracks_; endResetModel(); }
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const { return 2; }
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const { return tracks.size(); }
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const { if (!index.isValid() || role != Qt::DisplayRole) return QVariant(); else return toString( tracks[index.row()], index.column()); }
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const { if( column >= 2 || row >= tracks.size()) return QModelIndex(); else return createIndex(row, column, (void*)&tracks[row]);}
    virtual QModelIndex parent ( const QModelIndex & index ) const { return QModelIndex(); }
};

class MainWindow : public QMainWindow, public IMediaInterfaceCallbackSubscriber, public IEndpointCtrlCallbackSubscriber
{
    Q_OBJECT

public:
    explicit MainWindow(MediaInterface &m, EndpointCtrlInterface &epMgr, QWidget *parent = 0);
    ~MainWindow();


    /* Implements IMediaInterfaceCallbackSubscriber */
    virtual void connectionState( bool up );
    virtual void rootFolderUpdatedInd();
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress );
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume );

    virtual void getPlaylistsResponse( const Folder& rootfolder, void* userData );
    virtual void getTracksResponse( const std::deque<Track>& tracks, void* userData );
    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getAlbumResponse( const Album& album, void* userData );
    virtual void getArtistResponse( const Artist& artist, void* userData );
    virtual void genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData );
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData );
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData );

    virtual void getCurrentAudioEndpointsResponse( const std::set<std::string>& endpoints, void* userData );

    /* Implements IEndpointCtrlCallbackSubscriber */
    virtual void renameEndpointResponse( void* userData );
    virtual void getEndpointsResponse( const EndpointInfoList& endpoints, void* userData );

    virtual void getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData );
    virtual void audioEndpointsUpdatedNtf();

public slots:
    void updateGui();

private slots:
    void on_treeView_clicked(const QModelIndex &index);

    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_playButton_clicked();

    void on_prevButton_clicked();

    void on_nextButton_clicked();

private:
    Ui::MainWindow *ui;

    MediaInterface& m_;
    EndpointCtrlInterface& epMgr_;
    QStandardItemModel model;
    TrackListModel tracksModel;

    bool isPlaying;
};

#endif // MAINWINDOW_H
