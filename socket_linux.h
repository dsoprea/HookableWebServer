#ifndef __SOCKET_UNIX_H
#ifdef linux

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_STARTUP() {}
#define SOCKET_SHUTDOWN() {}

#define SOCKET(socket_type, protocol_type, address_family) socket(socket_type, protocol_type, 0)
#define SOCKET_READ(fd, buffer, buf_size) read(fd, buffer, buf_size)
#define SOCKET_WRITE(fd, chunk, send_length) write(fd, chunk, send_length)
#define SOCKET_BIND(socket, name, namelen) bind(socket, name, namelen)
#define SOCKET_LISTEN(socket, backlog) listen(socket, backlog)
#define SOCKET_ACCEPT(socket, addr, addrlen) accept(socket, addr, addrlen)
#define SOCKET_CLOSE(socket) close(socket);
#define SOCKET_NAME(socket, addr, addrlen) getsockname(socket, addr, addrlen)

#endif
#endif
