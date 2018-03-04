#include "Engine/Net/message.hpp"
#include "Engine/Net/message_definition.hpp"
#include "Engine/Net/session.hpp"
#include "Engine/Net/connection.hpp"
#include "Engine/Core/types.h"
#include "Engine/Core/Time.hpp"
#include "Engine/Net/UDP/udp_connection.hpp"

NetMessage::NetMessage()
    :m_sender(nullptr)
	,m_reliable_id(INVALID_RELIABLE_ID)
	,m_sequence_id(INVALID_SEQUENCE_ID)
    ,m_payload_bytes_used(0)
    ,m_payload_bytes_read(0)
	,m_sent_time(0.0f)
{
    m_stream_order = LITTLE_ENDIAN;
    memset(m_payload, 0, MAX_PAYLOAD_SIZE);
}

NetMessage::NetMessage(uint8_t msg_type_id)
    :m_message_type_id(msg_type_id)
	,m_reliable_id(INVALID_RELIABLE_ID)
	,m_sequence_id(INVALID_SEQUENCE_ID)
    ,m_sender(nullptr)
    ,m_payload_bytes_used(0)
    ,m_payload_bytes_read(0)
	,m_sent_time(0.0f)
{
    m_stream_order = LITTLE_ENDIAN;
    memset(m_payload, 0, MAX_PAYLOAD_SIZE);
}

NetMessage::NetMessage(const NetMessage& copy)
    :m_message_type_id(copy.m_message_type_id)
	,m_reliable_id(copy.m_reliable_id)
	,m_sequence_id(copy.m_sequence_id)
    ,m_sender(copy.m_sender)
    ,m_payload_bytes_used(copy.m_payload_bytes_used)
    ,m_payload_bytes_read(0)
	,m_sent_time(copy.m_sent_time)
{
    m_stream_order = LITTLE_ENDIAN;
    memcpy(m_payload, copy.m_payload, m_payload_bytes_used);
}

unsigned int NetMessage::write_bytes(void* bytes, unsigned int count)
{
    if(m_payload_bytes_used + count > MAX_PAYLOAD_SIZE){
        return 0;
    }

    byte_t* write_location = m_payload + m_payload_bytes_used;
    memcpy(write_location, bytes, count);

    m_payload_bytes_used += count;

    return count;
}

unsigned int NetMessage::read_bytes(void* out_bytes, unsigned int count)
{
    memcpy(out_bytes, m_payload + m_payload_bytes_read, count);
    m_payload_bytes_read += count;
    return count;
}

void NetMessage::write_string(const char* string)
{
    if(nullptr == string){
        write<uint16_t>(0xFFFF);
        return;
    }

    size_t string_length = strlen(string);    
    write<uint16_t>((uint16_t)string_length);

    if(string_length > 0){
        write_endian_aware_bytes((void*)string, string_length);
    }
}

bool NetMessage::read_string(char* out_string, unsigned int max_size)
{
    if(has_read_all_data()){
        return nullptr;
    }

    uint16_t string_length;
    read<uint16_t>(string_length);

    if(string_length > max_size){
        string_length = (uint16_t)max_size;
    }

    if(0xFFFF == string_length){
        memset(out_string, 0, max_size);
        return false;
    }else if(0 == string_length){
        memset(out_string, 0, max_size);
        return false;
    }else{
        read(out_string, string_length);
        out_string[string_length] = '\0';
        return true;
    }
}

bool NetMessage::has_read_all_data() const
{
    return (m_payload_bytes_read >= m_payload_bytes_used);
}

void NetMessage::write_to(u8* dest, NetSession* session)
{
    u16 body_size = get_body_size(session);
    memcpy(dest, &body_size, sizeof(body_size));
    dest += sizeof(body_size);

    memcpy(dest, &m_message_type_id, sizeof(m_message_type_id));
    dest += sizeof(m_message_type_id);

	m_sent_time = (float)get_current_time_seconds();
	memcpy(dest, &m_sent_time, sizeof(m_sent_time));
	dest += sizeof(m_sent_time);

	NetMessageDefinition* def = session->get_message_definition(m_message_type_id);

	if((nullptr != def) && (def->is_reliable())){
	    memcpy(dest, &m_reliable_id, sizeof(m_reliable_id));
	    dest += sizeof(m_reliable_id);
	}

	if((nullptr != def) && (def->is_in_order())){
	    memcpy(dest, &m_sequence_id, sizeof(m_sequence_id));
	    dest += sizeof(m_sequence_id);
	}

    memcpy(dest, m_payload, m_payload_bytes_used);
}

u16 NetMessage::get_body_size(NetSession* session)
{
	NetMessageDefinition* def = session->get_message_definition(m_message_type_id);

	u16 header_size = sizeof(m_message_type_id);
	header_size += sizeof(m_sent_time);

	if((nullptr != def) && (def->is_reliable())){
		header_size += sizeof(m_reliable_id);
	}

	if((nullptr != def) && (def->is_in_order())){
		header_size += sizeof(m_sequence_id);
	}

    return (u16)(header_size + m_payload_bytes_used);
}

u16 NetMessage::get_full_size(NetSession* session)
{
    u16 body_size = get_body_size(session);
    return body_size + sizeof(body_size); // the header of the message is the body size
}

//void NetMessage::reset()
//{
//    m_sender = nullptr;
//    m_payload_bytes_used = 0;
//    m_payload_bytes_read = 0;
//    memset(m_payload, 0, MAX_PAYLOAD_SIZE);
//}