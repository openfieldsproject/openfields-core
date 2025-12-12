#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <ofp.h>
#include "astro.h"

// Defines
#define MOD_NAME "|solar_tracking|"

/**
 * Main function to <desribe the purpose and functiopn of the module>
 *
*/

int main(void)
{
  ofplog(MOD_NAME " started.");
  if (ofptime_quality() < 0)
  {
    ofplog(MOD_NAME "Time is undetermined, aborting");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen("/opt/ofp/wdata/monitor/geo_location.ofp", "rb");
  if (!fp) 
  {
    ofplog(MOD_NAME "-unable to open geo location");
    return EXIT_FAILURE;
  }

  ofpdata fixdata;
  double lat, lon, alt;
  double az_deg, el_deg, energy_pcnt;

  size_t n = fread(&fixdata, 1, sizeof(fixdata), fp);
  fclose(fp);

  if (n != sizeof(fixdata)) 
  {
    ofplog(MOD_NAME "-corrupt geo location data.");
    return EXIT_FAILURE;
  }

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

  printf ("%lf    %lf     %lf\n",az_deg,el_deg,energy_pcnt);

  return EXIT_SUCCESS;
}
