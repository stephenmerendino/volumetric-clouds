#pragma once

#include "Engine/Net/net.hpp"
#include "Engine/Net/net_address.hpp"
#include <vector>

class UDPSocket;
class NetPacket;

class PacketChannel
{
	public:
		UDPSocket* m_socket;
		float m_packet_loss;
		float m_min_lag_ms;
		float m_max_lag_ms;

		std::vector<NetPacket*> m_lag_packet_queue;

	public:
		PacketChannel(UDPSocket* socket);
		~PacketChannel();

        bool bind(uint16_t port);
		bool is_socket_valid();

		unsigned int send(const net_address_t& addr, NetPacket* packet);
        bool receive(NetPacket** out_packet);

		void set_loss(float loss);
		void set_lag(float min_lag_ms, float max_lag_ms);

		net_address_t get_socket_address();
};