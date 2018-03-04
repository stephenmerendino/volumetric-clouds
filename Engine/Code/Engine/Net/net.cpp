#include "Engine/Net/net.hpp"

bool net_system_init()
{
    // pick the WinSock version
    WORD version = MAKEWORD(2, 2);

    WSADATA data;
    int error = ::WSAStartup(version, &data);

    return (0 == error);
}

void net_system_shutdown()
{
    ::WSACleanup();
}