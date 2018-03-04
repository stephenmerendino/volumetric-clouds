#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Net/session.hpp"
#include "Engine/Net/UDP/udp_connection.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <memory>

/*
struct packer_header_t
{
	u8 from_conn_index;
	u16 packet_ack;
	u16 last_received_ack;
	u16 prev_received_ack_bitfield;
	u8 reliable_bundle_count;
	u8 unreliable_bundle_count;
};
*/

#define FROM_CONN_INDEX_OFFSET 0
#define ACK_OFFSET 1
#define LAST_RECEIVED_ACK_OFFSET 3
#define PREV_RECEIVED_ACK_BITFIELD_OFFSET 5
#define RELIABLE_BUNDLE_COUNT_OFFSET 7
#define UNRELIABLE_BUNDLE_COUNT_OFFSET 8
#define PAYLOAD_START_OFFSET 9

NetPacket::NetPacket()
	:m_payload_bytes_used(PAYLOAD_START_OFFSET)
	,m_msg_cursor(nullptr)
{
	reset();
}

void NetPacket::reset()
{
	memset(m_payload, 0xFF, PACKET_MTU);

	set_from_conn_idx(INVALID_CONNECTION_INDEX);
	set_packet_ack(0xFFFF);
	set_reliable_bundle_count(0x00);
	set_unreliable_bundle_count(0x00);

	m_msg_cursor = m_payload + PAYLOAD_START_OFFSET;
	m_payload_bytes_used = PAYLOAD_START_OFFSET;
}

bool NetPacket::write(NetMessage* msg, NetSession* session)
{
	// write this net msg into the payload buffer IF there is enough room
	uint bytes_remaining = get_free_byte_count();
	uint msg_total_size = msg->get_full_size(session);
	if(msg_total_size > bytes_remaining){
		return false;
	}

	// increment payload size
	u8* dest = m_payload + m_payload_bytes_used;
	msg->write_to(dest, session);

	// track packet usage
	m_payload_bytes_used += msg_total_size;

	return true;
}

bool NetPacket::read(NetMessage** out_msg, NetSession* session)
{
	*out_msg = new NetMessage();

	// make sure we stil have messages to read
	u8 unrel_msg_count = get_unreliable_bundle_count();
	u8 rel_msg_count = get_reliable_bundle_count();
	if(unrel_msg_count == 0 && rel_msg_count == 0){
		return false;
	}

	// track using a temporary internal cursor
	u8* internal_cursor = m_msg_cursor;

	// get body size
	u16 body_size = *((u16*)internal_cursor);
	internal_cursor += sizeof(body_size);

	// get msg id
	u8 msg_id = *(u8*)(internal_cursor);
	(*out_msg)->m_message_type_id = msg_id;
	internal_cursor += sizeof(msg_id);
	body_size -= sizeof(msg_id);

	float timestamp = *((float*)internal_cursor);
	(*out_msg)->m_sent_time = timestamp;
	internal_cursor += sizeof(timestamp);
	body_size -= sizeof(timestamp);

	// get def
	NetMessageDefinition* def = session->get_message_definition(msg_id);
	if(nullptr == def){
		ASSERT_OR_DIE(true, "I shouldn't be here");
		m_msg_cursor = internal_cursor;
		return false;
	}

	// read in reliable and sequence ids if needed based on def
	if(def->is_reliable()){
		u16 reliable_id = *((u16*)internal_cursor);
		(*out_msg)->m_reliable_id = reliable_id;
		internal_cursor += sizeof(reliable_id);
		body_size -= sizeof(reliable_id);
	} else{
		(*out_msg)->m_reliable_id = INVALID_RELIABLE_ID;
	}

	if(def->is_in_order()){
		u16 sequence_id = *((u16*)(internal_cursor));
		(*out_msg)->m_sequence_id = sequence_id;
		internal_cursor += sizeof(sequence_id);
		body_size -= sizeof(sequence_id);
	} else{
		(*out_msg)->m_sequence_id = INVALID_SEQUENCE_ID;
	}

    // get payload
	if(body_size > 0){
	    u16 payload_size = body_size;
	    memcpy((*out_msg)->m_payload, internal_cursor, payload_size);
	    (*out_msg)->m_payload_bytes_used = payload_size;
	    // update actual cursor
	    internal_cursor += payload_size;
	}

    m_msg_cursor = internal_cursor;

    // update header for number of msg's left
	if(def->is_reliable()){
	    decrement_reliable_bundle_count();
	} else{
	    decrement_unreliable_bundle_count();
	}

    return true;
}

void NetPacket::increment_unreliable_bundle_count()
{
    u8 cur_bundle_count = get_unreliable_bundle_count();
    set_unreliable_bundle_count(cur_bundle_count + 1);
}

void NetPacket::decrement_unreliable_bundle_count()
{
    u8 cur_bundle_count = get_unreliable_bundle_count();
    set_unreliable_bundle_count(cur_bundle_count - 1);
}

void NetPacket::increment_reliable_bundle_count()
{
    u8 cur_bundle_count = get_reliable_bundle_count();
    set_reliable_bundle_count(cur_bundle_count + 1);
}

void NetPacket::decrement_reliable_bundle_count()
{
    u8 cur_bundle_count = get_reliable_bundle_count();
    set_reliable_bundle_count(cur_bundle_count - 1);
}

void NetPacket::set_from_conn_idx(u8 conn_idx)
{
    *((u8*)(m_payload + FROM_CONN_INDEX_OFFSET)) = conn_idx;
}

void NetPacket::set_packet_ack(u16 packet_ack)
{
    *((u16*)(m_payload + ACK_OFFSET)) = packet_ack;
}

void NetPacket::set_last_received_ack(u16 last_received_ack)
{
    *((u16*)(m_payload + LAST_RECEIVED_ACK_OFFSET)) = last_received_ack;
}

void NetPacket::set_prev_received_ack_bitfield(u16 bitfield)
{
    *((u16*)(m_payload + PREV_RECEIVED_ACK_BITFIELD_OFFSET)) = bitfield;
}

void NetPacket::set_reliable_bundle_count(u8 reliable_bundle_count)
{
    *((u8*)(m_payload + RELIABLE_BUNDLE_COUNT_OFFSET)) = reliable_bundle_count;
}

void NetPacket::set_unreliable_bundle_count(u8 unreliable_bundle_count)
{
    *((u8*)(m_payload + UNRELIABLE_BUNDLE_COUNT_OFFSET)) = unreliable_bundle_count;
}

u8 NetPacket::get_from_conn_idx()
{
    return *((u8*)(m_payload + FROM_CONN_INDEX_OFFSET));
}

u16 NetPacket::get_packet_ack()
{
    return *((u16*)(m_payload + ACK_OFFSET));
}

u16 NetPacket::get_last_received_ack()
{
    return *((u16*)(m_payload + LAST_RECEIVED_ACK_OFFSET));
}

u16 NetPacket::get_prev_received_ack_bitfield()
{
    return *((u16*)(m_payload + PREV_RECEIVED_ACK_BITFIELD_OFFSET));
}

u8 NetPacket::get_reliable_bundle_count()
{
    return *((u8*)(m_payload + RELIABLE_BUNDLE_COUNT_OFFSET));
}

u8 NetPacket::get_unreliable_bundle_count()
{
    return *((u8*)(m_payload + UNRELIABLE_BUNDLE_COUNT_OFFSET));
}

uint NetPacket::get_free_byte_count()
{
    return (uint)(PACKET_MTU - m_payload_bytes_used);
}

bool NetPacket::can_fit(NetMessage* msg, NetSession* session)
{
	uint free_byte_count = get_free_byte_count();
	uint msg_size = msg->get_full_size(session);
	return (msg_size <= free_byte_count);
}

bool NetPacket::operator<(const NetPacket& other) const
{
	return m_timestamp < other.m_timestamp;
}