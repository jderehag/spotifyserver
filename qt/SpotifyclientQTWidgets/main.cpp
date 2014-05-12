#include "mainwindow.h"
#include <QApplication>
#include "SocketHandling/SocketClient.h"
#include "RemoteMediaInterface.h"
#include "UIConsole.h"
#include "Platform/Utils/Utils.h"
#include "Platform/AudioEndpoints/AudioEndpointLocal.h"
#include "EndpointManager/RemoteEndpointManager.h"
#include "Platform/Timers/TimerFramework.h"

#include "Logger/applog.h"
#include "Logger/LoggerImpl.h"


int main(int argc, char *argv[])
{
    int ret;
    QApplication a(argc, argv);

    Platform::initTimers();

    ConfigHandling::ConfigHandler ch("spotifyclient.conf");
    ch.parseConfigFile();

    ConfigHandling::LoggerConfig cfg = ch.getLoggerConfig();
    //cfg.setLogTo(ConfigHandling::LoggerConfig::STDOUT);
    Logger::LoggerImpl l(cfg);

    EndpointId clientId( ch.getGeneralConfig() );
    Platform::AudioEndpointLocal audioEndpoint(ch.getAudioEndpointConfig(), clientId );

    SocketClient sc( ch.getNetworkConfig(), clientId );
    RemoteMediaInterface m(sc);

    RemoteEndpointManager epMgr( sc, clientId );
    epMgr.createAudioEndpoint(audioEndpoint, NULL, NULL);


    {
        MainWindow w( QStringLiteral("Client"), m, epMgr );
        w.show();

        ret = a.exec();
    }


    /* cleanup */
    sc.destroy();
    audioEndpoint.destroy();

    Platform::deinitTimers();

    log(LOG_DEBUG) << "Exit";
    return ret;
}
