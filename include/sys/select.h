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

struct timeval
{
   long tv_sec;
   long tv_usec;
};

typedef uint32_t __fd_mask;

typedef struct fd_set {
   __fd_mask __fds_bits;
} fd_set;

#define __NFDBITS (8 * (int) sizeof (__fd_mask))
#define __FD_BITS(set) ((set)->__fds_bits)
#define __FD_MASK(d)   ((__fd_mask) (1U << ((d) % __NFDBITS)))

#define FD_SETSIZE (32)

#define FD_CLR(d, set) \
    ((void) (__FD_BITS(set) &= ~__FD_MASK(d)))

#define FD_SET(d, set) \
    ((void) (__FD_BITS(set) |= __FD_MASK(d)))

#define FD_COPY(src, dst) \
    ((void) (__FD_BITS(dst) = __FD_BITS(src)))

#define FD_ISSET(d, set) \
    ((__FD_BITS(set) & __FD_MASK(d)) != 0)

#define FD_ZERO(set) \
    ((void) (__FD_BITS(set) = 0))

int
select(int nfds,
       fd_set *readfds,
       fd_set *writefds,
       fd_set *exceptfds,
       struct timeval *timeout);

#ifdef __cplusplus
}
#endif

/** @} */
