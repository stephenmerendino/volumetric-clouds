#pragma once

#include "Engine/Net/connection.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Core/Common.hpp"

class TCPSocket;
class NetMessage;

class TCPConnection : public NetConnection
{
    public:
        TCPSocket* m_socket;

        uint16_t m_msg_size_buffer;
        size_t m_msg_size_bytes_received;

        uint8_t m_msg_type_buffer;
        size_t m_msg_type_bytes_received;

        byte_t m_msg_payload_buffer[MAX_PAYLOAD_SIZE];
        size_t m_msg_payload_bytes_received;

    public:
        TCPConnection();
        virtual ~TCPConnection();

        virtual void send(NetMessage *msg) override;
        virtual bool receive(NetMessage **out_msg) override;
        virtual bool is_disconnected() const override;
		virtual void update(float ds) override;
		virtual void process_packet(NetPacket* packet) override;

        bool connect();

        bool receive_msg_size();
        bool receive_msg_type();
        bool receive_msg_payload();
        void apply_buffer_to_msg(NetMessage* msg);

        bool has_received_msg_size() const;
        bool has_received_msg_type() const;
        bool has_received_msg_payload() const;

        void receive_to_buffer(void* buffer, size_t& num_bytes_received_so_far, unsigned int num_bytes_left_to_receive);
};