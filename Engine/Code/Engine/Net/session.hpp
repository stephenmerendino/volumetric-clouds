#pragma once

#include "Engine/Net/message_definition.hpp"
#include "Engine/Core/event.h"

#include <vector>
#include <functional>

#define INVALID_CONNECTION_INDEX 0xFF
#define DEFAULT_PORT_ATTEMPT_RANGE 32

class NetConnection;
class NetMessage;
struct net_address_t;

enum CoreNetMessages : uint8_t
{
    NETMSG_PING,
    NETMSG_PONG,
    NETMSG_JOIN_REQUEST,
    NETMSG_JOIN_RESPONSE,
	NETMSG_HEARTBEAT,
	NETMSG_CONN_UPDATE,
	NETMSG_LEAVE,
    NETOBJECT_CREATE,
    NETOBJECT_DESTROY,
    NETOBJECT_UPDATE,
    NETOBJECT_SET_CLOCK,
    NUM_CORE_NET_MESSAGES
};

enum SessionState : uint8_t
{
    SESSION_INVALID,
    SESSION_DISCONNECTED,
    SESSION_DISCOVER,
    SESSION_CONNECTING,
    SESSION_JOINING,
    SESSION_READY
};

class NetSession
{
    public:
        SessionState                        m_state;
        unsigned int                        m_max_num_connections;

        NetConnection*                      m_connection_list;
        std::vector<NetConnection*>         m_connections;

        NetConnection*                      m_my_connection;
        NetConnection*                      m_host_connection;

        std::vector<NetMessageDefinition*>  m_message_definitions;

        Event<NetConnection*>* m_connection_joined_event;
        Event<NetConnection*>* m_connection_left_event;
        Event<NetConnection*>* m_session_joined_event;
        Event<NetConnection*>* m_net_tick_event;
        Event<>* m_host_left_event;

    public:
        NetSession(unsigned int max_num_connections = 8);
        virtual ~NetSession();

    public:
        virtual bool            host(uint16_t port, uint16_t range) = 0;
        virtual bool            join(const net_address_t& address) = 0;
        virtual void            leave() = 0;
        virtual void            update(float ds) = 0;
		virtual bool			start(uint16_t port, uint16_t range = DEFAULT_PORT_ATTEMPT_RANGE) = 0;

        virtual bool            start_listening() = 0;
        virtual void            stop_listening() = 0;
        virtual bool            is_listening() = 0;

        virtual void            set_state(SessionState new_state);

		virtual void			send_message_direct(const net_address_t& dest_addr, NetMessage* msg) = 0;
		virtual void			send_message_direct(uint conn_idx, NetMessage* msg) = 0;
		virtual void			send_packet_immediately(uint conn_idx, NetMessage* msg) = 0;

    public:
        bool                    is_host() const;
        bool                    is_client() const;
        bool                    is_running() const;
        bool                    is_ready() const;

        bool                    register_message(uint8_t msg_id, net_function_t handler, bool is_connectionless, bool is_reliable, bool is_in_order);

        template <typename T>
        bool                    register_message(uint8_t msg_id, T* caller, void(T::*method_cb)(NetMessage*), bool is_connectionless, bool is_reliable, bool is_in_order);
        void                    unregister_message(uint8_t msg_id);

		NetMessageDefinition*   get_message_definition(uint8_t msg_id) const;
        bool                    is_message_registered(uint8_t msg_id) const;
		bool					is_message_reliable(uint8_t msg_id) const;
		bool					is_message_in_order(uint8_t msg_id) const;
		bool					is_message_connectionless(uint8_t msg_id) const;

		uint8_t					get_number_of_live_connections() const;
		void					set_max_connections(u8 max_num_connections);
		bool					has_free_connection_slots() const;
        uint8_t                 get_free_connection_index() const;
        void                    join_connection(u8 index, NetConnection* new_connection);

        void                    destroy_connection(u8 connection_index);
        void                    destroy_connection(NetConnection* connection);
		void					destroy_all_connections();

        NetConnection*          get_connection_by_index(uint8_t connection_index);
        NetConnection*          get_connection_by_guid(const std::string& guid);
        NetConnection*          get_connection_by_address(const std::string& address);
		NetConnection*			get_connection_by_address(const net_address_t& address);

        void                    send_message_to_others(NetMessage const &msg);
        void                    send_message_to_index(unsigned int index, NetMessage const &msg);
        void                    send_message_to_all(NetMessage const &msg);
        void                    send_message_to_all_clients_but_index(NetMessage const &msg, unsigned int excluded_index);
        void                    send_message_to_host(NetMessage const &msg);

		std::string				get_host_address_string();
};

template <typename T>
bool NetSession::register_message(uint8_t msg_id, T* caller, void(T::*method_cb)(NetMessage*), bool is_connectionless, bool is_reliable, bool is_in_order)
{
    if(is_message_registered(msg_id)){
        return false;
    }

    NetMessageDefinition* new_def = new NetMessageDefinition();
    new_def->m_message_id = msg_id;
    new_def->m_provided_handler = *((void**)&method_cb);
    new_def->m_internal_handler = new_def->method_handler<T, decltype(method_cb)>;
    new_def->m_caller = caller;
    new_def->set_is_connectionless(is_connectionless);
    new_def->set_is_reliable(is_reliable);
    new_def->set_is_in_order(is_in_order);

    m_message_definitions.push_back(new_def);

    return true;
}