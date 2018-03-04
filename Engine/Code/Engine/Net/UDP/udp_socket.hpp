#pragma once

#include "Engine/Net/net.hpp"
#include "Engine/Net/net_address.hpp"

class NetPacket;

class UDPSocket
{
    public:
        SOCKET m_socket;
        net_address_t m_address;

    public:
        bool bind(uint16_t port);
		bool is_valid();

        unsigned int send(const net_address_t& addr, const void* payload, unsigned int payload_size_bytes);
        unsigned int receive(net_address_t* out_send_addr, void* payload, unsigned int max_payload_size_bytes);
};