#include "Engine/Net/UDP/udp_connection.hpp"
#include "Engine/Net/UDP/udp_session.hpp"
#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/message_definition.hpp"
#include "Engine/Net/UDP/packet_tracker.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/bit.h"
#include "Engine/Core/log.h"
#include "Engine/Core/Time.hpp"

UDPConnection::UDPConnection()
{
	init_state();
}

UDPConnection::UDPConnection(u8 conn_idx, const std::string& guid, const std::string& address)
	:NetConnection(conn_idx, guid, address)
{
	init_state();
}

UDPConnection::UDPConnection(u8 conn_idx, const std::string& guid, const net_address_t& net_addr)
	:NetConnection(conn_idx, guid, net_addr)
{
	init_state();
}

UDPConnection::~UDPConnection()
{
}

void UDPConnection::init_state()
{
	m_tick_interval.set_frequency(DEFAULT_UDP_TICK_FREQ);
	m_ack_cycle.set_range(0, MAX_ACK_ID);
	m_reliable_id_cycle.set_range(0, MAX_RELIABLE_ID);
	m_send_sequence_id_cycle.set_range(0, MAX_SEQUENCE_ID);
	m_recv_sequence_id_cycle.set_range(0, MAX_SEQUENCE_ID);
	m_packet_trackers.resize(PACKET_TRACKER_BUFFER_SIZE);

	m_next_send_ack = 0;
	m_last_ack_received = INVALID_ACK_ID;
	m_last_ack_sent = INVALID_ACK_ID;
	m_oldest_unconfirmed_reliable_id = INVALID_RELIABLE_ID;
	m_previous_acks_bitfield = 0;
	m_last_reliable_id_sent = 0;
	m_last_reliable_id_received = 0;

	m_time_since_last_recv = 0.0f;
	m_time_since_last_sent = 0.0f;
	m_connection_index = 0;
}

void UDPConnection::send(NetMessage *msg)
{
	if(m_owner->is_message_reliable(msg->m_message_type_id)){
		if(m_owner->is_message_in_order(msg->m_message_type_id)){
			msg->m_sequence_id = (u16)m_send_sequence_id_cycle.get_current();
			m_send_sequence_id_cycle.increment();
		}
		m_unsent_reliables.push_back(msg);
	} else{
		m_unsent_unreliables.push(msg);
	}
}

bool UDPConnection::receive(NetMessage **msg)
{
	UNUSED(msg);
	return false;
}

bool UDPConnection::is_disconnected() const
{
	return false;
}

void UDPConnection::process_packet(NetPacket* packet) 
{
	if(nullptr == packet){
		return;
	}

	m_time_since_last_recv = 0.0f;

	process_received_packet_ack(packet->get_packet_ack());

	u16 received_packet_ack = packet->get_last_received_ack();
	u32 received_prev_ack_bitfield = packet->get_prev_received_ack_bitfield();
	confirm_acks(received_packet_ack, received_prev_ack_bitfield);

    NetMessage* msg = nullptr;
    while(packet->read(&msg, m_owner)){
		msg->m_sender = this;
		msg->m_sender_address = m_address;

		if(m_owner->is_message_in_order(msg->m_message_type_id)){
			process_in_order_message(msg);
		} else{
	        process_message(msg);
		}

		SAFE_DELETE(msg);
		//msg = new NetMessage();
    }
}

void UDPConnection::process_in_order_message(NetMessage* msg)
{
	if(msg->m_sequence_id == m_recv_sequence_id_cycle.get_current()){
        process_message(msg);
		m_recv_sequence_id_cycle.increment();
		process_waiting_in_order_messages();
	} else{
		for(uint i = 0; i < m_waiting_in_order_messages.size(); i++){
			if(m_waiting_in_order_messages[i]->m_sequence_id == msg->m_sequence_id){
				return;
			}
		}

		if(has_reliable_been_processed(msg->m_reliable_id)){
			return;
		}

		m_waiting_in_order_messages.push_back(new NetMessage(*msg));
		sort_waiting_in_order_messages();
	}
}

void UDPConnection::process_message(NetMessage* msg)
{
    if(nullptr == msg){
        return;
    }

    NetMessageDefinition* def = m_owner->get_message_definition(msg->m_message_type_id);
    ASSERT_OR_DIE(nullptr != def, Stringf("Could not find a definition for message id %i", msg->m_message_type_id));

	// don't process connectionless messages unless we know the sender
	if(!def->is_connectionless() && nullptr == msg->m_sender){
		//log_printf("Received non-connectionless message for unknown connection");
		return;
	}

	if(def->is_reliable()){
		if(m_reliable_id_cycle.is_greater(msg->m_reliable_id, m_last_reliable_id_received)){
			m_last_reliable_id_received = msg->m_reliable_id;
			purge_old_reliable_ids_processed();
		}

		if(has_reliable_been_processed(msg->m_reliable_id)){
			return;
		} else{
			m_processed_reliables.push_back(msg->m_reliable_id);
		}
	}

    def->handle(msg);
}

void UDPConnection::update(float ds)
{
	UNUSED(ds);

	m_time_since_last_recv += ds;
	m_time_since_last_sent += ds;

	if(m_tick_interval.check_and_reset()){
		m_owner->m_net_tick_event->trigger(this);
	}
}

void UDPConnection::append_packet_header(NetPacket* p)
{
	if(nullptr == p){
		return;
	}

	m_last_ack_sent = get_next_send_ack();
	p->set_packet_ack(m_last_ack_sent);
	p->set_last_received_ack(m_last_ack_received);
	p->set_prev_received_ack_bitfield(m_previous_acks_bitfield);

	if(nullptr != m_owner->m_my_connection){
		p->set_from_conn_idx(m_owner->m_my_connection->m_connection_index);
	}
}

NetPacket* UDPConnection::consolidate_packet()
{
	if(m_unsent_unreliables.empty() && 
	   m_unsent_reliables.empty() && 
	   m_unsent_reliables.empty())
	{
		return nullptr;
	}

	NetPacket* packet = new NetPacket();
	append_packet_header(packet);

	//PacketTracker* tracker = new PacketTracker(packet->get_packet_ack());
	PacketTracker& tracker = get_tracker_for_ack(packet->get_packet_ack());
	tracker.m_packet_ack_id = packet->get_packet_ack();
	tracker.m_reliable_ids.clear();
	tracker.m_confirmed = false;

	// resend old but unconfirmed reliables
	for(uint sent_rel_idx = 0; sent_rel_idx < m_sent_but_unconfirmed_reliables.size(); ++sent_rel_idx){
		NetMessage* sent_rel_msg = m_sent_but_unconfirmed_reliables[sent_rel_idx];
		if(nullptr == sent_rel_msg){
			continue;
		}

		float now = (float)get_current_time_seconds();
		float time_since_sent = now - sent_rel_msg->m_last_sent_time;

		if(time_since_sent >= RELIABLE_TIME_BEFORE_RESEND && packet->can_fit(sent_rel_msg, m_owner)){
			packet->write(sent_rel_msg, m_owner);
			packet->increment_reliable_bundle_count();
			sent_rel_msg->m_last_sent_time = now;
			tracker.m_reliable_ids.push_back(sent_rel_msg->m_reliable_id);
		}
	}

	// try to fit any unsent reliables in
	for(uint unsent_rel_idx = 0; unsent_rel_idx < m_unsent_reliables.size(); ++unsent_rel_idx){
		NetMessage* unsent_rel_msg = m_unsent_reliables[unsent_rel_idx];
		if(nullptr == unsent_rel_msg){
			continue;
		}

		u16 next_reliable_id = (u16)m_reliable_id_cycle.get_current();
		int window_delta = m_reliable_id_cycle.get_abs_delta(next_reliable_id, m_oldest_unconfirmed_reliable_id);

		// make sure we have a valid oldeset unconfirmed reliable id and if we do that we are within our window
		if(INVALID_RELIABLE_ID == m_oldest_unconfirmed_reliable_id){
			m_oldest_unconfirmed_reliable_id = next_reliable_id;
		} else if(window_delta > RELIABLE_ID_WINDOW){
			break;
		}

		// if it can't fit, there's no point in even going further
		if(!packet->can_fit(unsent_rel_msg, m_owner)){
			continue;
		}

		// set the reliable id appropriately
		unsent_rel_msg->m_reliable_id = next_reliable_id;
		m_last_reliable_id_sent = unsent_rel_msg->m_reliable_id;
		m_reliable_id_cycle.increment();

		// set time sent stamp
		float now = (float)get_current_time_seconds();
		unsent_rel_msg->m_last_sent_time = now;
		
		// write to packet and track it in packet tracker
		packet->write(unsent_rel_msg, m_owner);
		packet->increment_reliable_bundle_count();
		tracker.m_reliable_ids.push_back(unsent_rel_msg->m_reliable_id);

		// put it in the correct list
		m_unsent_reliables.erase(m_unsent_reliables.begin() + unsent_rel_idx);
		m_sent_but_unconfirmed_reliables.push_back(unsent_rel_msg);
	}

	// resort based on last sent time oldest to newest
	std::sort(m_sent_but_unconfirmed_reliables.begin(), 
		      m_sent_but_unconfirmed_reliables.end(), 
		      [](NetMessage* a, NetMessage* b){
					return a->m_last_sent_time > b->m_last_sent_time;
			  });

	// try to append any unsent unreliables if possible, discard the rest
	while(!m_unsent_unreliables.empty()){
		NetMessage* msg = m_unsent_unreliables.front();
		m_unsent_unreliables.pop();

		if(packet->can_fit(msg, m_owner)){
			packet->write(msg, m_owner);
			packet->increment_unreliable_bundle_count();
		} 

		SAFE_DELETE(msg);
	}

	return packet;
}

NetPacket* UDPConnection::build_packet_to_send()
{
	NetPacket* packet = consolidate_packet();
	if(nullptr == packet){
		return nullptr;
	}

	return packet;
}

u16 UDPConnection::get_next_send_ack()
{
	u16 next_ack = (u16)m_ack_cycle.get_current();
	m_ack_cycle.increment();
	return next_ack;
}

void UDPConnection::process_received_packet_ack(u16 received_packet_ack)
{
	if(INVALID_ACK_ID == received_packet_ack){
		return;
	}

	// this is the first time we've received an ack, no reason to shift the bitfield any
	if(INVALID_ACK_ID == m_last_ack_received){
		m_last_ack_received = received_packet_ack;
		return;
	}

	ASSERT_OR_DIE(received_packet_ack != m_last_ack_received, "Received same ack twice in a row");

	shift_ack_bitfield(received_packet_ack);
}

void UDPConnection::confirm_acks(u16 received_packet_ack, u32 received_prev_ack_bitfield)
{
	// confirm acks
	confirm_tracker(received_packet_ack);
	for(uint bit_idx = 1; bit_idx <= sizeof(received_prev_ack_bitfield); ++bit_idx){
		// check if lowest bit is set, if so process ack based on the received packet ack
		u32 mask = BIT(0);
		if(IS_BIT_SET(received_prev_ack_bitfield, mask)){
			u16 prev_ack = (u16)(received_packet_ack - bit_idx);
			confirm_tracker(prev_ack);
		}

		// shift down
		received_prev_ack_bitfield >>= 1;
	}
}

void UDPConnection::confirm_tracker(u16 packet_ack)
{
	PacketTracker& tracker = get_tracker_for_ack(packet_ack);
	if(tracker.m_packet_ack_id != packet_ack){
		return;
	}

	if(tracker.m_confirmed){
		return;
	}

	tracker.m_confirmed = true;
	
	//log_printf("Packet Ack %i confirmed", packet_ack);

	for(uint rel_idx = 0; rel_idx < tracker.m_reliable_ids.size(); ++rel_idx){
		confirm_reliable(tracker.m_reliable_ids[rel_idx]);
	}
}

PacketTracker& UDPConnection::get_tracker_for_ack(u16 packet_ack)
{
	return m_packet_trackers[packet_ack % PACKET_TRACKER_BUFFER_SIZE];
}

void UDPConnection::confirm_reliable(u16 reliable_id)
{
	for(uint rel_idx = 0; rel_idx < m_sent_but_unconfirmed_reliables.size(); ++rel_idx){
		if(reliable_id == m_sent_but_unconfirmed_reliables[rel_idx]->m_reliable_id){
			//log_printf("    Reliable msg %i confirmed", reliable_id);
			SAFE_DELETE(m_sent_but_unconfirmed_reliables[rel_idx]);
			m_sent_but_unconfirmed_reliables.erase(m_sent_but_unconfirmed_reliables.begin() + rel_idx);
			return;
		}
	}
}

void UDPConnection::shift_ack_bitfield(u16 latest_packet_ack)
{
	if(INVALID_ACK_ID == latest_packet_ack){
		return;
	}

	u16 delta = (u16)m_ack_cycle.get_abs_delta(latest_packet_ack, m_last_ack_received);

	if(m_ack_cycle.is_greater(latest_packet_ack, m_last_ack_received)){
		m_last_ack_received = latest_packet_ack;
		m_previous_acks_bitfield = m_previous_acks_bitfield << delta;
	}

	u16 old_last_ack_mask = BIT(delta - 1);
	m_previous_acks_bitfield = SET_BIT(m_previous_acks_bitfield, old_last_ack_mask);
}

bool UDPConnection::has_reliable_been_processed(u16 reliable_id)
{
	for(uint rid = 0; rid < m_processed_reliables.size(); ++rid){
		if(reliable_id == m_processed_reliables[rid]){
			return true;
		}
	}

	return false;
}

u16 UDPConnection::get_live_reliable_count()
{
	return (u16)m_sent_but_unconfirmed_reliables.size();
}

void UDPConnection::purge_old_reliable_ids_processed()
{
	for(uint i = 0; i < m_processed_reliables.size(); i++){
		uint delta = m_reliable_id_cycle.get_abs_delta(m_last_reliable_id_received, m_processed_reliables[i]);
		if(delta > RELIABLE_ID_WINDOW){
			m_processed_reliables.erase(m_processed_reliables.begin() + i);
		}
	}
}

void UDPConnection::process_waiting_in_order_messages()
{
	while(!m_waiting_in_order_messages.empty()){
		NetMessage* msg = m_waiting_in_order_messages[0];
		if(msg->m_sequence_id == m_recv_sequence_id_cycle.get_current()){
			process_message(msg);
			SAFE_DELETE(msg);
			m_waiting_in_order_messages.erase(m_waiting_in_order_messages.begin());
			m_recv_sequence_id_cycle.increment();
		} else{
			return;
		}
	}
}

void UDPConnection::sort_waiting_in_order_messages()
{
	std::sort(m_waiting_in_order_messages.begin(), m_waiting_in_order_messages.end(), [](NetMessage* a, NetMessage* b){
		return a->m_sequence_id < b->m_sequence_id;
	});
}