#ifndef O_LOGGER
#define O_LOGGER
typedef enum
{
    e_log_level_debug,
    e_log_level_info,
    e_log_level_error,
    e_log_level_fatal,
}log_level_t;
void logger_init(const char *file, log_level_t limit_level);
void logger_log(log_level_t level, const char *fmt, ...);
void logger_close();
#define l_og(level, fmt, ...) logger_log(level, "%15s: line %-3d -- "fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define l_og_error(msg) logger_log(e_log_level_fatal, "%15s: line %-3d ** %s: %s\n", __FILE__, __LINE__, msg, strerror(errno))
#endif /* O_LOGGER */
