#ifndef SOCKETLOG_H
#define SOCKETLOG_H
#include <stdio.h>
#include <errno.h>

#ifndef TEMP_FAILURE_RETRY
/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })
#endif

#define LOG_HEAD(fmt) "[%s:%d] " fmt

#define SLOGD(format, ...) printf(LOG_HEAD(format),__FUNCTION__, __LINE__,  ##__VA_ARGS__)
#define SLOGI(format, ...) printf(LOG_HEAD(format),__FUNCTION__, __LINE__,  ##__VA_ARGS__)
#define SLOGW(format, ...) printf(LOG_HEAD(format),__FUNCTION__, __LINE__,  ##__VA_ARGS__)
#define SLOGE(format, ...) printf(LOG_HEAD(format),__FUNCTION__, __LINE__,  ##__VA_ARGS__)
#define SLOGV(format, ...) printf(LOG_HEAD(format),__FUNCTION__, __LINE__,  ##__VA_ARGS__)


#endif