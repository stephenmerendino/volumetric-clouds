#include "Engine/Net/remote_command_service.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Net/connection.hpp"
#include "Engine/Net/TCP/tcp_connection.hpp"
#include "Engine/Config/EngineConfig.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Engine.hpp"

RemoteCommandService* RemoteCommandService::s_instance = nullptr;

static void console_print_cb(void* user_arg, const std::string& message)
{
    DebuggerPrintf("%s\n", message.c_str());
    RemoteCommandService::get_instance()->send_echo(message);
}

void RemoteCommandService::setup_message_definitions()
{
    m_session.register_message(RCS_SEND_COMMAND, this, &RemoteCommandService::on_command, false, true, true);
    m_session.register_message(RCS_SEND_ECHO, this, &RemoteCommandService::on_echo, false, true, true);
}

void RemoteCommandService::update(float ds)
{
    if(m_session.is_running()){
        m_session.update(ds);
    }else{
        net_address_t join_addr = get_my_address(RCS_PORT);

        char* config_join_address;
        if(ConfigGetString(&config_join_address, "rcsJoinAddress")){
            get_net_address_from_string(&join_addr, config_join_address, false);
        }

        if(!m_session.join(join_addr)){
            m_session.host(RCS_PORT);
            m_session.start_listening();
        }
    }
}

void RemoteCommandService::render()
{
    if(!m_is_visible){
        return;
    }

    // do stuff
    g_theRenderer->SetModel(Matrix4::IDENTITY);
    g_theRenderer->SetView(Matrix4::IDENTITY);
    g_theRenderer->SetAspectNormalizedOrtho();

    Vector2 cursor(1.3f, 0.99f);

    // render the title
    g_theRenderer->DrawText2d(cursor, 0.0225f, Rgba::WHITE, "Remote Command Service", get_engine_font());

    // render state of the session
    float width = get_engine_font().GetTextWidth("Remote Command Service ", 0.0225f);
    Vector2 state_cursor = cursor + Vector2(width, 0.0f);

    std::string connection_state_string;
    switch(m_session.m_state){
        case SESSION_DISCONNECTED:  connection_state_string = "[DISCONNECTED]"; break;
        case SESSION_JOINING:       connection_state_string = "[JOINING]"; break;
        case SESSION_READY:         connection_state_string = "[READY]"; break;
    }

    switch(m_session.m_state){
        case SESSION_DISCONNECTED:  g_theRenderer->DrawText2d(state_cursor, 0.0225f, Rgba::RED, connection_state_string.c_str(), get_engine_font()); break;
        case SESSION_JOINING:       g_theRenderer->DrawText2d(state_cursor, 0.0225f, Rgba::YELLOW, connection_state_string.c_str(), get_engine_font()); break;
        case SESSION_READY:         g_theRenderer->DrawText2d(state_cursor, 0.0225f, Rgba::GREEN, connection_state_string.c_str(), get_engine_font()); break;
    }

    cursor.y -= get_engine_font().GetLineHeight(0.0225f);

    // render my connection info
    if(nullptr != m_session.m_host_connection){
        std::string host_address_string = net_address_to_string(m_session.m_host_connection->m_address);
        g_theRenderer->DrawText2d(cursor, 0.02f, Rgba::GREY, Stringf("[%s] join address: %s", (m_session.is_client() ? "client" : "host"), host_address_string.c_str()).c_str(), get_engine_font());
    }else{
        g_theRenderer->DrawText2d(cursor, 0.02f, Rgba::GREY, "Configuring session...", get_engine_font());
    }

    // render connections
    cursor.y -= get_engine_font().GetLineHeight(0.02f);
    unsigned int number_live_connections = m_session.get_number_of_live_clients();
    g_theRenderer->DrawText2d(cursor, 0.02f, Rgba::WHITE, Stringf("client(s): %i/%i", number_live_connections, m_session.m_max_num_connections).c_str(), get_engine_font());

    // render each connection
    cursor.y -= get_engine_font().GetLineHeight(0.02f);
    cursor.x += 0.025f;

    for(unsigned int conn_idx = 0; conn_idx < m_session.m_connections.size(); ++conn_idx){
        NetConnection* conn = m_session.m_connections[conn_idx];
        if(nullptr == conn){
            continue;
        }

        unsigned int conn_luid = (unsigned int)conn->m_connection_index;
        std::string conn_address_string = net_address_to_string(conn->m_address);

        if(conn == m_session.m_host_connection){
            g_theRenderer->DrawText2d(cursor, 0.02f, Rgba::GREEN, Stringf("* [%i] %s %s", conn_luid, conn_address_string.c_str(), connection_state_string.c_str()).c_str(), get_engine_font());
        }else if(conn == m_session.m_my_connection){
            g_theRenderer->DrawText2d(cursor, 0.02f, Rgba::GREY, Stringf("- [%i] ANY %s", conn_luid, connection_state_string.c_str()).c_str(), get_engine_font());
        }else{
            g_theRenderer->DrawText2d(cursor, 0.02f, Rgba::WHITE, Stringf("- [%i] %s %s", conn_luid, conn_address_string.c_str(), connection_state_string.c_str()).c_str(), get_engine_font());
        }

        cursor.y -= get_engine_font().GetLineHeight(0.02f);
    }
}

void RemoteCommandService::join(const net_address_t& addr)
{
    m_session.leave();
    m_session.join(addr);
}

void RemoteCommandService::on_command(NetMessage* message)
{
    char command_and_args[512];
    message->read_string(command_and_args, 512);
    
    net_address_t sender_addr = message->m_sender->m_address;
    console_printf(Rgba::YELLOW, "[%s]: %s", net_address_to_string(sender_addr).c_str(), command_and_args);

    m_current_sender = message->m_sender;

    // bind to console print event
    console_register_to_print_event(nullptr, console_print_cb);
    // run command
    console_run_command_and_args(command_and_args);
    // unbind to console print event
    console_unregister_to_print_event(nullptr, console_print_cb);

    m_current_sender = nullptr;
}

void RemoteCommandService::on_echo(NetMessage* message)
{
    if(m_echo_enabled){
        char response[512];
        message->read_string(response, 512);
        console_printf(Rgba::YELLOW, "[%s] %s", net_address_to_string(message->m_sender->m_address).c_str(), response);
    }
}

void RemoteCommandService::send_command_to_others(const std::string& command_and_args)
{
    NetMessage* message = new NetMessage(RCS_SEND_COMMAND);
    message->write_string(command_and_args.c_str());
    m_session.send_message_to_others(*message);
    delete message;
}

void RemoteCommandService::send_command_to_index(unsigned int index, const std::string& command_and_args)
{
    NetMessage* message = new NetMessage(RCS_SEND_COMMAND);
    message->write_string(command_and_args.c_str());
    m_session.send_message_to_index(index, *message);
    delete message;
}

void RemoteCommandService::send_command_to_all(const std::string& command_and_args)
{
    NetMessage* message = new NetMessage(RCS_SEND_COMMAND);
    message->write_string(command_and_args.c_str());
    m_session.send_message_to_all(*message);
    delete message;
}

void RemoteCommandService::send_echo(const std::string& message)
{
    NetMessage* echo_message = new NetMessage(RCS_SEND_ECHO);
    echo_message->write_string(message.c_str());
    m_current_sender->send(echo_message);
}

void RemoteCommandService::set_visibility(bool is_visible)
{
    m_is_visible = is_visible;
}

bool RemoteCommandService::init()
{
    s_instance = new RemoteCommandService();
    return true;
}

void RemoteCommandService::shutdown()
{
    delete s_instance;
}

RemoteCommandService* RemoteCommandService::get_instance()
{
    return s_instance;
}

RemoteCommandService::RemoteCommandService()
    :m_current_sender(nullptr)
    ,m_is_visible(false)
    ,m_echo_enabled(true)
{
    setup_message_definitions();
}

RemoteCommandService::~RemoteCommandService()
{
}

COMMAND(rc, "Runs a remote command")
{
    RemoteCommandService* rcs = RemoteCommandService::get_instance();

    std::string command_and_args = args.m_raw_args;
    if(command_and_args.empty()){
        console_warning("No command included.");
    }

    rcs->send_command_to_others(command_and_args);
}

COMMAND(rc_join, "[ip:port] Join a remote command service")
{
    std::string address_string = args.next_string_arg();
    net_address_t addr;
    if(get_net_address_from_string(&addr, address_string, false)){
        RemoteCommandService::get_instance()->join(addr);
    }
}

COMMAND(rci, "[idx:int] Send a remote command to a specific connection id (conn index)")
{
    RemoteCommandService* rcs = RemoteCommandService::get_instance();

    int conn_index = args.next_int_arg();
    std::string command_and_args = args.get_remaining_args_as_string();
    if(command_and_args.empty()){
        console_warning("No command included.");
    }

    rcs->send_command_to_index(conn_index, command_and_args);
}

COMMAND(rca, "Send a remote command everyone including yourself")
{
    RemoteCommandService* rcs = RemoteCommandService::get_instance();

    std::string command_and_args = args.get_remaining_args_as_string();
    if(command_and_args.empty()){
        console_warning("No command included.");
    }

    rcs->send_command_to_all(command_and_args);
}

COMMAND(rc_enable_echo, "Enables remote echo")
{
    RemoteCommandService::get_instance()->m_echo_enabled = true;
}

COMMAND(rc_disable_echo, "Enables remote echo")
{
    RemoteCommandService::get_instance()->m_echo_enabled = false;
}