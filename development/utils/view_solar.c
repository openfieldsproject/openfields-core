#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ofp.h>   // your header for ofpdata

int main(void)
{
  ofpdata fixdata;

  FILE *fp = fopen("/opt/ofp/wdata/monitor/solar_tracking.ofp", "rb");
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
    double az, el, pcnt;
    uint8_t *p = fixdata.data;
    memcpy(&az, p, sizeof(double));
    memcpy(&el, p + sizeof(double), sizeof(double));
    memcpy(&pcnt, p + 2*sizeof(double), sizeof(double));

    printf("Az: %.9lf\n", az);
    printf("El: %.9lf\n", el);
    printf("\%: %.3lf\n", pcnt);
  }
  else
  {
    fprintf(stderr, "Data array too small to unpack solar tracking data\n");
  }

    return EXIT_SUCCESS;
}
