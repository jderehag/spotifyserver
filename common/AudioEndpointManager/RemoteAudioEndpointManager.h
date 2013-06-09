/*
 * RemoteAudioEndpointManager.h
 *
 *  Created on: 2 Jun 2013
 *      Author: Jesse
 */

#ifndef REMOTEAUDIOENDPOINTMANAGER_H_
#define REMOTEAUDIOENDPOINTMANAGER_H_

#include "AudioEndpointManagerCtrlInterface.h"
#include "SocketHandling/Messenger.h"
class RemoteAudioEndpointManager : public AudioEndpointCtrlInterface, public IMessageSubscriber
{
private:
    Messenger& messenger_;

public:
    RemoteAudioEndpointManager( Messenger& m );
    virtual ~RemoteAudioEndpointManager();

    virtual void addEndpoint( Platform::AudioEndpoint& ep ,IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void removeEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void getEndpoints( IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    virtual void connectionState( bool up );
    virtual void receivedMessage( const Message* msg );
    virtual void receivedResponse( const Message* rsp, const Message* req, void* userData );
};

#endif /* REMOTEAUDIOENDPOINTMANAGER_H_ */
