#pragma once

#include "Engine/Net/net_address.hpp"
#include "Engine/Core/types.h"
#include <queue>

class NetMessage;
class NetSession;
class NetPacket;

#define GUID_LENGTH 128

class NetConnection
{
    public:
        NetSession* m_owner;
		char m_guid[GUID_LENGTH];
        net_address_t m_address;
        u8 m_connection_index;

    public:
        NetConnection();
        NetConnection(u8 conn_idx, const std::string& guid, const std::string& address);
        NetConnection(u8 conn_idx, const std::string& guid, const net_address_t& net_addr);
        virtual ~NetConnection();

        virtual void send(NetMessage *msg) = 0;
        virtual bool receive(NetMessage **msg) = 0;
        virtual bool is_disconnected() const = 0;
		virtual void update(float ds) = 0;
		virtual void process_packet(NetPacket* packet) = 0;

		void set_guid(const std::string guid);
		bool has_guid(const std::string& guid);
		bool has_address(const std::string& address);
		bool has_address(const net_address_t& address);
};