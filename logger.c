#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"

static int output_fd = 1;
static log_level_t log_level = 
#ifdef O_SERVER_DEBUG
e_log_level_debug;
#else
e_log_level_info;
#endif

void logger_init(const char *file, log_level_t limit_level)
{
    switch(limit_level)
    {
        case e_log_level_debug:
        case e_log_level_info:
        case e_log_level_fatal:
            log_level = limit_level;
            break;
        default:
            break;
    }
    output_fd = open(file, O_APPEND | O_CREAT | O_RDWR);
    if(output_fd < 0)
    {
        perror("open");
        return;
    }
}
void logger_log(log_level_t level, const char *fmt, ...)
{
    int len;
    char buffer[1024];
    struct timeval log_time;
    struct tm gt;
    va_list ap;
    if(output_fd >= 0 && level >= log_level)
    {
        gettimeofday(&log_time, NULL);
        localtime_r((time_t*)&log_time.tv_sec, &gt);
        len = sprintf(buffer, "%4d/%02d/%02d %02d:%02d:%02d.%03ld : ", 
                gt.tm_year + 1900,
                gt.tm_mon,
                gt.tm_mday,
                gt.tm_hour,
                gt.tm_min,
                gt.tm_sec,
                log_time.tv_usec/1000
                );
        va_start(ap, fmt);
        len += vsnprintf(buffer + len, sizeof(buffer) - len, fmt, ap);
        va_end(ap);
        if(write(output_fd, buffer, len) < 0)
        {
            perror("write");
        }
    }
}
void logger_close()
{
    if(output_fd > 0)
    {
        close(output_fd);
    }
}
