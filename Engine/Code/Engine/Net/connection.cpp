#include "Engine/Net/connection.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Core/Common.hpp"

NetConnection::NetConnection()
    :m_owner(nullptr)
{
	MemZeroArray((char*)&m_guid[0], GUID_LENGTH);
}

NetConnection::NetConnection(u8 conn_idx, const std::string& guid, const std::string& address)
	:m_connection_index(conn_idx)
{
	set_guid(guid);
	get_net_address_from_string(&m_address, address, false);
}

NetConnection::NetConnection(u8 conn_idx, const std::string& guid, const net_address_t& net_addr)
	:m_connection_index(conn_idx)
	,m_address(net_addr)
{
	set_guid(guid);
}

NetConnection::~NetConnection()
{
}

void NetConnection::set_guid(const std::string guid)
{
	MemZeroArray((char*)&m_guid[0], GUID_LENGTH);
	memcpy(m_guid, guid.c_str(), guid.length());
}

bool NetConnection::has_guid(const std::string& guid)
{
	std::string this_guid = std::string(m_guid);
	return this_guid == guid;
}

bool NetConnection::has_address(const std::string& address)
{
	net_address_t comparand_addr;
	get_net_address_from_string(&comparand_addr, address, false);
	return m_address == comparand_addr;
}

bool NetConnection::has_address(const net_address_t& address)
{
	return m_address == address;
}