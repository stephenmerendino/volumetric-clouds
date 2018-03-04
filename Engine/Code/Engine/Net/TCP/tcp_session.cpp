#include "Engine/Net/TCP/tcp_session.hpp"
#include "Engine/Net/TCP/tcp_connection.hpp"
#include "Engine/Net/TCP/tcp_socket.hpp"
#include "Engine/Net/loopback_connection.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Net/message_definition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/log.h"

TCPSession::TCPSession()
    :NetSession()
    ,m_listen_socket(nullptr)
{
    register_message(NETMSG_JOIN_RESPONSE, this, &TCPSession::on_join_response, false, true, true);
}

TCPSession::~TCPSession()
{
    leave();
}

bool TCPSession::start(uint16_t port, uint16_t range)
{
	UNUSED(port);
	UNUSED(range);
	return true;
}

bool TCPSession::host(uint16_t port, uint16_t range)
{
	UNUSED(range);

    ASSERT_RECOVERABLE(!is_running(), "Session is already running");

    m_my_connection = new LoopBackConnection(); 
	m_my_connection->m_address = get_my_address(port);
    m_my_connection->m_owner = this;

	join_connection( 0, m_my_connection );
	m_host_connection = m_my_connection;

	set_state(SESSION_READY);

    log_printf("Hosting session at address %s", net_address_to_string(m_host_connection->m_address).c_str());

	return true;
}

bool TCPSession::join(const net_address_t& address)
{
    log_printf("Joining session at address %s", net_address_to_string(address).c_str());

    TCPConnection* host = new TCPConnection();
    host->m_address = address;

    join_connection(0, host); // 0 for this class; 
    host->m_socket = new TCPSocket();
    host->m_owner = this;
    m_host_connection = host;

    // Try to connect to host
    if(!host->connect()) {
        log_printf("Failed to join session at address %s", net_address_to_string(address).c_str());
        leave();
        return false;
    }

    log_printf("Joined session at address %s", net_address_to_string(address).c_str());

    m_my_connection = new LoopBackConnection();
    m_my_connection->m_address = get_my_address(NET_DEFAULT_PORT);
    m_my_connection->m_owner = this;

    set_state(SESSION_JOINING);
    return true;
}

void TCPSession::leave()
{
    if(m_host_connection != nullptr){
        log_printf("Leaving session at address %s", net_address_to_string(m_host_connection->m_address).c_str());
    }

    destroy_connection(m_my_connection);
    destroy_connection(m_host_connection);

    for(unsigned int i = 0; i < m_connections.size(); ++i) {
        destroy_connection(m_connections[i]);
    }

    stop_listening();

    set_state(SESSION_DISCONNECTED);
}

void TCPSession::update(float ds)
{
	UNUSED(ds);

    // accept new connections if listening
    if (is_listening()){
		TCPSocket *socket = m_listen_socket->accept();
		if(nullptr != socket){
			TCPConnection* new_guy = new TCPConnection();
			new_guy->m_socket = socket; 
            new_guy->m_address = socket->m_address;

			uint8_t conn_idx = get_free_connection_index();
			if (conn_idx == INVALID_CONNECTION_INDEX) {
				delete new_guy; 
			} else {
				join_connection(conn_idx, new_guy);
                new_guy->m_socket->set_blocking(false);

                log_printf("New connection [%i] from [%s]", conn_idx, net_address_to_string(new_guy->m_address).c_str());

				send_join_info(new_guy);
                m_connection_joined_event->trigger(new_guy);
			}
		}
	}

    // process messages
    for(unsigned int i = 0; i < m_connections.size(); i++){
        NetConnection* conn = m_connections[i];

        if(nullptr == conn){
            continue;
        }

        NetMessage* msg;
        while(conn->receive(&msg)){
            process_message(msg);
            delete msg;
        }
    }

    // handle disconnections
	for (unsigned int i = 0; i < m_connections.size(); ++i) {
		NetConnection* cp = m_connections[i];
		if ((nullptr != cp) && (cp != m_my_connection)) {
			TCPConnection *tcp_connection = (TCPConnection*)cp;
			if (tcp_connection->is_disconnected()) {
                log_printf("%s disconnected", net_address_to_string(tcp_connection->m_address).c_str());
                m_connection_left_event->trigger(tcp_connection);
				destroy_connection(tcp_connection); 
			}
		}
	}

    // leave if there is no longer a host
	if(is_running() && nullptr == m_host_connection) {
		leave(); 
        m_host_left_event->trigger();
	}
}

void TCPSession::process_message(NetMessage* msg)
{
    if(nullptr == msg){
        return;
    }

    NetMessageDefinition* def = get_message_definition(msg->m_message_type_id);
    ASSERT_OR_DIE(nullptr != def, Stringf("Could not find a definition for message id %i", msg->m_message_type_id));

    def->handle(msg);
}

void TCPSession::send_join_info(NetConnection* connection) 
{
	NetMessage* msg = new NetMessage(NETMSG_JOIN_RESPONSE);
	msg->write<uint8_t>(connection->m_connection_index);

	connection->send(msg); 
}

void TCPSession::on_join_response(NetMessage* msg)
{
    uint8_t my_conn_index;
    msg->read(my_conn_index);

    join_connection(my_conn_index, m_my_connection);
    set_state(SESSION_READY);

	m_session_joined_event->trigger(m_my_connection);
}

bool TCPSession::start_listening()
{
    if(!is_host()) {
        return false;
    }

    if(is_listening()) {
        return true;
    }

    m_listen_socket = new TCPSocket();
    if(m_listen_socket->listen(m_my_connection->m_address)) {
        m_listen_socket->set_blocking(false);
        return true;
    }
    else {
        delete m_listen_socket;
        m_listen_socket = nullptr;
        return false;
    }
}

void TCPSession::stop_listening()
{
    if(is_listening()) {
        delete m_listen_socket;
        m_listen_socket = nullptr;
    }
}

bool TCPSession::is_listening()
{
    return (nullptr != m_listen_socket);
}

unsigned int TCPSession::get_number_of_live_clients() const
{
    unsigned int number_live = 0;
    for(const NetConnection* c : m_connections){
        if(nullptr != c && c != m_host_connection){
            number_live++;
        }
    }

    return number_live;
}

void TCPSession::send_message_direct(const net_address_t& dest_addr, NetMessage* msg)
{
	//#TODO
	// not doing anything atm
	UNUSED(dest_addr);
	UNUSED(msg);
}

void TCPSession::send_message_direct(uint conn_idx, NetMessage* msg)
{
	send_message_to_index(conn_idx, *msg);
}

void TCPSession::send_packet_immediately(uint conn_idx, NetMessage* msg)
{
	send_message_to_index(conn_idx, *msg);
}