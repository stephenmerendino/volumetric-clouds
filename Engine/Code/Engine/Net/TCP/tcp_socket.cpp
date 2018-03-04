#include "Engine/Net/TCP/tcp_socket.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/log.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

TCPSocket::TCPSocket()
    :m_socket(INVALID_SOCKET)
    ,m_is_listen_socket(false)
{
}

TCPSocket::~TCPSocket()
{
    close();
}

bool TCPSocket::join(net_address_t address)
{
    if(is_valid()){
        return false;
    } 

    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sock){
        return false;
    }

    sockaddr_storage internal_address;
    int addr_size = 0;
    socket_address_from_net_address((sockaddr*)&internal_address, &addr_size, address);

    int result = ::connect(sock, (sockaddr*)&internal_address, sizeof(sockaddr_in));
    if(0 != result){
        ::closesocket(sock);
        return false;
    }

    m_socket = sock;
    m_address = address;
    return true;
}

bool TCPSocket::join(const char* address)
{
    std::vector<std::string> tokens = tokenize_string_by_delimeter(address, ':');

    // invalid expression passed in
    if(2 != tokens.size()){
        return false;
    }

    uint16_t port = (uint16_t)atoi(tokens[1].c_str());

    std::vector<net_address_t> addresses = get_addresses_from_hostname(tokens[0].c_str(), port, false);

    if(0 == addresses.size()){
        return false;
    }

    return join(addresses[0]);
}

void TCPSocket::close()
{
    if(is_valid()){
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        m_is_listen_socket = false;
    }
}

bool TCPSocket::listen(uint16_t port)
{
    if(is_valid()){
        return false;
    }

    std::vector<net_address_t> addresses = get_addresses_from_hostname("", port, true); 
    if(0 == addresses.size()){
        return false;
    }

    return listen(addresses[0]);
}

bool TCPSocket::listen(net_address_t net_addr)
{
    SOCKET listen_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == listen_socket){
        return false;
    }

    sockaddr_storage listen_address;
    int addr_size = 0;
    socket_address_from_net_address((sockaddr*)&listen_address, &addr_size, net_addr);

    // associate address to this socket
    int result = ::bind(listen_socket, (sockaddr*)&listen_address, addr_size);
    if(SOCKET_ERROR == result){
        ::closesocket(listen_socket);
        return false;
    }

    int max_queued = 8;
    result = ::listen(listen_socket, (int)max_queued);
    if(SOCKET_ERROR == result){
        ::closesocket(listen_socket);
        return false;
    }

    m_socket = listen_socket;
    m_address = net_addr;
    m_is_listen_socket = true;
    return true;
}

bool TCPSocket::is_listening() const
{
    return m_is_listen_socket;
}

TCPSocket* TCPSocket::accept()
{
    if(!is_listening()){
        return nullptr;
    }

    sockaddr_storage conn_addr;
    int conn_addr_size = sizeof(conn_addr);

    SOCKET incoming_socket = ::accept(m_socket, (sockaddr*)&conn_addr, &conn_addr_size);
    if(INVALID_SOCKET == incoming_socket){
        return nullptr;
    }

    net_address_t net_addr;
    if(!net_address_from_socket_address(&net_addr, (sockaddr*)&conn_addr)){
        ::closesocket(incoming_socket);
        return nullptr;
    }

    TCPSocket* incoming_tcp_socket = new TCPSocket();
    incoming_tcp_socket->m_socket = incoming_socket;
    incoming_tcp_socket->m_address = net_addr;
    return incoming_tcp_socket;
}

bool TCPSocket::is_valid() const
{
    return (INVALID_SOCKET != m_socket);
}

unsigned int TCPSocket::send(const void* payload, unsigned int payload_size_bytes)
{
    if(!is_valid()){
        return 0;
    }

    if(is_listening()){
        // todo: ASSERT_RETURN_VALUE, recoverable error
        return 0;
    }

    if(0 == payload_size_bytes){
        return 0;
    }

    int bytes_sent = ::send(m_socket, (const char*)payload, (int)payload_size_bytes, 0);
    if(bytes_sent <= 0){
        int error = ::WSAGetLastError();
        log_printf("TCPSocket send error: %i", error);        
        close(); // something went wrong
        return 0;
    }

    ASSERT_OR_DIE((unsigned int)bytes_sent == payload_size_bytes, "TCPSocket send error: Numbers of bytes sent does not equal payload size");

    return bytes_sent;
}

unsigned int TCPSocket::receive(void* payload, unsigned int max_payload_size_bytes)
{
    if(!is_valid() || (0 == max_payload_size_bytes)){
        return 0; 
    }

    if(is_listening()){
        // todo: ASSERT_RETURN_VALUE, recoverable error
        return 0;
    }

    ASSERT_OR_DIE(nullptr != payload, "TCPSocket receive error: Null payload passed");

    int bytes_read = ::recv(m_socket, (char*)payload, (int)max_payload_size_bytes, 0);
    if(bytes_read <= 0){
        if(bytes_read == SOCKET_ERROR){
            int error = ::WSAGetLastError();
            if(WSAEWOULDBLOCK != error){
                close();
            }
        }else{
            check_for_disconnect();
        }
        return 0;
    }

    return (unsigned int)bytes_read;
}

void TCPSocket::check_for_disconnect()
{
    if(!is_valid()){
        return;
    }

    WSAPOLLFD fd;
    fd.fd = m_socket;
    fd.events = POLLRDNORM;

    if(SOCKET_ERROR == ::WSAPoll(&fd, 1, 0)){
        return;
    }

    if(0 != (fd.revents & POLLHUP)){
        close();
    }
}

void TCPSocket::set_blocking(bool is_blocking)
{
    if(!is_valid()){
        return;
    }

    u_long not_blocking = is_blocking ? 0 : 1;
    ::ioctlsocket(m_socket, FIONBIO, &not_blocking);
}