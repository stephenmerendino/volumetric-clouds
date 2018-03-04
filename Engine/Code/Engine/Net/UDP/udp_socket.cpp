#include "Engine/Net/UDP/udp_socket.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Net/net_packet.hpp"

bool UDPSocket::bind(uint16_t port)
{
    std::vector<net_address_t> bind_addresses = get_addresses_from_hostname("", port, true, true);
    if(bind_addresses.empty()){
        return false;
    }

    for(unsigned int addr_idx = 0; addr_idx < bind_addresses.size(); ++addr_idx){
        net_address_t bind_addr = bind_addresses[addr_idx];
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(INVALID_SOCKET != m_socket){
            sockaddr_storage internal_address;
            int addr_size = 0;
            socket_address_from_net_address((sockaddr*)&internal_address, &addr_size, bind_addr);

            int result = ::bind(sock, (sockaddr*)&internal_address, addr_size);
            if(0 != result){
                ::closesocket(sock);
                return false;
            }

            u_long non_blocking = 1;
            ioctlsocket(sock, FIONBIO, &non_blocking);

            m_socket = sock;
            m_address = bind_addr;
            break;
        }
    }

    return true;
}

bool UDPSocket::is_valid()
{
	return (INVALID_SOCKET != m_socket);
}

unsigned int UDPSocket::send(const net_address_t& addr, const void* payload, unsigned int payload_size_bytes)
{
    if(INVALID_SOCKET == m_socket){
        return 0;
    }

    sockaddr sock_addr;
    int sock_addr_size;
    socket_address_from_net_address(&sock_addr, &sock_addr_size, addr);

    unsigned int bytes_sent = ::sendto(m_socket, 
                                       (const char*)payload, 
                                       (int)payload_size_bytes, 
                                       0, 
                                       (const sockaddr*)&sock_addr, 
                                       sock_addr_size);

    return bytes_sent;
}

unsigned int UDPSocket::receive(net_address_t* out_send_addr, void* payload, unsigned int max_payload_size_bytes)
{
    if(INVALID_SOCKET == m_socket){
        return 0;
    }

    sockaddr_storage addr;
    int addrlen = sizeof(addr);

    int size = ::recvfrom(m_socket,
                          (char*)payload,
                          max_payload_size_bytes,
                          0,
                          (sockaddr*)&addr,
                          &addrlen);

    if(size > 0){
        if(addrlen != sizeof(sockaddr_in)){
            return 0;
        }

        //convert sockaddr to net_address_t for sender
        sockaddr_in from_addr;
        memcpy(&from_addr, &addr, addrlen);
        net_address_from_socket_address(out_send_addr, (sockaddr*)&from_addr);

        return size;
    }

    return 0U;
}