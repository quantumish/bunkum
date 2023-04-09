#include <stdio.h>

#include "time.h"

const char* days[] = {"Sun", "Mon", "Tue", "Wed",  "Thu", "Fri", "Sat"};
const char* months[] =
    {"Jan", "Feb", "Mar", "Apr",  "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void time_to_str(time_t t, char* buf) {
    struct tm* gmt = gmtime(&t);
    sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT",
            days[gmt->tm_wday], gmt->tm_mday, months[gmt->tm_mon],
            gmt->tm_year + 1900, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
}
