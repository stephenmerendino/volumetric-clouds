#pragma once

#include "Engine/Net/session.hpp"
#include "Engine/Net/TCP/tcp_connection.hpp"

class TCPSocket;

class TCPSession : public NetSession
{
    public:
        TCPSocket* m_listen_socket;

    public:
        TCPSession();
        virtual ~TCPSession();

    public:
        virtual bool host(uint16_t port, uint16_t range = DEFAULT_PORT_ATTEMPT_RANGE) override;
        virtual bool join(const net_address_t& address) override;
        virtual void leave() override;
        virtual void update(float ds) override;
		virtual bool start(uint16_t port, uint16_t range = DEFAULT_PORT_ATTEMPT_RANGE);

        virtual bool start_listening() override;
        virtual void stop_listening() override;
        virtual bool is_listening() override;

		void send_message_direct(const net_address_t& dest_addr, NetMessage* msg) override;
		void send_message_direct(uint conn_idx, NetMessage* msg) override;
		void send_packet_immediately(uint conn_idx, NetMessage* msg) override;

    public:
        void process_message(NetMessage* msg);
        void send_join_info(NetConnection* connection);
        void on_join_response(NetMessage* msg);

        unsigned int get_number_of_live_clients() const;
};