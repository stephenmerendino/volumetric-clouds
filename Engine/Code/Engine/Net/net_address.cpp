#include <Engine/Net/net_address.hpp>
#include "Engine/Net/net.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

bool net_address_t::operator==(const net_address_t& comparand)
{
	return ((address == comparand.address) && (port == comparand.port));
}

bool net_address_from_socket_address(net_address_t* out_net_address, sockaddr* socket_address)
{
    if(nullptr == socket_address){
        return false;
    }

    if(AF_INET != socket_address->sa_family){
        return false;
    }

    sockaddr_in* ipv4_addr = (sockaddr_in*)socket_address;

    out_net_address->port = ntohs(ipv4_addr->sin_port);
    out_net_address->address = ntohl(ipv4_addr->sin_addr.S_un.S_addr);

    return true;
}

void socket_address_from_net_address(sockaddr* out_socket_address, int* out_address_size, const net_address_t& net_address)
{
    sockaddr_in* out_address = (sockaddr_in*)out_socket_address;
    memset(out_address, 0, sizeof(sockaddr_in));

    out_address->sin_family = AF_INET;
    out_address->sin_port = htons(net_address.port);
    out_address->sin_addr.S_un.S_addr = htonl(net_address.address);

    *out_address_size = sizeof(sockaddr_in);
}

bool get_net_address_from_string(net_address_t* out_addr, const std::string& address_string, bool bindable)
{
    std::vector<std::string> tokens = tokenize_string_by_delimeter(address_string.c_str(), ':');
    if(tokens.size() != 2){
        return false;
    }

    // validate port
    uint16_t port = (uint16_t)atoi(tokens[1].c_str());
    if(port == 0){
        return false;
    }

    std::vector<net_address_t> addresses = get_addresses_from_hostname(tokens[0].c_str(), port, bindable);
    if(addresses.empty()){
        return false;
    }

    *out_addr = addresses[0];
    return true;
}

std::vector<net_address_t> get_addresses_from_hostname(const char* hostname, uint16_t port, bool bindable, bool is_udp)
{
    char service[16];
    sprintf_s(service, "%u", port);

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = (is_udp ? SOCK_DGRAM : SOCK_STREAM);
    hints.ai_flags |= (bindable ? AI_PASSIVE : 0);

    // Only use numeric host if hostname is integer based (not google.com)
    // hints.ai_flags |= AI_NUMERICHOST;

    addrinfo* results = nullptr;
    int status = ::getaddrinfo(hostname, service, &hints, &results);

    std::vector<net_address_t> addresses;
    if(0 != status){
        return addresses;
    }

    addrinfo* addr = results;
    while(nullptr != addr){
        net_address_t net_addr;
        if(net_address_from_socket_address(&net_addr, addr->ai_addr)){
            addresses.push_back(net_addr);
        }

        addr = addr->ai_next;
    }

    ::freeaddrinfo(results);
    return addresses;
}

std::string net_address_to_string(const net_address_t& addr)
{
    unsigned int d = (unsigned int)(addr.address & 0x000000FF);
    unsigned int c = (unsigned int)((addr.address & 0x0000FF00) >> 8);
    unsigned int b = (unsigned int)((addr.address & 0x00FF0000) >> 16);
    unsigned int a = (unsigned int)((addr.address & 0xFF000000) >> 24);

    char ip_string[32];
    sprintf_s(ip_string, "%u.%u.%u.%u:%u", a, b, c, d, addr.port);

    return std::string(ip_string);
}

net_address_t get_my_address(uint16_t port)
{
    std::vector<net_address_t> my_addresses = get_addresses_from_hostname("", port, true);

    ASSERT_OR_DIE(my_addresses.size() > 0, "Error:Could not find my own address");

    return my_addresses[0];
}