#include "Engine/Net/Object/net_object_type_definition.hpp"
#include "Engine/Net/Object/net_object.hpp"
#include "Engine/Net/message.hpp"

void noop_append_create_info(NetMessage* m, void* o)
{
    UNUSED(m);
    UNUSED(o);
}

void* noop_process_create_info(NetMessage* m, NetObject* no)
{
    UNUSED(m);
    UNUSED(no);
    return nullptr;
}

void noop_append_destroy_info(NetMessage* m, void* o)
{
    UNUSED(m);
    UNUSED(o);
}

void noop_process_destroy_info(NetMessage* m, void* o)
{
    UNUSED(m);
    UNUSED(o);
}

void* noop_create_snapshot()
{
    return nullptr;
}

void noop_refresh_current_snapshot(void* s, void* l)
{
    UNUSED(s);
    UNUSED(l);
}

void noop_append_snapshot(NetMessage* m, void* s)
{
    UNUSED(m);
    UNUSED(s);
}

void noop_process_snapshot(NetMessage* m, void* s)
{
    UNUSED(m);
    UNUSED(s);
}

void noop_apply_snapshot(void* s, void* l, double ds)
{
    UNUSED(s);
    UNUSED(l);
    UNUSED(ds);
}

NetObjectTypeDefinition::NetObjectTypeDefinition()
    :m_snapshot_size(0)
{
    append_create_info = noop_append_create_info;
    process_create_info = noop_process_create_info;

    append_destroy_info = noop_append_destroy_info;
    process_destroy_info = noop_process_destroy_info;

    create_snapshot = noop_create_snapshot;
    refresh_current_snapshot = noop_refresh_current_snapshot;
    append_snapshot = noop_append_snapshot;
    process_snapshot = noop_process_snapshot;
    apply_snapshot = noop_apply_snapshot;
}