#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <ofp.h>
#include <rrd.h>
#include "solar_tracking.h"

// Defines
#define MOD_NAME "|solar_tracking|"

#define GEO_FILE    "/opt/ofp/wdata/monitor/geo_location.ofp"
#define OUTPUT_FILE "/opt/ofp/wdata/monitor/solar_tracking.ofp"
#define RRD_FILE    "/opt/ofp/wdata/rrd/solar_pcnt.rrd"


void update_rrd(double * solar_pct)
{
  optind = 0;   // REQUIRED even in short-lived programs
  char value[32];
  snprintf(value, sizeof(value), "N:%.2lf", * solar_pct);
  
  char *args[] = 
  {
      "update",
      RRD_FILE,
      value
  };
 
  if (rrd_update(3, args) != 0) 
  {
      ofplog(MOD_NAME "RRD update error");
      rrd_clear_error();
      return;
  }

  return;
}



int main(void)
{
  ofplog(MOD_NAME " started.");
  if (ofptime_quality() < 0)
  {
    ofplog(MOD_NAME "Time is undetermined, aborting");
    return EXIT_FAILURE;
  }

  ofpdata fixdata;
  double lat, lon, alt;
  double az_deg, el_deg, energy_pcnt;

  int read_status = ofp_read(GEO_FILE, &fixdata);

  if (read_status == 0)
  {
    return EXIT_FAILURE;
  }

  else 
  {
    /* Unpack the 3 doubles from fixdata.data */
    if (sizeof(fixdata.data) >= 3 * sizeof(double)) 
    {
      uint8_t *p = fixdata.data;
      memcpy(&lat, p, sizeof(double));
      memcpy(&lon, p + sizeof(double), sizeof(double));
      memcpy(&alt, p + 2*sizeof(double), sizeof(double));
    } 
    else 
    {
      ofplog(MOD_NAME "-corrupt geo location data.");
      return EXIT_FAILURE;
    }

    sun_az_el(lat,lon,&az_deg,&el_deg, &energy_pcnt);

    ofpdata solar_data;
    pack_solardata(&solar_data,az_deg,el_deg,energy_pcnt);

    if ( ofp_write(OUTPUT_FILE, &solar_data) != 1)
    {
      ofplog(MOD_NAME "Error writing ofp file");
      return EXIT_FAILURE;  
    }

    update_rrd(&energy_pcnt);
    ofplog(MOD_NAME "RRD update complete");
    return EXIT_SUCCESS;
  }
}
