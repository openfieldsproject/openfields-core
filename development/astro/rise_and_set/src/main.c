#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <ofp.h>
#include <math.h>
#include "rise_and_set.h"


// Defines
#define MOD_NAME "|<astro_rise_set>|"
#define OFP_INPUT "/opt/ofp/wdata/monitor/geo_location.ofp"
#define OFP_OUTPUT "/opt/ofp/wdata/monitor/daily_astro.ofp"

int main(void)
{
  ofplog(MOD_NAME " started.");
  if (ofptime_quality() < 0)
  {
    ofplog(MOD_NAME "Time is undetermined, aborting");
    return EXIT_FAILURE;
  }
 
  ofpdata fixdata;
  
  if (ofp_read(OFP_INPUT,&fixdata) == 0)
  {
    //check the flags for the reason
    // add reson code and ofp log later
    return EXIT_FAILURE;
  }

  if (sizeof(fixdata.data) >= 3 * sizeof(double)) 
  {
    double lat, lon, alt;
    double lunar_cycle_pcnt, lunar_illumination;
    const uint8_t *p = fixdata.data;
    time_t sunrise, sunset;
    time_t nowtime = time(NULL);
 
    memcpy(&lat, p, sizeof(double));
    memcpy(&lon, p + sizeof(double), sizeof(double));
    memcpy(&alt, p + 2*sizeof(double), sizeof(double));
    
    get_daily_astro(lat, lon, nowtime,&sunrise,&sunset,&lunar_cycle_pcnt);
    
    lunar_illumination = (1 - (sin(lunar_cycle_pcnt))) * 50.0;

    //debuggin 
    printf ("Rise : %jd\nSet : %jd \ncycle : %.2lf\nIllum : %.2lf\n\n", sunrise,sunset,lunar_cycle_pcnt,lunar_illumination);              
  } 
  else 
  {
    ofplog(MOD_NAME " Invalid data within source.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
