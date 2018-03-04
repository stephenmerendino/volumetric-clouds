#pragma once

#include "Engine/Net/message.hpp"
#include <functional>

#define NETMSG_RELIABLE         0x01
#define NETMSG_IN_ORDER         0x02
#define NETMSG_CONNECTIONLESS   0x04

typedef void(*net_function_t)(NetMessage*);

class NetMessageDefinition
{
    typedef void(*internal_handler)(NetMessageDefinition*, NetMessage*);

    public:
        uint8_t m_message_id;
        internal_handler m_internal_handler;
        void* m_provided_handler;
        void* m_caller;
        byte_t m_flags;

    public:
        NetMessageDefinition();
        void handle(NetMessage* msg);

        bool is_reliable() const;
        bool is_in_order() const;
        bool is_connectionless() const;

        void set_is_reliable(bool new_setting);
        void set_is_in_order(bool new_setting);
        void set_is_connectionless(bool new_setting);

    public:
        static void function_handler(NetMessageDefinition* def, NetMessage* msg);

        template <typename T, typename MCB>
        static void method_handler(NetMessageDefinition* def, NetMessage* msg);
};

template <typename T, typename MCB>
void NetMessageDefinition::method_handler(NetMessageDefinition* def, NetMessage* msg)
{
    MCB mcb = *(MCB*)&(def->m_provided_handler);
    T* obj = (T*)(def->m_caller);
    (obj->*mcb)(msg);
}