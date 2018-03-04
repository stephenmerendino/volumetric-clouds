#pragma once

#include "Engine/Net/session.hpp"
#include "Engine/Net/Object/net_object.hpp"
#include <inttypes.h>

class NetSession;
class NetObject;
class NetObjectTypeDefinition;

void net_object_system_init();
void net_object_system_shutdown();
void net_object_system_tick();

void net_object_system_set_update_hz(float hz);

void net_object_system_register_session(NetSession* session);
NetSession* net_object_get_session();

void net_object_system_register_type(uint8_t object_type, NetObjectTypeDefinition* defn);
NetObjectTypeDefinition* net_object_find_definition(uint8_t object_type);

void net_object_register(NetObject* nop);
void net_object_unregister(NetObject* nop);
NetObject* net_object_find(net_object_id_t net_id);

net_object_id_t net_object_get_unused_id();
NetObject* net_object_replicate(void* object_ptr, uint8_t type_id);
void net_object_stop_replication(net_object_id_t net_id);

void net_object_system_init_connection(uint8_t new_connection);

double net_object_system_get_tick_freq();
unsigned int net_object_system_get_num_objects();