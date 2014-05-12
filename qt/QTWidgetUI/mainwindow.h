#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTreeWidgetItem>
#include <QMainWindow>
#include <QTimer>
#include "GlobalActionInterface.h"
#include "TrackListModel.h"
#include "MediaInterface/MediaInterface.h"
#include "EndpointManager/EndpointManagerCtrlInterface.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow, public IMediaInterfaceCallbackSubscriber, public IGlobalActionCallbacks
{
    Q_OBJECT

public:
    explicit MainWindow( QString& title, MediaInterface &m, EndpointCtrlInterface &epMgr, QWidget *parent = 0 );
    ~MainWindow();


    /* Implements IMediaInterfaceCallbackSubscriber */
    virtual void connectionState( bool up );
    virtual void rootFolderUpdatedInd();
    virtual void playlistUpdatedInd( const std::string& link );
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress );
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume );

    virtual void getPlaylistsResponse( const Folder& rootfolder, void* userData );
    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getArtistResponse( const Artist& artist, void* userData );
    virtual void genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData );
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData );
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData );

    virtual void getCurrentAudioEndpointsResponse( const std::set<std::string>& endpoints, void* userData );


    /* Implements IGlobalActionCallbacks */
    virtual void browseAlbum( std::string link );
    virtual void browseArtist( std::string link );
    virtual void enqueueTrack( std::string link );

public slots:
    void updateGui();
    void progressUpdate();

private slots:
    void on_playButton_clicked();
    void on_prevButton_clicked();
    void on_nextButton_clicked();
    void on_playlistsTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_playlistsTree_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_actionShowEndpoints_triggered();
    void on_repeatButton_clicked(bool checked);
    void on_shuffleButton_clicked(bool checked);
    void on_actionBack_triggered();
    void on_actionForward_triggered();

private:
    void newPage( QWidget* page );

    Ui::MainWindow *ui;

    MediaInterface& m_;
    EndpointCtrlInterface& epMgr_;

    GlobalActionSlots actions;

    unsigned int progress_;
    QTimer progressTimer;
    bool isPlaying;
};

#endif // MAINWINDOW_H
