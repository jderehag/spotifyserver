/*
 * AudioEndpointRemotePeer.h
 *
 *  Created on: 14 nov 2012
 *      Author: Jesse
 */

#ifndef AUDIOENDPOINTREMOTEPEER_H_
#define AUDIOENDPOINTREMOTEPEER_H_

#include "ClientHandler/SocketPeer.h"
#include "Platform/AudioEndpoints/AudioEndpointLocal.h"

class AudioEndpointRemotePeer : public SocketPeer
{
    Platform::AudioEndpointLocal endpoint_;

    virtual void processMessage(const Message* msg);

public:
    AudioEndpointRemotePeer( Socket* socket, ConfigHandling::AudioEndpointConfig& config );
    virtual ~AudioEndpointRemotePeer();
};

#endif /* AUDIOENDPOINTREMOTEPEER_H_ */
