#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ofp.h>   // your header for ofpdata

int main(void)
{
  ofpdata fixdata;

  FILE *fp = fopen("/opt/ofp/wdata/monitor/daily_astro.ofp", "rb");
  if (!fp) 
  {
    perror("fopen");
    return EXIT_FAILURE;
  }

  size_t n = fread(&fixdata, 1, sizeof(fixdata), fp);
  fclose(fp);

  if (n != sizeof(fixdata)) 
  {
    fprintf(stderr, "Error: read %zu bytes, expected %zu\n", n, sizeof(fixdata));
    return EXIT_FAILURE;
  }

  printf("Version: %u\n", fixdata.version);
  printf("Epoch: %ld\n", fixdata.epoch);
  printf("Source: %s\n", fixdata.source);
  printf("Notes: %s\n", fixdata.notes);

  /* Unpack the 3 doubles from fixdata.data */
  if (sizeof(fixdata.data) >= 3 * sizeof(double)) 
  {
    time_t sunrise,sunset; 
    double orbit,illumination;
    uint8_t *p = fixdata.data;
    memcpy(&sunrise, p, sizeof(time_t));
    memcpy(&sunset, p + sizeof(time_t), sizeof(time_t));
    memcpy(&orbit, p + 2*sizeof(time_t), sizeof(double));
    memcpy(&illumination, p + 2*sizeof(time_t) + sizeof(double), sizeof(double));

    printf("Rise: %jd\n", sunrise);
    printf("Set: %jd\n", sunset);
    printf("Orbit: %.2lf\n", orbit);
    printf("Illum : %.2f\n", illumination);
  }
  else
  {
    fprintf(stderr, "Data array too small to unpack solar tracking data\n");
  }

    return EXIT_SUCCESS;
}
