#pragma once

#include "Engine/Net/connection.hpp"
#include <queue>

class NetMessage;

class LoopBackConnection : public NetConnection
{
    public:
        std::queue<NetMessage*> m_messages;

    public:
        virtual ~LoopBackConnection();

        virtual void send(NetMessage *msg) override;
        virtual bool receive(NetMessage **msg) override;
        virtual bool is_disconnected() const override;
        virtual void update(float ds) override;
		virtual void process_packet(NetPacket* packet) override;
};