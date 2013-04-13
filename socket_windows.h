#ifndef __SOCKET_WINDOWS_H
#ifdef _WIN32

#include <WinSock2.h>

// Headers.

bool socket_startup();
bool socket_shutdown();

// Constants.

#define socklen_t int

// Macros.

#define SOCKET_STARTUP() socket_startup()
#define SOCKET_SHUTDOWN() socket_shutdown()

#define SOCKET(socket_type, protocol_type, address_family) socket(socket_type, protocol_type, 0)
#define SOCKET_READ(socket, buffer, buf_size) recv(socket, buffer, buf_size, 0)
#define SOCKET_WRITE(socket, chunk, send_length) send(socket, chunk, send_length, 0)
#define SOCKET_BIND(socket, name, namelen) bind(socket, name, namelen)
#define SOCKET_LISTEN(socket, backlog) listen(socket, backlog)
#define SOCKET_ACCEPT(socket, addr, addrlen) accept(socket, addr, addrlen)
#define SOCKET_CLOSE(socket) closesocket(socket);
#define SOCKET_NAME(socket, addr, addrlen) getsockname(socket, addr, addrlen)

#endif
#endif
