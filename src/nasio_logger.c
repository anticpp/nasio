
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>


#ifdef NASIO_DEBUG

#define NASIO_MAX_LOG_LEN 2048

static const char *nasio_log_level_name[] = {
        "ALL", "TRACE", "INFO", "DEBUG",
        "WARN", "ERROR", "FATAL"
};

void default_log_cb(int level, const char *fmt, ...) {
    char tmp[NASIO_MAX_LOG_LEN] = {""};

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp)-1, fmt, ap);
    va_end(ap);

    struct timeval tv;
    gettimeofday(&tv, 0);

    struct tm tm;
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE || _POSIX_SOURCE
    localtime_r( (time_t *)&(tv.tv_sec), &tm );
#else
    /*
     * Well, suppose it will never fails.
     */
    tm = *localtime( (time_t *)&(tv.tv_sec) );
#endif

    fprintf(stderr, "[%04d%02d%02d %02d:%02d:%02d.%d] [%s] %s\n"
                    , 1900+tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec
                    , nasio_log_level_name[level], tmp);
}

#endif //#ifdef NASIO_DEBUG
