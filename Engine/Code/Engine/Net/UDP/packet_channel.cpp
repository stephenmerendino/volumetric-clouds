#include "Engine/Net/UDP/packet_channel.hpp"
#include "Engine/Net/UDP/udp_socket.hpp"
#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/session.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"
#include <algorithm>

PacketChannel::PacketChannel(UDPSocket* socket)
	:m_socket(socket)
	,m_packet_loss(0.5f)
	,m_min_lag_ms(80.0f)
	,m_max_lag_ms(120.0f)
{
}

PacketChannel::~PacketChannel()
{
	SAFE_DELETE(m_socket);
}

bool PacketChannel::bind(uint16_t port)
{
	return m_socket->bind(port);
}

bool PacketChannel::is_socket_valid()
{
	if(nullptr == m_socket){
		return false;
	}

	return m_socket->is_valid();
}

unsigned int PacketChannel::send(const net_address_t& addr, NetPacket* packet)
{
	return m_socket->send(addr, packet->m_payload, packet->m_payload_bytes_used);
}

bool PacketChannel::receive(NetPacket** out_packet)
{
	NetPacket* packet = new NetPacket();
    int bytes_received = m_socket->receive(&packet->m_sender, packet->m_payload, PACKET_MTU); 

	// packet loss
    if(bytes_received > 0){
		packet->m_payload_bytes_used = bytes_received;

		float process_chance = GetRandomFloatZeroToOne();
		if(process_chance < m_packet_loss){
			SAFE_DELETE(packet);
			*out_packet = nullptr;
			return false;
		}

		float packet_lag_ms = GetRandomFloatInRange(m_min_lag_ms, m_max_lag_ms);
		float timestamp = ((float)get_current_time_seconds() * 1000.0f) + packet_lag_ms;
		packet->m_timestamp = timestamp;

		// keep list in ascending order
		m_lag_packet_queue.push_back(packet);
		std::sort(m_lag_packet_queue.begin(), m_lag_packet_queue.end(), [](NetPacket* a, NetPacket* b) {
			return *a < *b;
		});
	} else{
		SAFE_DELETE(packet);
	}

	if(m_lag_packet_queue.empty()){
		SAFE_DELETE(packet);
		return false;
	}

	// copy over front packet if time is past
	float current_time_ms = (float)get_current_time_seconds() * 1000.0f;
	NetPacket* front = m_lag_packet_queue.front();
	if(front->m_timestamp < current_time_ms){
		*out_packet = front;
		//out_packet->reset();
		//memcpy(out_packet->m_payload, front->m_payload, front->m_payload_bytes_used);
		//out_packet->m_payload_bytes_used = front->m_payload_bytes_used;
		//out_packet->m_sender = front->m_sender;
		//out_packet->m_timestamp = front->m_timestamp;

		// clean up packet
		//SAFE_DELETE(front);
		m_lag_packet_queue.erase(m_lag_packet_queue.begin());

	    return true;
	} else{
		// no packet is ready yet, we need to sim more lag
		return false;
	}
}

void PacketChannel::set_loss(float loss)
{
	m_packet_loss = Min(loss, 1.0f);
}

void PacketChannel::set_lag(float min_lag_ms, float max_lag_ms)
{
	m_min_lag_ms = Max(min_lag_ms, 0.0f);
	m_min_lag_ms = Min(min_lag_ms, m_max_lag_ms);
	m_max_lag_ms = Max(m_min_lag_ms, max_lag_ms);
}

net_address_t PacketChannel::get_socket_address()
{
	if(nullptr == m_socket){
		return net_address_t();
	}

	return m_socket->m_address;
}