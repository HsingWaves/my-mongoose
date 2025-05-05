
#include <stdio.h>
#include <time.h>

int main() {
    struct tm utc_time = {0};

    // Set the UTC time fields
    utc_time.tm_year = 2025 - 1900;  // Years since 1900
    utc_time.tm_mon  = 4 - 1;        // Months since January (0-11)
    utc_time.tm_mday = 5;            // Day of the month (1-31)
    utc_time.tm_hour = 12;           // Hours since midnight (0-23)
    utc_time.tm_min  = 30;           // Minutes after the hour (0-59)
    utc_time.tm_sec  = 0;            // Seconds after the minute (0-59)

    // Convert UTC struct tm to time_t (epoch time, UTC based)
    time_t epoch_time = _mkgmtime(&utc_time); 
    if (epoch_time == -1) {
        printf("Failed to convert time.\n");
    } else {
        printf("Epoch time (UTC): %lld\n", (long long)epoch_time);

        // Convert back to UTC time for verification
        struct tm *verified_time = gmtime(&epoch_time);
        printf("Verified UTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
            verified_time->tm_year + 1900,
            verified_time->tm_mon + 1,
            verified_time->tm_mday,
            verified_time->tm_hour,
            verified_time->tm_min,
            verified_time->tm_sec
        );
    }

    return 0;
}
