#pragma once

#include "Engine/Net/connection.hpp"
#include "Engine/Core/interval.h"
#include "Engine/Math/cycle.h"

#define DEFAULT_UDP_TICK_FREQ 20
#define RELIABLE_TIME_BEFORE_RESEND 0.150f

#define MAX_ACK_ID 63354
#define INVALID_ACK_ID 0xffff

#define MAX_RELIABLE_ID 65534
#define INVALID_RELIABLE_ID 0xffff
#define RELIABLE_ID_WINDOW 1024

#define MAX_SEQUENCE_ID 65534
#define INVALID_SEQUENCE_ID 0xffff

#define PACKET_TRACKER_BUFFER_SIZE 256

class UDPSession;
class PacketTracker;

class UDPConnection : public NetConnection
{
	public:
		Interval m_tick_interval;
		Cycle m_ack_cycle;
		Cycle m_reliable_id_cycle;
		Cycle m_send_sequence_id_cycle;
		Cycle m_recv_sequence_id_cycle;

		std::queue<NetMessage*> m_unsent_unreliables;
		std::vector<NetMessage*> m_unsent_reliables;
		std::vector<NetMessage*> m_sent_but_unconfirmed_reliables;
		std::vector<NetMessage*> m_waiting_in_order_messages;
		std::vector<u16> m_processed_reliables;

		std::vector<PacketTracker> m_packet_trackers;

		u16 m_next_send_ack;
		u16 m_last_ack_received;
		u16 m_last_ack_sent;
		u16 m_previous_acks_bitfield;

		u16 m_oldest_unconfirmed_reliable_id;
		u16 m_last_reliable_id_sent;
		u16 m_last_reliable_id_received;

		f32 m_time_since_last_recv;
		f32 m_time_since_last_sent;

		u32 m_join_token;

    public:
        UDPConnection();
        UDPConnection(u8 conn_idx, const std::string& guid, const std::string& address);
        UDPConnection(u8 conn_idx, const std::string& guid, const net_address_t& net_addr);
        virtual ~UDPConnection();

        virtual void send(NetMessage *msg) override;
        virtual bool receive(NetMessage **msg) override;
        virtual bool is_disconnected() const override;
		virtual void process_packet(NetPacket* packet) override;

	public:
		void init_state();
		void process_message(NetMessage* msg);
		void update(float ds);

		void append_packet_header(NetPacket* p);
		NetPacket* consolidate_packet();
		NetPacket* build_packet_to_send();

		u16 get_next_send_ack();
		void confirm_acks(u16 received_packet_ack, u32 received_prev_ack_bitfield);
		void confirm_tracker(u16 packet_ack);
		void process_received_packet_ack(u16 received_packet_ack);
		void shift_ack_bitfield(u16 latest_packet_ack);

		PacketTracker& get_tracker_for_ack(u16 packet_ack);
		void confirm_reliable(u16 reliable_id);

		bool has_reliable_been_processed(u16 reliable_id);

		u16 get_live_reliable_count();
		void purge_old_reliable_ids_processed();

		void process_in_order_message(NetMessage* msg);
		void process_waiting_in_order_messages();
		void sort_waiting_in_order_messages();
};