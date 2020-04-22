#pragma once
#include <wut.h>
#include <stdint.h>

/**
 * \ingroup nsysnet
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;

struct sockaddr
{
   sa_family_t sa_family;
   char sa_data[];
};

struct sockaddr_storage
{
   sa_family_t ss_family;
   char ss_padding[14];
};

struct linger
{
   int l_onoff;
   int l_linger;
};

#define SOCK_STREAM     1
#define SOCK_DGRAM      2

#define SOL_SOCKET      -1

#define SO_REUSEADDR    0x0004      //! Reuse of local addresses is supported.
#define SO_KEEPALIVE    0x0008      //! Connections are kept alive with periodic messages.
#define SO_DONTROUTE    0x0010      //! Bypass normal routing.
#define SO_BROADCAST    0x0020      //! Transmission of broadcast messages is supported.
#define SO_LINGER       0x0080      //! Socket lingers on close.
#define SO_OOBINLINE    0x0100      //! Out-of-band data is transmitted in line.
#define SO_TCPSACK      0x0200      //! Set tcp selective acknowledgment.
#define SO_WINSCALE     0x0400      //! Set tcp window scaling.
#define SO_SNDBUF       0x1001      //! Send buffer size.
#define SO_RCVBUF       0x1002      //! Receive buffer size.
#define SO_TYPE         0x1008      //! Socket type.
#define SO_ERROR        0x1009      //! Socket error status.
#define SO_NBIO         0x1014      //! Set socket to NON-blocking mode.
#define SO_BIO          0x1015      //! Set socket to blocking mode.
#define SO_NONBLOCK     0x1016      //! Set/get blocking mode via optval param.

#define MSG_OOB         0x0001
#define MSG_PEEK        0x0002
#define MSG_DONTROUTE   0x0004
#define MSG_EOR   		0x0010
#define MSG_DONTWAIT    0x0020

#define PF_UNSPEC       0
#define PF_INET         2
#define PF_INET6        23

#define AF_UNSPEC       PF_UNSPEC
#define AF_INET         PF_INET
#define AF_INET6        PF_INET6

#define SHUT_RD         0
#define SHUT_WR         1
#define SHUT_RDWR       2

int
accept(int sockfd,
       struct sockaddr *addr,
       socklen_t *addrlen);

int
bind(int sockfd,
     const struct sockaddr *addr,
     socklen_t addrlen);

int
connect(int sockfd,
        const struct sockaddr *addr,
        socklen_t addrlen);

int
getpeername(int sockfd,
            struct sockaddr *addr,
            socklen_t *addrlen);

int
getsockname(int sockfd,
            struct sockaddr *addr,
            socklen_t *addrlen);

int
getsockopt(int sockfd,
           int level,
           int optname,
           void *optval,
           socklen_t *optlen);

int
listen(int sockfd,
       int backlog);

int
recv(int sockfd,
     void *buf,
     size_t len,
     int flags);

int
recvfrom(int sockfd,
         void *buf,
         size_t len,
         int flags,
         struct sockaddr *src_addr,
         socklen_t *addrlen);

int
send(int sockfd,
     const void *buf,
     size_t len,
     int flags);

int
sendto(int sockfd,
       const void *buf,
       size_t len,
       int flags,
       const struct sockaddr *dest_addr,
       socklen_t addrlen);

int
setsockopt(int sockfd,
           int level,
           int optname,
           const void *optval,
           socklen_t optlen);

int
shutdown(int sockfd,
         int how);

int
socket(int domain,
       int type,
       int protocol);

#ifdef __cplusplus
}
#endif

/** @} */
