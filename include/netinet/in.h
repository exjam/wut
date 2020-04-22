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

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
typedef uint16_t sa_family_t;

struct in_addr
{
   in_addr_t s_addr;
};

struct sockaddr_in
{
   sa_family_t    sin_family;
   in_port_t      sin_port;
   struct in_addr sin_addr;
   unsigned char  sin_zero[8];
};

#define IPPROTO_IP         0
#define IPPROTO_TCP        6
#define IPPROTO_UDP        17

#define INADDR_ANY         0
#define INADDR_BROADCAST   0xFFFFFFFF

uint32_t
htonl(uint32_t val);

uint16_t
htons(uint16_t val);

uint32_t
ntohl(uint32_t val);

uint16_t
ntohs(uint16_t val);

#ifdef __cplusplus
}
#endif

/** @} */
