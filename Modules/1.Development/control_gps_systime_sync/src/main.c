/*
 * TimeSync_RAMFS.c
 * NASA^10 style C program using RAMFS + inotify for time synchronization
 *
 * Behavior:
 *   - Watches CONTROL/REQUEST/timesync file for requests
 *   - Reads DATA/GPS/epoch and DATA/DATE_TIME/rtc_epoch files
 *   - Compares them and updates system time if drift exceeds TOLERANCE
 *
 * Build:
 *   gcc -o TimeSync_RAMFS TimeSync_RAMFS.c -lrt
 *
 * Run:
 *   sudo ./TimeSync_RAMFS
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#define TOLERANCE 1

#define FILE_REQUEST "/opt/ofp/wdata/broker/CONTROL/REQUEST/timesync"
#define FILE_GPS     "/opt/ofp/wdata/broker/DATA/GPS/epoch"
#define FILE_RTC     "/opt/ofp/wdata/broker/DATA/DATE_TIME/rtc_epoch"

/* Safely set system time */
static int setSystemTime(time_t new_time)
{
    struct timespec ts;
    ts.tv_sec = new_time;
    ts.tv_nsec = 0;
    if (clock_settime(CLOCK_REALTIME, &ts) != 0) {
        perror("clock_settime failed");
        return -1;
    }
    return 0;
}

/* Read epoch from a file */
static time_t readEpoch(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char buf[64];
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return 0; }
    fclose(f);
    return atol(buf);
}

int main(void)
{
    int fd = inotify_init();
    if (fd < 0) { perror("inotify_init"); return 1; }

    int wd = inotify_add_watch(fd, FILE_REQUEST, IN_MODIFY);
    if (wd < 0) { perror("inotify_add_watch"); return 1; }

    printf("Watching %s for time sync requests...\n", FILE_REQUEST);

    char buf[1024];
    while (1)
    {
        int len = read(fd, buf, sizeof(buf));
        if (len <= 0) continue;

        // Timesync request detected
        printf("Timesync request detected\n");

        time_t gps_epoch = readEpoch(FILE_GPS);
        time_t rtc_epoch = readEpoch(FILE_RTC);

        if (gps_epoch == 0 || rtc_epoch == 0) {
            printf("GPS or RTC epoch not available yet\n");
            continue;
        }

        time_t delta = gps_epoch - rtc_epoch;
        if (delta < 0) delta = -delta;

        if (delta > TOLERANCE) {
            printf("Time drift %ld seconds, updating system time...\n", (long)delta);
            if (setSystemTime(gps_epoch + 1) == 0)
                printf("System time updated to GPS epoch: %ld\n", (long)gps_epoch);
        } else {
            printf("Time drift within tolerance (%ld sec)\n", (long)delta);
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}

