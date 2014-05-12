#ifndef GLOBALACTIONINTERFACE_H
#define GLOBALACTIONINTERFACE_H

#include <QAction>

class IGlobalActionCallbacks
{
public:
    virtual void browseArtist( std::string link ) = 0;
    virtual void browseAlbum( std::string link ) = 0;
    virtual void enqueueTrack( std::string link ) = 0;
};

class GlobalActionSlots : public QObject
{
    Q_OBJECT
public:
    GlobalActionSlots( IGlobalActionCallbacks& cb_ );
public slots:
    void browseArtist();
    void browseAlbum();
    void enqueueTrack();

private:
    IGlobalActionCallbacks& cb;
};

#endif // GLOBALACTIONINTERFACE_H
