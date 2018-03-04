#pragma once

#include "Engine/Net/TCP/tcp_session.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Core/Console.hpp"

enum RCSNetMessages : uint8_t
{
    RCS_SEND_COMMAND = NUM_CORE_NET_MESSAGES,
    RCS_SEND_ECHO
};

class RemoteCommandService
{
    public:
        TCPSession m_session;
        NetConnection* m_current_sender;
        bool m_is_visible;
        bool m_echo_enabled;

    public:
        void setup_message_definitions();

        void update(float ds);
        void render();

        void join(const net_address_t& addr);

        void on_command(NetMessage* message);
        void on_echo(NetMessage* message);

        void send_command_to_others(const std::string& command_and_args);
        void send_command_to_index(unsigned int index, const std::string& command_and_args);
        void send_command_to_all(const std::string& command_and_args);
        void send_echo(const std::string& message);

        void set_visibility(bool is_visible);

    public:
        static bool init();
        static void shutdown();
        static RemoteCommandService* get_instance();


    private:
        static RemoteCommandService* s_instance;

    private:
        RemoteCommandService();
        ~RemoteCommandService();
};