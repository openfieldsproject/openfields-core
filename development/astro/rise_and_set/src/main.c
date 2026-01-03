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


void packdata (time_t sunrise,time_t sunset,double lunar_cycle_pcnt,double lunar_illumination, ofpdata * output_data)
{
  if (!output_data) return;

  memset(output_data, 0, sizeof(*output_data));

  output_data->version = 1;
  output_data->epoch = time(NULL);
  strncpy(output_data->source, "ASTRO rise_and_set", sizeof(output_data->source)-1);

  /* Pack the calculated data */
  uint8_t *p = output_data->data;
  memcpy(p, &sunrise, sizeof(time_t));
  memcpy(p + sizeof(time_t), &sunset, sizeof(time_t));
  memcpy(p + 2*sizeof(time_t), &lunar_cycle_pcnt, sizeof(double));
  memcpy(p + 2*sizeof(time_t) + sizeof(double), &lunar_illumination, sizeof(double));

  snprintf(output_data->notes,sizeof(output_data->notes),"Sunrise (Epoch), Sunset (Epoch), Lunar Cycle Percentage \
    and Lunar Illumination, each one is 8 bytes - times are in time_t and percentages are doubles");


}

int main(void)
{
  ofplog(MOD_NAME " started.");
  if (ofptime_quality() < 0)
  {
    ofplog(MOD_NAME "Time is undetermined, aborting");
    return EXIT_FAILURE;
  }
 
  ofpdata fixdata,output_data;
  
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
    
    lunar_illumination = (1 - (cos(lunar_cycle_pcnt/50.0*M_PI))) * 50.0;

    //debuggin 
    //printf ("Rise : %jd\nSet : %jd \ncycle : %.2lf\nIllum : %.2lf\n\n", sunrise,sunset,lunar_cycle_pcnt,lunar_illumination);              

    packdata (sunrise,sunset,lunar_cycle_pcnt,lunar_illumination,&output_data);
    
    if ( ofp_write(OFP_OUTPUT, &output_data) != 1)
    {
      ofplog(MOD_NAME "Error writing ofp file");
      return EXIT_FAILURE;  
    }

    ofplog(MOD_NAME "Output complete");

  } 
  else 
  {
    ofplog(MOD_NAME " Invalid data within source.");
    return EXIT_FAILURE;
  }


  return EXIT_SUCCESS;
}
