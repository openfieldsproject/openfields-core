#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ofp.h>
#include <gps.h>

// Define the host and port for gpsd
#define GPSD_HOST "localhost"
#define GPSD_PORT "2947"

#define MOD_NAME "|set_gpsdposition|"
#define ofp_output "/opt/ofp/wdata/monitor/geo_location.ofp"

/**
 * Main function to connect to gpsd, read altitude, latitude and longitude.
 *
*/

int main(void) 
{
  struct gps_data_t gps_data;
  int ret;
  int attempts = 0;

  // We increase max attempts slightly to account for the tighter loop logic
  const int max_attempts = 60;

  // Total wait time = max_attempts * wait_time_per_loop (e.g., 60 seconds)
  const long wait_time_us = 1000000; // 1 second wait per gps_waiting call

  ofplog(MOD_NAME"Connecting to gpsd " GPSD_HOST":" GPSD_PORT);

  // 1. Open the connection to gpsd
  if ((ret = gps_open(GPSD_HOST, GPSD_PORT, &gps_data)) != 0) 
  {
    ofplog(MOD_NAME"Error connecting to gpsd:(Check if gpsd is running and accessible)");
    return EXIT_FAILURE;
  }

  // 2. Request time data stream from gpsd
  // WATCH_ENABLE activates data streaming. WATCH_JSON is typical.
  gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

  // 3. Loop to read data until a valid time fix is found
  while (attempts < max_attempts) 
  {
    // Wait up to 1 second for data to arrive from gpsd
    if (!gps_waiting(&gps_data, wait_time_us)) 
    {
      // Only print status and increment attempt if a full 1-second timeout occurred
      attempts++;
      continue;
    }

    // Read the data from gpsd (non-blocking read)
    if ((ret = gps_read(&gps_data, NULL, 0)) <= 0) 
      {
        if (ret == -1) 
          ofplog(MOD_NAME"Error reading GPS data:");
        
        // Go back to the top of the loop immediately without sleeping
        continue;
      }

      // Check for a valid fix mode (MODE_2D or MODE_3D) AND that TIME_SET flag is active.
      if (gps_data.fix.mode >= MODE_2D && (gps_data.set & TIME_SET)) 
      {
        ofpdata fixdata;
        memset(&fixdata, 0, sizeof fixdata);
      
        void pack_gps_with_osm_link(ofpdata *fixdata, double lat, double lon, double alt);

        pack_gps_with_osm_link(&fixdata,
                       gps_data.fix.latitude,
                       gps_data.fix.longitude,
                       gps_data.fix.altitude);

        ofplog(MOD_NAME"GPS Fix Found - Logging position");
        ofp_write(ofp_output,&fixdata);
        break;
      } 
      else 
        attempts++;
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
