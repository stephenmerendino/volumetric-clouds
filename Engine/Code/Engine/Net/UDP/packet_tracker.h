#pragma once

#include "Engine/Core/types.h"
#include <vector>

class PacketTracker
{
	public:
		u16 m_packet_ack_id;
		std::vector<u16> m_reliable_ids;
		bool m_confirmed;

	public:
		PacketTracker();
};