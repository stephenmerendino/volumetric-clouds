#include "Engine/Net/UDP/udp_session.hpp"
#include "Engine/Net/UDP/udp_socket.hpp"
#include "Engine/Net/UDP/udp_connection.hpp"
#include "Engine/Net/UDP/packet_channel.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Net/message_definition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/log.h"

#define HEARTBEAT_TIME 1.0f
#define DISCOVER_TIMEOUT 30.0f
#define CONNECTING_TIMEOUT 30.0f
#define DEFAULT_UDP_PORT 1919
#define HEALTHY_TIME 3.0f
#define TIMEOUT_TIME 30.0f

UDPSession::UDPSession()
	:m_packet_channel(nullptr)
	,m_is_listening(false)
{
	register_message(NETMSG_HEARTBEAT, nullptr, false, false, false);
	register_message<UDPSession>(NETMSG_PING, this, &UDPSession::handle_ping_msg, true, false, false);
	register_message<UDPSession>(NETMSG_PONG, this, &UDPSession::handle_pong_msg, true, false, false);
	register_message<UDPSession>(NETMSG_JOIN_REQUEST, this, &UDPSession::handle_join_request_msg, true, false, false);
	register_message<UDPSession>(NETMSG_JOIN_RESPONSE, this, &UDPSession::handle_join_response_msg, true, false, false);
	register_message<UDPSession>(NETMSG_LEAVE, this, &UDPSession::handle_leave_msg, false, false, false);

	m_discover_interval.set_frequency(4);
	m_connecting_interval.set_frequency(4);
}

UDPSession::~UDPSession()
{
    SAFE_DELETE(m_packet_channel);
}

void UDPSession::handle_ping_msg(NetMessage* msg)
{
	// host receives a ping from a person trying to discover the host
	NetMessage response(NETMSG_PONG);

	response.write(m_host_connection->m_connection_index);
	response.write(m_max_num_connections);

	send_message_direct(msg->m_sender_address, &response);

	std::string sender_addr = net_address_to_string(msg->m_sender_address);
	log_printf("Received ping. Sending pong back to joinee at %s", sender_addr.c_str());
}

void UDPSession::handle_pong_msg(NetMessage* msg)
{
	if(SESSION_DISCOVER != m_state){
		return;
	}

	// client receives a response back from the host
	u8 host_idx;
	msg->read(host_idx);
	msg->read(m_max_num_connections);

	m_join_token = (u32)rand();

	set_state(SESSION_CONNECTING);

	create_connection(host_idx, "host", msg->m_sender_address);
	m_host_connection = m_connections[host_idx];

	log_printf("Generated join token: %i", m_join_token);

	m_connecting_timeout = DISCOVER_TIMEOUT;
	m_connecting_interval.reset();

	log_printf("Received pong from host [host_conn_index: %i]", host_idx);
}

void UDPSession::handle_join_request_msg(NetMessage* msg)
{
	if(SESSION_READY != m_state){
		return;
	}

	// host receives a message from client trying to join
	u32 join_token;
	msg->read(join_token);

	log_printf("Join token: %i", join_token);

	// #TODO
	//char guid[32];
	//msg->read_string(guid, 32);

	// check current connections to see if we have a connection with this token
	for(uint i = 0; i < m_connections.size(); i++){
		NetConnection* conn = m_connections[i];
		if(nullptr == conn){
			continue;
		}

		UDPConnection* udp_conn = static_cast<UDPConnection*>(conn);
		if(join_token == udp_conn->m_join_token){
			log_printf("%i - %i", i, join_token);
			NetMessage accept(NETMSG_JOIN_RESPONSE);
			accept.write(true);
			accept.write(udp_conn->m_connection_index);
			send_message_direct(msg->m_sender_address, &accept);

			log_printf("Received join request. Already had connection, sending an accept response");

			return;
		}
	}

	// can't accept new connections
	if(!is_listening() || !has_free_connection_slots()){
		NetMessage accept(NETMSG_JOIN_RESPONSE);
		accept.write(false);
		send_message_direct(msg->m_sender_address, &accept);
		log_printf("Received join request. Can't join, sending a deny");
		return;
	}

	// if we can, create connection and send accept
	u8 new_conn_index = get_free_connection_index();
	UDPConnection* new_conn = new UDPConnection(new_conn_index, "todo_guid", msg->m_sender_address);
	new_conn->m_owner = this;
	new_conn->m_join_token = join_token;
	new_conn->m_connection_index = new_conn_index;
	join_connection(new_conn_index, new_conn);

	// send success
	NetMessage accept(NETMSG_JOIN_RESPONSE);
	accept.write(true);
	accept.write(new_conn_index);
	send_message_direct(msg->m_sender_address, &accept);

	m_connection_joined_event->trigger(new_conn);

	log_printf("Received join request. Created connection and sending an accept response");
}

void UDPSession::handle_join_response_msg(NetMessage* msg)
{
	if(SESSION_CONNECTING != m_state){
		return;
	}

	// client receives message back from host for if they successfully joined
	bool success;
	msg->read(success);

	if(!success){
		leave();
		log_warningf("Failed to join");
		return;
	}

	u8 conn_idx;
	msg->read(conn_idx);

	UDPConnection* conn = new UDPConnection(conn_idx, "todo_guid", m_packet_channel->get_socket_address());
	conn->m_owner = this;
	conn->m_join_token = m_join_token;
	join_connection(conn_idx, conn);
	m_my_connection = conn;

	set_state(SESSION_JOINING);

	log_printf("Received join response from host.");

	m_connection_joined_event->trigger(conn);
}

void UDPSession::handle_leave_msg(NetMessage* msg)
{
	u8 conn_index;
	msg->read(conn_index);

	m_connection_left_event->trigger(m_connections[conn_index]);
	destroy_connection(conn_index);

	log_printf("Received leave message for conn index %i", conn_index);
}

bool UDPSession::host(uint16_t port, uint16_t range)
{
	if(SESSION_DISCONNECTED != m_state && SESSION_INVALID != m_state){
		return false;
	}

	if(!is_bound()){
		start(port, range);
	}

	net_address_t net_addr = m_packet_channel->get_socket_address();
	create_connection(0, "host", net_addr);
	m_host_connection = m_connections[0];
	m_my_connection = m_connections[0];

	set_state(SESSION_READY);

	return true;
}

bool UDPSession::join(const net_address_t& address)
{
	if(SESSION_INVALID == m_state){
		bool started = start(DEFAULT_UDP_PORT);
		if(!started){
			return false;
		}
	}

	if(SESSION_DISCONNECTED != m_state){
		return false;
	}

	set_state(SESSION_DISCOVER);
	m_host_join_address = address;
	m_discover_timeout = DISCOVER_TIMEOUT;
	m_discover_interval.reset();

    return true;
}

void UDPSession::leave()
{
	if(nullptr == m_my_connection){
		set_state(SESSION_DISCONNECTED);
		return;
	}

	NetMessage* leave = new NetMessage(NETMSG_LEAVE);
	leave->write(m_my_connection->m_connection_index);

	for(uint i = 0; i < m_connections.size(); i++){
		if(nullptr != m_connections[i]){
			send_packet_immediately(i, leave);
			flush_connection(static_cast<UDPConnection*>(m_connections[i]));
		}
	}

	set_state(SESSION_DISCONNECTED);
}

void UDPSession::update(float ds)
{
	switch(m_state){
		case SESSION_DISCOVER: session_discover_update(ds); break;
		case SESSION_CONNECTING: session_connecting_update(ds); break;
		case SESSION_JOINING: session_joining_update(ds); break;
		case SESSION_READY: session_ready_update(ds); break;
	}
}

void UDPSession::session_discover_update(float ds)
{
	// handle timeout
	m_discover_timeout -= ds;
	if(m_discover_timeout <= 0.0f){
		log_warningf("discover timed out...");
		leave();
		return;
	}

	if(m_discover_interval.check_and_reset()){
		// ping host
		NetMessage msg(NETMSG_PING);
		send_message_direct(m_host_join_address, &msg);
		log_printf("sending ping to host...");
	}

	process_incoming_packets();
}

void UDPSession::session_connecting_update(float ds)
{
	m_connecting_timeout -= ds;
	if(m_connecting_timeout <= 0.0f){
		log_warningf("connecting timed out...");
		leave();
		return;
	}

	if(m_connecting_interval.check_and_reset()){
		// ping host
		NetMessage msg(NETMSG_JOIN_REQUEST);
		msg.write(m_join_token);
		//#TODO guid
		//msg.write_string(m_join_guid);
		send_message_direct(m_host_connection->m_connection_index, &msg);
		log_printf("sending join request to host...");
	}

	process_incoming_packets();
}

void UDPSession::session_joining_update(float ds)
{
	UNUSED(ds);
	set_state(SESSION_READY);
	log_printf("Fully connected to host.");
	//#TODO inform everyone that I'm ready?
	m_session_joined_event->trigger(m_my_connection);
}

void UDPSession::session_ready_update(float ds)
{
	if(nullptr == m_host_connection){
		leave();
		return;
	}

	for(uint conn_idx = 0; conn_idx < m_connections.size(); ++conn_idx){
		NetConnection* conn = m_connections[conn_idx];
		if(nullptr != conn){
			UDPConnection* udp_conn = static_cast<UDPConnection*>(conn);
			udp_conn->update(ds);

			// timeout connections that aren't mine
			if(udp_conn != m_my_connection){
				if(udp_conn->m_time_since_last_recv < HEALTHY_TIME || 
				   udp_conn->m_time_since_last_sent > HEARTBEAT_TIME){
					send_heartbeat(udp_conn);
				}

				if(udp_conn->m_time_since_last_recv > TIMEOUT_TIME){
					destroy_connection(udp_conn);
				}
			}
		}
	}

	process_incoming_packets();
}

void UDPSession::process_incoming_packets()
{
    // handle incoming messages appropriately
	NetPacket* packet = nullptr;// = new NetPacket();
    while(m_packet_channel->receive(&packet)){

		u8 from_index = packet->get_from_conn_idx();
		if(INVALID_CONNECTION_INDEX == from_index){
			process_connectionless_packet(packet);
		} else{
			NetConnection* conn = get_connection_by_index(from_index);
			if(nullptr != conn){
				conn->process_packet(packet);
			}
		}
		SAFE_DELETE(packet);
    }

	SAFE_DELETE(packet);
}

void UDPSession::process_connectionless_packet(NetPacket* packet)
{
	// pull out messages, set their sender address, process
    NetMessage* msg;
    while(packet->read(&msg, this)){
		msg->m_sender = nullptr;
		msg->m_sender_address = packet->m_sender;

		if(is_message_connectionless(msg->m_message_type_id)){
		    NetMessageDefinition* def = get_message_definition(msg->m_message_type_id);
		    ASSERT_OR_DIE(nullptr != def, Stringf("Could not find a definition for message id %i", msg->m_message_type_id));
		    def->handle(msg);
		}

		SAFE_DELETE(msg);
    }

	SAFE_DELETE(msg);
}

bool UDPSession::start_listening()
{
	m_is_listening = true;
    return true;
}

void UDPSession::stop_listening()
{
	m_is_listening = false;
}

bool UDPSession::is_listening()
{
	return m_is_listening;
}

bool UDPSession::start(uint16_t port, uint16_t range)
{
    if(nullptr != m_packet_channel){
        return false;
    }

    UDPSocket* socket = new UDPSocket();

    unsigned int attempts_left = range;
    while(attempts_left > 0){
        if(socket->bind(port)){
			m_packet_channel = new PacketChannel(socket);
			m_net_tick_event->subscribe_method(nullptr, this, &UDPSession::handle_net_tick);
			set_state(SESSION_DISCONNECTED);
            return true;
        }else{
            attempts_left--;
            port++;
        }
    }

    SAFE_DELETE(socket);
    return false;
}

bool UDPSession::stop()
{
	if(SESSION_INVALID == m_state){
		return false;
	}

	SAFE_DELETE(m_packet_channel);
	set_state(SESSION_INVALID);

	return true;
}

bool UDPSession::is_bound()
{
	return (nullptr != m_packet_channel) && (m_packet_channel->is_socket_valid());
}

void UDPSession::send_message_direct(const net_address_t& dest_addr, NetMessage* msg)
{
    // build a netpacket and send directly to address using socket
    NetPacket packet;

	//UDPConnection* udp_conn = static_cast<UDPConnection*>(m_my_connection);
	//udp_conn->append_packet_header(&packet);
    packet.write(msg, this);
	packet.set_from_conn_idx(INVALID_CONNECTION_INDEX);
	packet.set_unreliable_bundle_count(1);

    m_packet_channel->send(dest_addr, &packet);
}

void UDPSession::send_message_direct(uint conn_idx, NetMessage* msg)
{
	UDPConnection* dest = static_cast<UDPConnection*>(m_connections[conn_idx]);
	send_message_direct(dest->m_address, msg);
}

void UDPSession::send_packet_immediately(uint conn_idx, NetMessage* msg)
{
	UDPConnection* dest = static_cast<UDPConnection*>(m_connections[conn_idx]);
	dest->send(new NetMessage(*msg));
	NetPacket* packet = dest->build_packet_to_send();

	if(nullptr != packet){
		m_packet_channel->send(dest->m_address, packet);
		dest->m_time_since_last_sent = 0.0f;
		SAFE_DELETE(packet);
	}
}

bool UDPSession::create_connection(u8 index, const std::string& guid, const std::string& address)
{
	net_address_t net_addr;
	get_net_address_from_string(&net_addr, address, false);
	return create_connection(index, guid, net_addr);
}

bool UDPSession::create_connection(u8 index, const std::string& guid, const net_address_t& net_addr)
{
	if(nullptr != get_connection_by_index(index)){
		return false;
	}

	if(nullptr != get_connection_by_guid(guid)){
		return false;
	}

	if(nullptr != get_connection_by_address(net_addr)){
		return false;
	}

	UDPConnection* new_conn = new UDPConnection(index, guid, net_addr);
	new_conn->m_owner = this;

	join_connection(index, new_conn);

	if(new_conn->m_address == m_packet_channel->m_socket->m_address){
		m_my_connection = new_conn;
	}

	return true;
}

void UDPSession::handle_net_tick(void* user_arg, NetConnection* conn)
{
	UNUSED(user_arg);

	UDPConnection* udp_conn = static_cast<UDPConnection*>(conn);
	ASSERT_OR_DIE(nullptr != udp_conn, "Failed to cast to UDPConnection*");

	NetPacket* packet = udp_conn->build_packet_to_send();
	if(nullptr != packet){
	    m_packet_channel->send(udp_conn->m_address, packet);
		udp_conn->m_time_since_last_sent = 0.0f;
		SAFE_DELETE(packet);
	}
}

void UDPSession::set_packet_loss(float packet_loss)
{
	m_packet_channel->m_packet_loss = packet_loss;
}

void UDPSession::set_packet_lag(float min_lag_ms, float max_lag_ms)
{
	m_packet_channel->set_lag(min_lag_ms, max_lag_ms);
}

void UDPSession::send_heartbeat(UDPConnection* udp_conn)
{
	NetMessage* heartbeat = new NetMessage(NETMSG_HEARTBEAT);
	udp_conn->send(heartbeat);
}

void UDPSession::flush_connection(UDPConnection* conn)
{
	NetPacket* packet = conn->build_packet_to_send();
	while(nullptr != packet){
	    m_packet_channel->send(conn->m_address, packet);
		SAFE_DELETE(packet);
	}
}