#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

#define LOG_MAX_MSGLEN 128

#define ANSI_RED "\x1b[31m"
#define ANSI_BOLDRED "\x1b[1;31m"
#define ANSI_GREY "\x1b[38;5;239m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_MAGENTA "\x1b[36m"
#define ANSI_RESET "\x1b[0m"

char* lvl_to_color(enum log_level lvl) {
    switch (lvl) {
    case LOG_TRACE:
        return ANSI_MAGENTA;
    case LOG_DEBUG:
        return ANSI_GREEN;
    case LOG_INFO:
        return ANSI_CYAN;
    case LOG_WARN:
        return ANSI_YELLOW;
    case LOG_ERROR:
        return ANSI_RED;
    case LOG_FATAL:
        return ANSI_BOLDRED;
    }
}

size_t log_min_level = 0;

void log_msg(enum log_level lvl, char* lvl_name, char* fmt, ...) {
    if (lvl < log_min_level) return;
    
    char msg[LOG_MAX_MSGLEN];
    
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);

    time_t now = time(0);
    struct tm* local = localtime(&now);

    char* lvl_color = lvl_to_color(lvl);
    
    printf(ANSI_GREY "%02d/%02d/%02d %02d:%02d:%02d %s%s" ANSI_RESET ": %s\n",
           local->tm_mon, local->tm_mday, local->tm_year,
           local->tm_hour, local->tm_min, local->tm_sec,
           lvl_color, lvl_name, msg);
}
