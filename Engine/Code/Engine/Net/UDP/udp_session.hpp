#pragma once

#include "Engine/Net/session.hpp"
#include "Engine/Core/interval.h"

class PacketChannel;
class NetMessage;
class NetPacket;
class UDPConnection;

class UDPSession : public NetSession
{
    public:
		PacketChannel* m_packet_channel;
		bool m_is_listening;

	public:
		net_address_t m_host_join_address;
		float m_discover_timeout;
		Interval m_discover_interval;

	public:
		u32 m_join_token;
		char* m_join_guid;
		float m_connecting_timeout;
		Interval m_connecting_interval;

    public:
        UDPSession();
        virtual ~UDPSession();

    public:
        virtual bool host(uint16_t port, uint16_t range = DEFAULT_PORT_ATTEMPT_RANGE) override;
        virtual bool join(const net_address_t& address) override;
        virtual void leave() override;
        virtual void update(float ds) override;

        virtual bool start_listening() override;
        virtual void stop_listening() override;
        virtual bool is_listening() override;

        virtual bool start(uint16_t port, uint16_t range = DEFAULT_PORT_ATTEMPT_RANGE) override;

    public:
        bool stop();
		bool is_bound();
		void send_message_direct(const net_address_t& dest_addr, NetMessage* msg) override;
		void send_message_direct(uint conn_idx, NetMessage* msg) override;
		void send_packet_immediately(uint conn_idx, NetMessage* msg) override;
		bool create_connection(u8 index, const std::string& guid, const std::string& address);
		bool create_connection(u8 index, const std::string& guid, const net_address_t& net_addr);

		void session_discover_update(float ds);
		void session_connecting_update(float ds);
		void session_joining_update(float ds);
		void session_ready_update(float ds);

		void handle_ping_msg(NetMessage* msg);
		void handle_pong_msg(NetMessage* msg);
		void handle_join_request_msg(NetMessage* msg);
		void handle_join_response_msg(NetMessage* msg);
		void handle_leave_msg(NetMessage* msg);

		void handle_net_tick(void* user_arg, NetConnection* conn);

		void process_incoming_packets();
		void process_connectionless_packet(NetPacket* packet);

		void set_packet_loss(float packet_loss);
		void set_packet_lag(float min_lag_ms, float max_lag_ms);
		void send_heartbeat(UDPConnection* udp_conn);

		void flush_connection(UDPConnection* conn);
};