#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ofp.h>   // your header for ofpdata

#define OFP_DAILYD "/opt/ofp/wdata/monitor/daily_astro.ofp"
#define OFP_RSD    "/opt/ofp/wdata/monitor/astro_rise_set.ofp"

int main(void)
{
  ofpdata dailyd,rsdata;

  if (ofp_read(OFP_DAILYD,&dailyd) == 0)
  {
    //check the flags for the reason
    // add reson code and ofp log later
    return EXIT_FAILURE;
  }

  if (ofp_read(OFP_RSD,&rsdata) == 0)
  {
    //check the flags for the reason
    // add reson code and ofp log later
    return EXIT_FAILURE;
  }

  printf ("Rise and set data file\n");
  printf("Version: %u\n", rsdata.version);
  printf("Epoch: %ld\n", rsdata.epoch);
  printf("Source: %s\n", rsdata.source);
  printf("Notes: %s\n", rsdata.notes);

  /* Unpack the 3 doubles from fixdata.data */
  if (sizeof(rsdata.data) >= 3 * sizeof(double)) 
  {
    time_t sunrise,sunset; 
    double orbit,illumination;
    uint8_t *p = rsdata.data;
    memcpy(&sunrise, p, sizeof(time_t));
    memcpy(&sunset, p + sizeof(time_t), sizeof(time_t));
    memcpy(&orbit, p + 2*sizeof(time_t), sizeof(double));
    memcpy(&illumination, p + 2*sizeof(time_t) + sizeof(double), sizeof(double));

    printf("Rise: %jd\n", sunrise);
    printf("Set: %jd\n", sunset);
    printf("Orbit: %.2lf\n", orbit);
    printf("Illum : %.2f\n\n", illumination);
  }
  else
  {
    fprintf(stderr, "Data array too small to unpack solar tracking data\n");
  }

  
//-------------------------------------------------
  printf("Daily stats file read\n");
  printf("Version: %u\n", dailyd.version);
  printf("Epoch: %ld\n", dailyd.epoch);
  printf("Source: %s\n", dailyd.source);
  printf("Notes: %s\n", dailyd.notes);

  /* Unpack the 3 doubles from fixdata.data */
  if (sizeof(dailyd.data) >= 4 * sizeof(int)) 
  {
    int dow,dom,doy,evd,evt;
    uint8_t *p = dailyd.data;

    memcpy(&dow, p, sizeof(int));
    memcpy(&dom, p + sizeof(int), sizeof(int));
    memcpy(&doy, p + 2*sizeof(int), sizeof(int));
    memcpy(&evd, p + 3*sizeof(int), sizeof(int));
    memcpy(&evt, p + 4*sizeof(int), sizeof(int));


    printf("Day of week: %d\n", dow);
    printf("Day of Month: %d\n", dom);
    printf("Day of Year : %d\n", doy);
    printf("Event in %d days\n", evd);
    printf("Event Type: %d\n----------\n\n\n",evt);
  }
  else
  {
    fprintf(stderr, "Data array too small to unpack solar tracking data\n");
  }







    return EXIT_SUCCESS;
}
