#include "Engine/Net/TCP/tcp_connection.hpp"
#include "Engine/Net/TCP/tcp_socket.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Core/Common.hpp"

TCPConnection::TCPConnection()
    :NetConnection()
    ,m_socket(nullptr)
    ,m_msg_size_bytes_received(0)
    ,m_msg_type_bytes_received(0)
    ,m_msg_payload_bytes_received(0)
{
}

TCPConnection::~TCPConnection()
{
    SAFE_DELETE(m_socket);
}

void TCPConnection::send(NetMessage *msg)
{
    // calculate the message size
    uint16_t message_size = (uint16_t)(msg->m_payload_bytes_used + 1); // data + message_id

    // send the message size and message id
    m_socket->send(&message_size, sizeof(message_size));
    m_socket->send(&msg->m_message_type_id, sizeof(msg->m_message_type_id));

    // send the actual message data
    if(0 < msg->m_payload_bytes_used){
        m_socket->send(msg->m_payload, msg->m_payload_bytes_used);
    }

    delete msg;
}

bool TCPConnection::receive(NetMessage **msg)
{
    bool msg_size_received = receive_msg_size();
    if(!msg_size_received){
        return false;
    }

    bool msg_type_received = receive_msg_type();
    if(!msg_type_received){
        return false;
    }

    bool msg_payload_received = receive_msg_payload();
    if(!msg_payload_received){
        return false;
    }

    *msg = new NetMessage();
    apply_buffer_to_msg(*msg);
    return true;
}

bool TCPConnection::receive_msg_size()
{
    if(has_received_msg_size()){
        return true;
    }

    // read message size
    uint16_t message_size_bytes_left = (uint16_t)(sizeof(m_msg_size_buffer) - m_msg_size_bytes_received);
    receive_to_buffer(&m_msg_size_buffer, m_msg_size_bytes_received, message_size_bytes_left);

    return has_received_msg_size();
}

bool TCPConnection::receive_msg_type()
{
    if(has_received_msg_type()){
        return true;
    }

    // read message type
    uint16_t message_type_bytes_left = (uint16_t)(sizeof(m_msg_type_buffer) - m_msg_type_bytes_received);
    receive_to_buffer(&m_msg_type_buffer, m_msg_type_bytes_received, message_type_bytes_left);

    return has_received_msg_type();
}

bool TCPConnection::receive_msg_payload()
{
    if(has_received_msg_payload()){
        return true;
    }

    // read message payload
    uint16_t payload_bytes_left = (uint16_t)(m_msg_size_buffer - sizeof(m_msg_type_buffer) - m_msg_payload_bytes_received);
    receive_to_buffer(m_msg_payload_buffer, m_msg_payload_bytes_received, payload_bytes_left);

    return has_received_msg_payload();
}

void TCPConnection::apply_buffer_to_msg(NetMessage* msg)
{
    // set message type id
    msg->m_message_type_id = m_msg_type_buffer;

    // memcpy payload
    memcpy(msg->m_payload, m_msg_payload_buffer, m_msg_payload_bytes_received);
    msg->m_payload_bytes_used = m_msg_payload_bytes_received;

    // set sender
    msg->m_sender = this;

    // reset tracking vars
    m_msg_size_bytes_received = 0;
    m_msg_type_bytes_received = 0;
    m_msg_payload_bytes_received = 0;
}

bool TCPConnection::connect()
{
    m_socket->join(m_address);
    m_socket->set_blocking(false);
    return m_socket->is_valid();
}

bool TCPConnection::is_disconnected() const
{
    if(nullptr == m_socket){
        return true;
    }

    m_socket->check_for_disconnect();

    return (false == m_socket->is_valid());
}

void TCPConnection::update(float ds)
{
	UNUSED(ds);
}

void TCPConnection::process_packet(NetPacket* packet)
{
	UNUSED(packet);
}

bool TCPConnection::has_received_msg_size() const
{
    return (m_msg_size_bytes_received == sizeof(m_msg_size_buffer));
}

bool TCPConnection::has_received_msg_type() const
{
    return (m_msg_type_bytes_received == sizeof(m_msg_type_buffer));
}

bool TCPConnection::has_received_msg_payload() const
{
    if(!has_received_msg_size()){
        return false;
    }

    size_t payload_size = m_msg_size_buffer - sizeof(m_msg_type_buffer);
    return (m_msg_payload_bytes_received == payload_size);
}

void TCPConnection::receive_to_buffer(void* buffer, size_t& num_bytes_received_so_far, unsigned int num_bytes_left_to_receive)
{
    unsigned int bytes_recv = m_socket->receive((byte_t*)buffer + num_bytes_received_so_far, num_bytes_left_to_receive);
    if(bytes_recv != 0){
        num_bytes_received_so_far += bytes_recv;
    }
}