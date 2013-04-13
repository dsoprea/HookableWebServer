#include "socket_windows.h"

bool socket_startup()
{
    WSADATA wsaData;
    int result;

    if((result = WSAStartup(MAKEWORD(2,2), &wsaData)) != NO_ERROR)
        return false;

    return true;
}

bool socket_shutdown()
{
    WSACleanup();

    return true;
}
