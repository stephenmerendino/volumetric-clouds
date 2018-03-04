#pragma once

#include <inttypes.h>
#include <map>

typedef uint16_t net_object_id_t;

class NetMessage;
class NetObjectTypeDefinition;

struct net_connection_state_t
{
    void* m_last_sent_snapshot;
};

// Net Object System
class NetObject
{
    public:
        uint8_t m_type_id;
        net_object_id_t m_net_id;
        NetObjectTypeDefinition* m_defn;

        void* m_local_object;
        void* m_current_snapshot;

        void* m_last_received_snapshot;
        double m_last_received_snapshot_client_timestamp;
		double m_last_updated_timestamp;

        bool m_is_local_dirty;
        bool m_snapshot_is_valid;

        std::map<uint8_t, net_connection_state_t*> m_conn_states;

    public:
        NetObject(NetObjectTypeDefinition *defn);
        ~NetObject();

        void refresh_current_snapshot();
        void append_snapshot(NetMessage* msg);
        void process_snapshot(NetMessage* msg);
        void apply_latest_snapshot();

        void save_last_sent_snapshot(uint8_t conn_index, void* snapshot);
        bool is_synced(uint8_t conn_index);
        net_connection_state_t* get_or_create_conn_state(uint8_t conn_index);
};