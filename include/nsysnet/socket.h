#pragma once
#include <wut.h>
#include <stdint.h>


/**
 * \defgroup nsysnet_socket Socket
 * \ingroup nsysnet
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Errors returned by nsysnet socket functions
 * WARNING: these do not match with sys/errno.h (where EAGAIN is 11 for example).
 */
#define NSN_EAGAIN          6
#define NSN_EWOULDBLOCK     6


void
socket_lib_init();

void
socket_lib_finish();

int
socketclose(int sockfd);

int
socketlasterr();

#ifdef __cplusplus
}
#endif

/** @} */
