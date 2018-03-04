#pragma once

#include "Engine/Net/net_address.hpp"
#include "Engine/Core/types.h"

class NetMessage;
class NetSession;

#define PACKET_MTU 1452

class NetPacket
{
    public:
        net_address_t m_sender;

        byte m_payload[PACKET_MTU];
        uint m_payload_bytes_used;
        byte* m_msg_cursor;

		float m_timestamp;

    public:
        NetPacket(); 

		void reset();
        bool write(NetMessage* msg, NetSession* session);
        bool read(NetMessage** out_msg, NetSession* session);

        u8 get_from_conn_idx();
        u16 get_packet_ack();
		u16 get_last_received_ack();
		u16 get_prev_received_ack_bitfield();
        u8 get_reliable_bundle_count();
        u8 get_unreliable_bundle_count();

        void set_from_conn_idx(u8 conn_idx);
        void set_packet_ack(u16 packet_ack);
		void set_last_received_ack(u16 last_received_ack);
		void set_prev_received_ack_bitfield(u16 bitfield);
        void set_reliable_bundle_count(u8 reliable_bundle_count);
        void set_unreliable_bundle_count(u8 unreliable_bundle_count);

        void increment_unreliable_bundle_count();
        void decrement_unreliable_bundle_count();
        void increment_reliable_bundle_count();
        void decrement_reliable_bundle_count();

        uint get_free_byte_count();

		bool can_fit(NetMessage* msg, NetSession* session);

		bool operator<(const NetPacket& other) const;
};