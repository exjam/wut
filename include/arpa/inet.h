#pragma once
#include <wut.h>
#include <stdint.h>
#include <netinet/in.h>

/**
 * \ingroup nsysnet
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t socklen_t;

int
inet_aton(const char *cp, struct in_addr *inp);

const char *
inet_ntoa(struct in_addr in);

char *
inet_ntoa_r(struct in_addr in, char *buf);

const char *
inet_ntop(int af, const void *src, char *dst, socklen_t size);

int
inet_pton(int af, const char *src, void *dst);

#ifdef __cplusplus
}
#endif

/** @} */
