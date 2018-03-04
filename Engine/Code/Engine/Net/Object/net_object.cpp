#include "Engine/Net/Object/net_object.hpp"
#include "Engine/Net/Object/net_object_type_definition.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/Time.hpp"
#include <stdlib.h>

NetObject::NetObject(NetObjectTypeDefinition *defn)
    :m_type_id(0)
    ,m_defn(defn)
    ,m_last_received_snapshot(nullptr)
    ,m_last_received_snapshot_client_timestamp(0.0f)
    ,m_current_snapshot(nullptr)
    ,m_is_local_dirty(false)
    ,m_snapshot_is_valid(false)
	,m_last_updated_timestamp(0.0f)
{
    m_current_snapshot = m_defn->create_snapshot();
    m_last_received_snapshot = m_defn->create_snapshot();
}

NetObject::~NetObject()
{
    free(m_current_snapshot);
    free(m_last_received_snapshot);

    std::map<uint8_t, net_connection_state_t*>::iterator it; 
    for(it = m_conn_states.begin(); it != m_conn_states.end(); it++){
        net_connection_state_t* conn = it->second;
        free(conn->m_last_sent_snapshot);
        SAFE_DELETE(conn);
    }
    m_conn_states.clear();
}

void NetObject::refresh_current_snapshot()
{
    m_defn->refresh_current_snapshot(m_current_snapshot, m_local_object);
}

void NetObject::append_snapshot(NetMessage* msg)
{
    m_defn->append_snapshot(msg, m_current_snapshot);
}

void NetObject::process_snapshot(NetMessage* msg)
{
    m_defn->process_snapshot(msg, m_last_received_snapshot);
    m_is_local_dirty = true;
    m_snapshot_is_valid = true;
}

void NetObject::apply_latest_snapshot()
{
    if(!m_snapshot_is_valid){
        return;
    }

    double dt = get_current_time_seconds() - m_last_received_snapshot_client_timestamp;
    if(dt <= 0.0f){
        return;
    }
    m_defn->apply_snapshot(m_last_received_snapshot, m_local_object, dt);
    m_is_local_dirty = false;
}

void NetObject::save_last_sent_snapshot(uint8_t conn_index, void* snapshot)
{
    net_connection_state_t* conn_state = get_or_create_conn_state(conn_index);
    memcpy(conn_state->m_last_sent_snapshot, snapshot, m_defn->m_snapshot_size);
}

bool NetObject::is_synced(uint8_t conn_index)
{
    net_connection_state_t* conn_state = get_or_create_conn_state(conn_index);
    return (memcmp(conn_state->m_last_sent_snapshot, m_current_snapshot, m_defn->m_snapshot_size) == 0);
}

net_connection_state_t* NetObject::get_or_create_conn_state(uint8_t conn_index)
{
    std::map<uint8_t, net_connection_state_t*>::iterator it = m_conn_states.find(conn_index); 
    if(it == m_conn_states.end()){
        net_connection_state_t* new_conn_state = new net_connection_state_t();
        new_conn_state->m_last_sent_snapshot = malloc(m_defn->m_snapshot_size);
        memset(new_conn_state->m_last_sent_snapshot, 0, m_defn->m_snapshot_size);
        m_conn_states[conn_index] = new_conn_state;
        return new_conn_state;
    }else{
        return m_conn_states[conn_index];
    }
}