#pragma once

#include "Engine/Core/BinaryStream.hpp"
#include "Engine/Core/types.h"
#include "Engine/Net/net_address.hpp"

class NetSession;
class NetConnection;
class NetMessageDefinition;

#define MAX_PAYLOAD_SIZE 1024

class NetMessage : public BinaryStream
{
    public:
        uint8_t m_message_type_id;
		float m_sent_time; // special workaround for udp clock
		uint16_t m_reliable_id;
		uint16_t m_sequence_id;

        NetConnection* m_sender;
		net_address_t m_sender_address;

        byte_t m_payload[MAX_PAYLOAD_SIZE];
        size_t m_payload_bytes_used;
        size_t m_payload_bytes_read;

		float m_last_sent_time;

    public:
        NetMessage();
        NetMessage(uint8_t msg_type_id);
        NetMessage(const NetMessage& copy);

    	virtual unsigned int write_bytes(void* bytes, unsigned int count) override;
    	virtual unsigned int read_bytes(void* out_bytes, unsigned int count) override;

        void write_string(const char* string);
        bool read_string(char* out_string, unsigned int max_size);
        bool has_read_all_data() const;

        void write_to(u8* dest, NetSession* session);
        u16 get_body_size(NetSession* session);
        u16 get_full_size(NetSession* session);

        //void reset();
};