#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ofp.h>

#define MOD_NAME "|set_gpsdtime|"

// Required for interacting with gpsd
#include <gps.h>

// Define the host and port for gpsd
#define GPSD_HOST "localhost"
#define GPSD_PORT "2947"

/**
 * Main function to connect to gpsd, read time, and set system clock.
 * This version is optimized for setting time to the nearest second.
 */
int main(int argc, const char * const argv[]) 
{
  if (argc > 1 && strcmp(argv[1], "-d") == 0)
  {
    ofplog(MOD_NAME"INFO: -d option: set_gpsdtime@boot waiting 60 seconds.");
    sleep(60);
  }
  struct gps_data_t gps_data;
  int ret;
  int attempts = 0;
  // We increase max attempts slightly to account for the tighter loop logic
  const int max_attempts = 500;

  // Total wait time = max_attempts * wait_time_per_loop (e.g., 60 seconds)
  const long wait_time_us = 1000000; // 1 second wait per gps_waiting call

  ofplog(MOD_NAME"Setting time via GPSD on " GPSD_HOST ":"GPSD_PORT".");

  // 1. Open the connection to gpsd
  if ((ret = gps_open(GPSD_HOST, GPSD_PORT, &gps_data)) != 0) 
  {
    ofplog(MOD_NAME"Error connecting to GPSD.");
    return EXIT_FAILURE;
  }

  // 2. Request time data stream from gpsd
  ofplog(MOD_NAME"Connection successful. Requesting time stream.");
  // WATCH_ENABLE activates data streaming. WATCH_JSON is typical.
  gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

  // 3. Loop to read data until a valid time fix is found
  while (attempts < max_attempts) 
  {
    // Wait up to 1 second for data to arrive from gpsd
    if (!gps_waiting(&gps_data, wait_time_us)) 
    {
      attempts++;
      continue;
    }

    // Read the data from gpsd (non-blocking read)
    if ((ret = gps_read(&gps_data, NULL, 0)) <= 0) 
    {
      if (ret == -1) 
      {
        ofplog(MOD_NAME"Error reading GPS data.");
      }
      // Go back to the top of the loop immediately without sleeping
      continue;
    }

    // Check for a valid fix mode (MODE_2D or MODE_3D) AND that TIME_SET flag is active.
    if (gps_data.fix.mode >= MODE_2D && (gps_data.set & TIME_SET)) 
    {
      ofplog(MOD_NAME"GPS Time Fix Found");
            
      // Get the time from the 'time' field as a struct timespec
      struct timespec ts = gps_data.fix.time;
            
      // Use ts.tv_sec to check for valid time
      if (ts.tv_sec == 0) 
      {
        attempts++;
        continue;
      }

      // Format the time for display (using gmtime since GPS time is UTC)
      char time_str[64];
      strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", gmtime(&ts.tv_sec));

      // Display time, ignoring fractional seconds
      ofplog(MOD_NAME"GPS UTC Time received (seconds precision)");

      // 4. Set the system time using clock_settime
      ofplog(MOD_NAME"Attempting to set system time.");
      // CLOCK_REALTIME is the system-wide real-time clock.
      if (clock_settime(CLOCK_REALTIME, &ts) == 0) 
      {
        ofpdata data;
        data.version = 1;
        data.flags = 0;
        data.epoch = time(NULL);
        strcpy (data.notes,"No data required, as epoch is the data.");

        ofp_write("/opt/ofp/wdata/monitor/timesync.ofp", &data);
        
        ofplog(MOD_NAME"Successfully set system time to GPS time!");
      } 
      else 
      {
        // clock_settime requires CAP_SYS_TIME (usually root permissions)
        ofplog(MOD_NAME"Error setting system time (clock_settime)");
        ofplog(MOD_NAME"Ensure the program is run with root/sudo privileges.\n");
      }

      // Time successfully read and processed, exit the loop
      break; 

    }
    else                
      attempts++;
    
  }
    
  // 5. Clean up
  gps_stream(&gps_data, WATCH_DISABLE, NULL);
  gps_close(&gps_data);
    
  if (attempts >= max_attempts) 
  {
    ofplog(MOD_NAME"Failed to get a reliable GPS time fix after 500 attempts.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


