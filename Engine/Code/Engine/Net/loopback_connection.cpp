#include "Engine/Net/loopback_connection.hpp"
#include "Engine/Net/message.hpp"

LoopBackConnection::~LoopBackConnection()
{
}

void LoopBackConnection::send(NetMessage *msg)
{
    // enqueue message
    m_messages.push(msg);
}

bool LoopBackConnection::receive(NetMessage **msg)
{
    //dequeue message
    if(m_messages.empty()){
        *msg = nullptr;
        return false;
    }

    *msg = m_messages.front();
    (*msg)->m_sender = this;
    m_messages.pop();
    return true;
}

bool LoopBackConnection::is_disconnected() const
{
    return false;
}

void LoopBackConnection::update(float ds)
{
	UNUSED(ds);
}

void LoopBackConnection::process_packet(NetPacket* packet)
{
	UNUSED(packet);
}