#include "mainwindow.h"
#include <QApplication>
#include "EndpointManager/EndpointManager.h"
#include "LibSpotifyIf/LibSpotifyIf.h"
#include "ClientHandler/ClientHandler.h"
#include "Platform/AudioEndpoints/AudioEndpointLocal.h"
#include "ConfigHandling/ConfigHandler.h"
#include "Platform/Utils/Utils.h"
#include "Platform/Timers/TimerFramework.h"

#include "Logger/applog.h"
#include "Logger/LoggerImpl.h"

bool simPacketDrop;

int main(int argc, char *argv[])
{
    int ret;
    QApplication a(argc, argv);

    Platform::initTimers();

    ConfigHandling::ConfigHandler ch("spotifyserver.conf");
    ch.parseConfigFile();

    Logger::LoggerImpl l(ch.getLoggerConfig());

    ConfigHandling::SpotifyConfig spConfig = ch.getSpotifyConfig();



    LibSpotify::LibSpotifyIf libspotifyif(spConfig);
    libspotifyif.logIn();

    EndpointId serverId( ch.getGeneralConfig() );
    Platform::AudioEndpointLocal audioEndpoint( ch.getAudioEndpointConfig(), serverId );
    EndpointManager epMgr( libspotifyif );
    epMgr.registerId( serverId );
    epMgr.createAudioEndpoint( audioEndpoint, NULL, NULL );
    epMgr.addAudioEndpoint( serverId.getId(), NULL, NULL );

    ClientHandler clienthandler( ch.getNetworkConfig(), libspotifyif, epMgr );

    MainWindow w( QStringLiteral("Server"), libspotifyif, epMgr );
    w.show();

    ret = a.exec();


    libspotifyif.logOut();
    sleep_ms( 2000 ); // todo wait for spotify to log out

    /* cleanup */
    libspotifyif.destroy();
    clienthandler.destroy();
    audioEndpoint.destroy();

    Platform::deinitTimers();

    log(LOG_DEBUG) << "Exit";
    return ret;
}
