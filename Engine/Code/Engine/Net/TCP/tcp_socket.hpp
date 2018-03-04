#pragma once

#include "Engine/Net/net.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Core/Common.hpp"
#include <inttypes.h>

class NetMessage;

class TCPSocket
{
    public:
        SOCKET m_socket;
        net_address_t m_address;
        bool m_is_listen_socket;

    public:
        TCPSocket();
        ~TCPSocket();

        // client
        bool join(net_address_t address);
        bool join(const char* address);
        void close();

        // host
        bool listen(uint16_t port);
        bool listen(net_address_t net_addr);
        bool is_listening() const;
        TCPSocket* accept();

        // both
        unsigned int send(const void* payload, unsigned int payload_size_bytes);
        unsigned int receive(void* payload, unsigned int max_payload_size_bytes);

        void set_blocking(bool is_blocking);
        void check_for_disconnect();
        bool is_valid() const;
};