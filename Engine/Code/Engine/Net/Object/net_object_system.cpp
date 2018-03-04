#include "Engine/Net/Object/net_object_system.hpp"
#include "Engine/Net/Object/net_object_type_definition.hpp"
#include "Engine/Net/session.hpp"
#include "Engine/Net/connection.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/interval.h"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/log.h"

#include <map>
#include <vector>

#define DEFAULT_UPDATE_HZ 20

static double g_host_clocktime = 0.0f;
static double g_client_clocktime = 0.0f;
static bool g_client_ready = false;

static net_object_id_t g_next_id;
static NetSession* g_session;
static Interval g_update_interval;
static std::map<uint8_t, NetObjectTypeDefinition*> g_registered_defns;
static std::map<net_object_id_t, NetObject*> g_registered_objects;

static void on_receive_net_object_create(NetMessage* msg)
{
    uint8_t type_id;
    msg->read(type_id);

    uint16_t net_id;
    msg->read(net_id);

    NetObjectTypeDefinition *defn = net_object_find_definition(type_id);
    ASSERT_OR_DIE(defn != nullptr, "No net object definition found");

    NetObject* nop = new NetObject(defn);
    nop->m_net_id = net_id;
    nop->m_type_id = type_id;

    void* local_object = defn->process_create_info(msg, nop);
	if(nullptr == local_object){
		return;
	}
    ASSERT_OR_DIE(local_object != nullptr, "Failed to construct local object from net object");

    nop->m_local_object = local_object;
    net_object_register(nop); // register object with system
}

static void on_receive_net_object_destroy(NetMessage* msg)
{
    uint16_t net_id;
    msg->read(net_id);

    NetObject *nop = net_object_find(net_id);

    if(nop == nullptr) {
        return;
    }

    net_object_unregister(nop);

    // THIS is critical;
    nop->m_defn->process_destroy_info(msg, nop->m_local_object);

    delete nop;
}

static void net_object_send_updates_to(NetConnection* conn)
{
    if(nullptr == conn){
        return;
    }

    for(unsigned int i = 0; i < g_registered_objects.size(); i++){
        NetObject* nop = g_registered_objects[(uint16_t)i];
        if(nullptr == nop){
            continue;
        }

        if(nop->is_synced(conn->m_connection_index)){
            continue;
        }

        NetMessage* msg = new NetMessage(NETOBJECT_UPDATE);

        msg->write(nop->m_net_id);
		double time = get_current_time_seconds();
        msg->write(time);
        nop->append_snapshot(msg);

        conn->send(msg);

        nop->save_last_sent_snapshot(conn->m_connection_index, nop->m_current_snapshot);
    }
}

static void net_object_send_updates()
{
    for(unsigned int i = 0; i < g_registered_objects.size(); ++i){
        NetObject* nop = g_registered_objects[(uint16_t)i];
        if(nullptr != nop){
            nop->refresh_current_snapshot();
        }
    }

    // start at 1, 0 is host
    for(unsigned int i = 1; i < g_session->m_connections.size(); i++){
        NetConnection* conn = g_session->m_connections[i];
        if(nullptr != conn){
            net_object_send_updates_to(conn);
        }
    }
}

static void on_receive_net_object_update(NetMessage* msg)
{
    uint16_t net_id;
    msg->read(net_id);

    double host_time;
    msg->read(host_time);

	host_time = msg->m_sent_time; // trying to fix times and stuff yo
	if(host_time <= 0.0001f){
		log_printf("Received an invalid host time %f", host_time);
	}

    NetObject* nop = net_object_find(net_id);
    if(nullptr == nop){
        return;
    }

	double now = host_time;
	if(now > nop->m_last_updated_timestamp){
	    double client_timestamp = (host_time - g_host_clocktime) + g_client_clocktime;
	    nop->m_last_received_snapshot_client_timestamp = client_timestamp;
	    nop->process_snapshot(msg);
		nop->m_last_updated_timestamp = now;
	}
}

static void net_object_system_connection_joined(void* user_arg, NetConnection* new_conn)
{
    NetMessage* msg = new NetMessage(NETOBJECT_SET_CLOCK);
    msg->write(get_current_time_seconds());
    new_conn->send(msg);
}

static void on_receive_net_object_set_clock(NetMessage* msg)
{
	if(!g_client_ready){
	    //msg->read(g_host_clocktime);
		g_host_clocktime = msg->m_sent_time;
	    g_client_clocktime = get_current_time_seconds();
	    g_client_ready = true;
	}
}

void net_object_system_init()
{
    g_next_id = 0;
    g_session = nullptr;
    g_update_interval.set_frequency(DEFAULT_UPDATE_HZ);
}

void net_object_system_shutdown()
{
    std::map<uint8_t, NetObjectTypeDefinition*>::iterator defn_it = g_registered_defns.begin();
    while(defn_it != g_registered_defns.end()){
        SAFE_DELETE(defn_it->second);
        defn_it++;
    }

    std::map<net_object_id_t, NetObject*>::iterator obj_it = g_registered_objects.begin();
    while(obj_it != g_registered_objects.end()){
        SAFE_DELETE(obj_it->second);
        obj_it++;
    }

    g_registered_defns.clear();
    g_registered_objects.clear();
}

void net_object_system_tick()
{
    // no session means there is nothing to update
    if(nullptr == net_object_get_session()){
        return;
    }

    if(g_update_interval.check_and_reset()){
        if(g_session->is_host()){
            net_object_send_updates();
        }
    }

    if(g_session->is_client() && g_client_ready){
        for(unsigned int i = 0; i < g_registered_objects.size(); ++i){
            NetObject* nop = g_registered_objects[(uint16_t)i];
            if(nullptr != nop /* && nop->m_is_local_dirty */){
                nop->apply_latest_snapshot();
            }
        } 
    }
}

void net_object_system_set_update_hz(float hz)
{
    g_update_interval.set_frequency(hz);
}

void net_object_system_register_session(NetSession* session)
{
    g_session = session;
    
    g_session->register_message(NETOBJECT_CREATE, on_receive_net_object_create, false, true, true);
    g_session->register_message(NETOBJECT_DESTROY, on_receive_net_object_destroy, false, true, true);
    g_session->register_message(NETOBJECT_UPDATE, on_receive_net_object_update, false, false, false);
    g_session->register_message(NETOBJECT_SET_CLOCK, on_receive_net_object_set_clock, false, true, false);

    g_session->m_connection_joined_event->subscribe(nullptr, net_object_system_connection_joined);
}

NetSession* net_object_get_session()
{
    return g_session;
}

void net_object_system_register_type(uint8_t object_type, NetObjectTypeDefinition* defn)
{
    g_registered_defns[object_type] = defn;
}

NetObjectTypeDefinition* net_object_find_definition(uint8_t object_type)
{
    std::map<uint8_t, NetObjectTypeDefinition*>::iterator found = g_registered_defns.find(object_type);
    if(g_registered_defns.end() == found){
        return nullptr;
    }

    return found->second;
}

void net_object_register(NetObject* nop)
{
    g_registered_objects[nop->m_net_id] = nop;
}

void net_object_unregister(NetObject* nop)
{
    std::map<uint16_t, NetObject*>::iterator it = g_registered_objects.find(nop->m_net_id);
    g_registered_objects.erase(it);
}

NetObject* net_object_find(net_object_id_t net_id)
{
    std::map<net_object_id_t, NetObject*>::iterator found = g_registered_objects.find(net_id);
    if(g_registered_objects.end() == found){
        return nullptr;
    }

    return found->second;
}

net_object_id_t net_object_get_unused_id()
{
    while(nullptr != net_object_find(g_next_id)){
        g_next_id++;
    }

    return g_next_id;
}

NetObject* net_object_replicate(void* object_ptr, uint8_t type_id)
{
    NetObjectTypeDefinition* defn = net_object_find_definition(type_id);
    if(nullptr == defn) {
        return nullptr;
    }

    NetObject* nop = new NetObject(defn);

    nop->m_local_object = object_ptr;
    nop->m_net_id = net_object_get_unused_id();
    nop->m_type_id = type_id;

    net_object_register(nop);

    NetMessage create(NETOBJECT_CREATE);
    create.write(nop->m_type_id);
    create.write(nop->m_net_id);

    defn->append_create_info(&create, object_ptr);

    NetSession *sp = net_object_get_session();
	if(nullptr != sp){
	    sp->send_message_to_others(create);
	}

    return nop;
}

void net_object_stop_replication(net_object_id_t net_id)
{
    // remove from our system
    NetObject *nop = net_object_find(net_id);
    if(nop == nullptr) {
        return;
    }

    net_object_unregister(nop);

    // tell everyone 
    NetMessage msg(NETOBJECT_DESTROY);
    msg.write(nop->m_net_id);

    // usually does nothing - no-op.
    nop->m_defn->append_destroy_info(&msg, nop->m_local_object);

    NetSession *sp = net_object_get_session();
	if(nullptr != sp){
	    sp->send_message_to_others(msg);
	}
}

void net_object_system_init_connection(uint8_t new_connection)
{
    for(unsigned int i = 0; i < g_registered_objects.size(); i++){
        NetObject* nop = g_registered_objects[(uint16_t)i];
        if(nullptr == nop){
            continue;
        }

        NetMessage create(NETOBJECT_CREATE);
        create.write(nop->m_type_id);
        create.write(nop->m_net_id);

        nop->m_defn->append_create_info(&create, nop->m_local_object);

        NetSession *sp = net_object_get_session();
		if(nullptr != sp){
	        sp->send_message_to_index(new_connection, create);
		}
    }
}

double net_object_system_get_tick_freq()
{
    return (1.0f / g_update_interval.m_interval_time);
}

unsigned int net_object_system_get_num_objects()
{
    unsigned int count = 0;
    std::map<net_object_id_t, NetObject*>::iterator it;
    for(it = g_registered_objects.begin(); it != g_registered_objects.end(); it++){
        if(nullptr != it->second){
            count++;
        }
    }
    return count;
}

COMMAND(net_set_hz, "[int:hz] Sets the update hz of the network tick")
{
    int hz = -1;
    hz = args.next_int_arg(); 
    if(-1 != hz){
        net_object_system_set_update_hz((float)hz);
    }
}