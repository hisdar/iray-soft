#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define ASSERT(x) if (unlikely(!(x))) \
	{ perror("assert(" __FILE__ ":" TOSTRING(__LINE__) "): "); exit(1); }

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* common.c */
void get_time_now(struct timespec *ts);
uint64_t get_time_elapsed_us(const struct timespec *ts_start, const struct timespec *ts_end);

/* send fd to another process */
ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd);
/* receive fd from another process */
ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd);

#endif
