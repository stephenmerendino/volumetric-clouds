#pragma once

class NetMessage;
class NetObject;

typedef void  (*append_create_info_cb)(NetMessage*, void* local_object);
typedef void* (*process_create_info_cb)(NetMessage*, NetObject* nop);
typedef void  (*append_destroy_info_cb)(NetMessage*, void* local_object);
typedef void  (*process_destroy_info_cb)(NetMessage*, void* local_object);
typedef void* (*create_snapshot_cb)();
typedef void  (*refresh_current_snapshot_cb)(void* snapshot, void* local_object);
typedef void  (*append_snapshot_cb)(NetMessage*, void* snapshot);
typedef void  (*process_snapshot_cb)(NetMessage*, void* snapshot);
typedef void  (*apply_snapshot_cb)(void* snapshot, void* local_object, double delta_seconds);

class NetObjectTypeDefinition
{
    public:
        append_create_info_cb append_create_info;
        process_create_info_cb process_create_info;

        append_destroy_info_cb append_destroy_info;
        process_destroy_info_cb process_destroy_info;

        create_snapshot_cb create_snapshot;
        refresh_current_snapshot_cb refresh_current_snapshot;
        append_snapshot_cb append_snapshot;
        process_snapshot_cb process_snapshot;
        apply_snapshot_cb apply_snapshot;

        size_t m_snapshot_size;

    public:
        NetObjectTypeDefinition();
};