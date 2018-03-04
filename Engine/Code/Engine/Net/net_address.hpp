#pragma once

#include <inttypes.h>
#include <vector>

#define NET_DEFAULT_PORT 12345

struct sockaddr;

struct net_address_t
{
    unsigned int    address = 0;
    uint16_t        port = 0;

	bool operator==(const net_address_t& comparand);
};

bool net_address_from_socket_address(net_address_t* out_net_address, sockaddr* socket_address);
void socket_address_from_net_address(sockaddr* out_socket_address, int* out_address_size, const net_address_t& net_address);

bool                        get_net_address_from_string(net_address_t* out_addr, const std::string& address_string, bool bindable);
std::vector<net_address_t>  get_addresses_from_hostname(const char* hostname, uint16_t port, bool bindable, bool is_udp = false);
std::string                 net_address_to_string(const net_address_t& addr);
net_address_t               get_my_address(uint16_t port);