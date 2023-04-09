#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <time.h>

char* day_to_str(int day);
char* mon_to_str(int mon);
void time_to_str(time_t t, char* buf);

#endif
