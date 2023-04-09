#ifndef LOG_H
#define LOG_H

enum log_level {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

void log_msg(enum log_level lvl, char* lvl_name, char* fmt, ...);
#define VA_ARGS(...) , ##__VA_ARGS__
#define log_trace(fmt, ...) log_msg(LOG_TRACE, "TRACE", fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_msg(LOG_DEBUG, "DEBUG", fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) log_msg(LOG_INFO, "INFO", fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) log_msg(LOG_WARN, "WARN", fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_msg(LOG_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...) log_msg(LOG_FATAL, "FATAL", fmt, ##__VA_ARGS__)
void log_set_min_lvl(enum log_level lvl);
void die(char* p);

#endif
