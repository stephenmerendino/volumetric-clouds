#include "Engine/Net/message_definition.hpp"
#include "Engine/Core/bit.h"

NetMessageDefinition::NetMessageDefinition()
    :m_flags(0)
{
    SET_BIT(m_flags, NETMSG_RELIABLE | NETMSG_IN_ORDER);
}

void NetMessageDefinition::handle(NetMessage* msg)
{
    m_internal_handler(this, msg);
}

void NetMessageDefinition::function_handler(NetMessageDefinition* def, NetMessage* msg)
{
    net_function_t func_cb = (net_function_t)def->m_provided_handler;
	if(nullptr != func_cb){
	    func_cb(msg);
	}
}

bool NetMessageDefinition::is_reliable() const
{
    return IS_BIT_SET(m_flags, NETMSG_RELIABLE);
}

bool NetMessageDefinition::is_in_order() const
{
    return IS_BIT_SET(m_flags, NETMSG_IN_ORDER);
}

bool NetMessageDefinition::is_connectionless() const
{
    return IS_BIT_SET(m_flags, NETMSG_CONNECTIONLESS);
}

void NetMessageDefinition::set_is_reliable(bool new_setting)
{
    if(is_reliable() == new_setting){
        return;
    } 

    FLIP_BIT(m_flags, NETMSG_RELIABLE);
}

void NetMessageDefinition::set_is_in_order(bool new_setting)
{
    if(is_in_order() == new_setting){
        return;
    } 

    FLIP_BIT(m_flags, NETMSG_IN_ORDER);
}

void NetMessageDefinition::set_is_connectionless(bool new_setting)
{
    if(is_connectionless() == new_setting){
        return;
    } 

    FLIP_BIT(m_flags, NETMSG_CONNECTIONLESS);
}