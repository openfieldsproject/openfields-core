#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

// Required for interacting with gpsd
#include <gps.h>

// Define the host and port for gpsd
#define GPSD_HOST "localhost"
#define GPSD_PORT "2947"

/**
 * Main function to connect to gpsd, read time, and set system clock.
 * This version is optimized for setting time to the nearest second.
 */
int main(void) {
    struct gps_data_t gps_data;
    int ret;
    int attempts = 0;
    // We increase max attempts slightly to account for the tighter loop logic
    const int max_attempts = 60; 
    
    // Total wait time = max_attempts * wait_time_per_loop (e.g., 60 seconds)
    const long wait_time_us = 1000000; // 1 second wait per gps_waiting call

    printf("GPS System Time Setter (Seconds Precision)\n");
    printf("Connecting to gpsd at %s:%s...\n", GPSD_HOST, GPSD_PORT);

    // 1. Open the connection to gpsd
    if ((ret = gps_open(GPSD_HOST, GPSD_PORT, &gps_data)) != 0) {
        fprintf(stderr, "Error connecting to gpsd: %s (Check if gpsd is running and accessible)\n",
                gps_errstr(ret));
        return EXIT_FAILURE;
    }

    // 2. Request time data stream from gpsd
    printf("Connection successful. Requesting time stream...\n");
    // WATCH_ENABLE activates data streaming. WATCH_JSON is typical.
    gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

    // 3. Loop to read data until a valid time fix is found
    while (attempts < max_attempts) {
        
        // Wait up to 1 second for data to arrive from gpsd
        if (!gps_waiting(&gps_data, wait_time_us)) {
            // Only print status and increment attempt if a full 1-second timeout occurred
            printf("Waiting for GPS data (Attempt %d/%d)...\n", attempts + 1, max_attempts);
            attempts++;
            continue;
        }

        // Read the data from gpsd (non-blocking read)
        if ((ret = gps_read(&gps_data, NULL, 0)) <= 0) {
            if (ret == -1) {
                 fprintf(stderr, "Error reading GPS data: %s\n", gps_errstr(ret));
            }
            // Go back to the top of the loop immediately without sleeping
            continue;
        }

        // Check for a valid fix mode (MODE_2D or MODE_3D) AND that TIME_SET flag is active.
        if (gps_data.fix.mode >= MODE_2D && (gps_data.set & TIME_SET)) {
            printf("\n--- GPS Time Fix Found ---\n");
            
            // Get the time from the 'time' field as a struct timespec
            struct timespec ts = gps_data.fix.time;
            
            // Use ts.tv_sec to check for valid time
            if (ts.tv_sec == 0) {
                printf("Warning: GPS time is zero. Waiting for a valid signal...\n");
                attempts++;
                continue;
            }

            // Format the time for display (using gmtime since GPS time is UTC)
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", gmtime(&ts.tv_sec));

            // Display time, ignoring fractional seconds
            printf("GPS UTC Time received (seconds precision): %s\n", time_str);

            // 4. Set the system time using clock_settime
            printf("Attempting to set system time...\n");
            // CLOCK_REALTIME is the system-wide real-time clock.
            if (clock_settime(CLOCK_REALTIME, &ts) == 0) {
                printf("Successfully set system time to GPS time!\n");
            } else {
                // clock_settime requires CAP_SYS_TIME (usually root permissions)
                perror("Error setting system time (clock_settime)");
                printf("Ensure the program is run with root/sudo privileges.\n");
            }

            // Time successfully read and processed, exit the loop
            break; 

        } else {
            // No time fix yet, display status and continue waiting/reading
            printf("Current status: Fix mode: %d, Time set: %s. Waiting for better fix...\n",
                   gps_data.fix.mode, (gps_data.set & TIME_SET) ? "YES" : "NO");
                   
            attempts++;
            // Immediately loop back to wait/read the next packet.
        }
    }
    
    // 5. Clean up
    gps_stream(&gps_data, WATCH_DISABLE, NULL);
    gps_close(&gps_data);
    
    if (attempts >= max_attempts) {
        fprintf(stderr, "\nFailed to get a reliable GPS time fix after %d attempts.\n", max_attempts);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
