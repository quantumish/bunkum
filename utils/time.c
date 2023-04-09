#include <stdio.h>

#include "time.h"

char* day_to_str(int day) {
    switch (day) {
    case 0:
        return "Sun";
    case 1:
        return "Mon";
    case 2:
        return "Tue";
    case 3:
        return "Wed";
    case 4:
        return "Thu";
    case 5:
        return "Fri";
    case 6:
        return "Sat";        
    }
    return "Wat";
}

char* mon_to_str(int mon) {
    switch (mon) {
    case 0:
        return "Jan";
    case 1:
        return "Feb";
    case 2:
        return "Mar";
    case 3:
        return "Apr";
    case 4:
        return "May";
    case 5:
        return "Jun";
    case 6:
        return "Jul";
    case 7:
        return "Aug";
    case 8:
        return "Sep";
    case 9:
        return "Oct";
    case 10:
        return "Nov";
    case 11:
        return "Dec";
    }
    return "Wat";
}


void time_to_str(time_t t, char* buf) {
    struct tm* gmt = gmtime(&t);
    sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT",
            day_to_str(gmt->tm_wday), gmt->tm_mday, mon_to_str(gmt->tm_mon),
            gmt->tm_year + 1900, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
}
