#include "Engine/Net/session.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Net/message_definition.hpp"
#include "Engine/Net/connection.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

NetSession::NetSession(unsigned int max_num_connections)
	:m_host_connection(nullptr)
	, m_my_connection(nullptr)
	, m_state(SESSION_INVALID)
	, m_max_num_connections(max_num_connections)
	, m_connection_joined_event(nullptr)
	, m_connection_left_event(nullptr)
	, m_host_left_event(nullptr)
{
	m_connection_joined_event = new Event<NetConnection*>();
	m_connection_left_event = new Event<NetConnection*>();
	m_net_tick_event = new Event<NetConnection*>();
    m_session_joined_event = new Event<NetConnection*>();
	m_host_left_event = new Event<>();
}

NetSession::~NetSession()
{
	for(unsigned int i = 0; i < m_message_definitions.size(); ++i){
		NetMessageDefinition*& def = m_message_definitions[i];
		delete def;
		def = nullptr;
	}

	m_message_definitions.clear();

	SAFE_DELETE(m_net_tick_event);
	SAFE_DELETE(m_connection_joined_event);
	SAFE_DELETE(m_connection_left_event);
	SAFE_DELETE(m_session_joined_event);
	SAFE_DELETE(m_host_left_event);
}

void NetSession::set_state(SessionState new_state)
{
	m_state = new_state;

	if(SESSION_DISCONNECTED == m_state){
		destroy_all_connections();
	}
}

bool NetSession::is_host() const
{
    return (nullptr != m_host_connection) && (m_host_connection == m_my_connection);
}

bool NetSession::is_client() const
{
    return (nullptr != m_my_connection) && (m_my_connection != m_host_connection);
}

bool NetSession::is_running() const
{
    return (nullptr != m_my_connection);
}

bool NetSession::is_ready() const
{
    return (SESSION_READY == m_state);
}

bool NetSession::register_message(uint8_t msg_id, net_function_t handler, bool is_connectionless, bool is_reliable, bool is_in_order)
{
    if(is_message_registered(msg_id)){
        return false;
    }

    NetMessageDefinition* new_def = new NetMessageDefinition();
    new_def->m_message_id = msg_id;
    new_def->m_provided_handler = (void*)handler;
    new_def->m_internal_handler = new_def->function_handler;
    new_def->set_is_connectionless(is_connectionless);
    new_def->set_is_reliable(is_reliable);
    new_def->set_is_in_order(is_in_order);

    m_message_definitions.push_back(new_def);

    return true;
}

void NetSession::unregister_message(uint8_t msg_id)
{
    for(unsigned int i = 0; i < m_message_definitions.size(); ++i){
        NetMessageDefinition* def = m_message_definitions[i];
        if(def->m_message_id == msg_id){
            m_message_definitions[i] = m_message_definitions.back();
            m_message_definitions.pop_back();
            delete def;
            return;
        }
    }
}

NetMessageDefinition* NetSession::get_message_definition(uint8_t msg_id) const
{
    for(unsigned int msg_def_count = 0; msg_def_count < m_message_definitions.size(); ++msg_def_count){
        NetMessageDefinition* def = m_message_definitions[msg_def_count];
        if((nullptr != def) && (msg_id == def->m_message_id)){
            return def;
        }
    }

    return nullptr;
}

bool NetSession::is_message_registered(uint8_t msg_id) const
{
    return (nullptr != get_message_definition(msg_id));
}

bool NetSession::is_message_reliable(uint8_t msg_id) const
{
	if(!is_message_registered(msg_id)){
		return false;
	}

	NetMessageDefinition* def = get_message_definition(msg_id);
	return def->is_reliable();
}

bool NetSession::is_message_in_order(uint8_t msg_id) const
{
	if(!is_message_registered(msg_id)){
		return false;
	}

	NetMessageDefinition* def = get_message_definition(msg_id);
	return def->is_in_order();
}

bool NetSession::is_message_connectionless(uint8_t msg_id) const
{
	if(!is_message_registered(msg_id)){
		return false;
	}

	NetMessageDefinition* def = get_message_definition(msg_id);
	return def->is_connectionless();
}

uint8_t	NetSession::get_number_of_live_connections() const
{
	uint8_t count = 0;

    for(uint conn_index = 0; conn_index < m_connections.size(); ++conn_index){
        if(nullptr != m_connections[conn_index]){
			++count;
        }
    }

	return count;
}

void NetSession::set_max_connections(u8 max_num_connections)
{
	m_max_num_connections = max_num_connections;
	m_connections.resize(max_num_connections);
}

uint8_t NetSession::get_free_connection_index() const
{
    // look for an existing free slot
    unsigned int conn_index;
    for(conn_index = 0; conn_index < m_connections.size(); ++conn_index){
        if(nullptr == m_connections[conn_index]){
            return (uint8_t)conn_index;
        }
    }

    // make sure there is enough room left for a new free slot
    if(conn_index < m_max_num_connections){
        return (uint8_t)conn_index;
    }else{
        return INVALID_CONNECTION_INDEX;
    }
}

bool NetSession::has_free_connection_slots() const
{
    for(uint conn_index = 0; conn_index < m_connections.size(); ++conn_index){
        if(nullptr == m_connections[conn_index]){
            return true;
        }
    }

	return false;
}

#include "Engine/Core/log.h"

void NetSession::join_connection(u8 index, NetConnection* new_connection)
{
    new_connection->m_connection_index = index;

    // Make sure index is either a new push back or that the existing slot is null
    ASSERT_OR_DIE((index >= m_connections.size()) || (m_connections[index] == nullptr), Stringf("Invalid join index %i for connection", index).c_str());

    if(index >= m_connections.size()){
        m_connections.resize(index + 1);
    }

    m_connections[index] = new_connection;
}

void NetSession::destroy_connection(u8 connection_index)
{
	NetConnection* conn = get_connection_by_index(connection_index);
	if(nullptr != conn){
		destroy_connection(conn);
	}
}

void NetSession::destroy_connection(NetConnection* connection)
{
    if(nullptr == connection) {
        return;
    }

    if(m_my_connection == connection) {
        m_my_connection = nullptr;
    }

    if(m_host_connection == connection) {
        m_host_connection = nullptr;
    }

    if(INVALID_CONNECTION_INDEX != connection->m_connection_index) {
        m_connections[connection->m_connection_index] = nullptr;
        connection->m_connection_index = INVALID_CONNECTION_INDEX;
    }

    delete connection;
}

NetConnection* NetSession::get_connection_by_index(uint8_t connection_index)
{
    if(connection_index < m_connections.size()){
        return m_connections[connection_index];
    }

    return nullptr;
}

NetConnection* NetSession::get_connection_by_guid(const std::string& guid)
{
	for(uint conn_idx = 0; conn_idx < m_connections.size(); ++conn_idx){
		NetConnection* conn = m_connections[conn_idx];
		if(nullptr == conn){
			continue;
		}

		if(conn->has_guid(guid)){
			return conn;
		}
	}

	return nullptr;
}

NetConnection* NetSession::get_connection_by_address(const std::string& address)
{
	for(uint conn_idx = 0; conn_idx < m_connections.size(); ++conn_idx){
		NetConnection* conn = m_connections[conn_idx];
		if(nullptr == conn){
			continue;
		}

		if(conn->has_address(address)){
			return conn;
		}
	}

	return nullptr;
}

NetConnection* NetSession::get_connection_by_address(const net_address_t& address)
{
	for(uint conn_idx = 0; conn_idx < m_connections.size(); ++conn_idx){
		NetConnection* conn = m_connections[conn_idx];
		if(nullptr == conn){
			continue;
		}

		if(conn->has_address(address)){
			return conn;
		}
	}

	return nullptr;
}

void NetSession::send_message_to_others(NetMessage const &msg)
{
    for(unsigned int index = 0; index < m_connections.size(); ++index){
        if(m_connections[index] != m_my_connection){
            send_message_to_index(index, msg);
        }
    }
}

void NetSession::send_message_to_index(unsigned int index, NetMessage const &msg)
{
    NetConnection* conn = m_connections[index];
    if(nullptr != conn){
        conn->send(new NetMessage(msg));
    }
}

void NetSession::send_message_to_all(NetMessage const &msg)
{
    for(unsigned int index = 0; index < m_connections.size(); ++index){
        send_message_to_index(index, msg);
    }
}

void NetSession::send_message_to_all_clients_but_index(NetMessage const &msg, unsigned int excluded_index)
{
    for(unsigned int index = 0; index < m_connections.size(); ++index){
        if(index != excluded_index && index != m_host_connection->m_connection_index){
            send_message_to_index(index, msg);
        }
    }
}

void NetSession::send_message_to_host(NetMessage const &msg)
{
    if(nullptr != m_host_connection){
        m_host_connection->send(new NetMessage(msg));
    }
}

std::string	NetSession::get_host_address_string()
{
	if(nullptr == m_host_connection){
		return "";
	}

	return net_address_to_string(m_host_connection->m_address);
}

void NetSession::destroy_all_connections()
{
	for(unsigned int index = 0; index < m_connections.size(); ++index){
		destroy_connection((u8)index);
	}
}