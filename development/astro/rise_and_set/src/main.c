#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <ofp.h>


// Defines
#define MOD_NAME "|<change module name>|"

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
 
  ofpdata fixdata;
  //ofpdata module_data;

  FILE *fp = fopen("/opt/ofp/wdata/monitor/geo_location.ofp", "rb");
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

  if (sizeof(fixdata.data) >= 3 * sizeof(double)) 
  {
    double lat, lon, alt;
    const uint8_t *p = fixdata.data;
    memcpy(&lat, p, sizeof(double));
    memcpy(&lon, p + sizeof(double), sizeof(double));
    memcpy(&alt, p + 2*sizeof(double), sizeof(double));
  } 
  else 
  {
    ofplog(MOD_NAME " Invalid data within source.");
    return EXIT_FAILURE;
  }


  return EXIT_SUCCESS;
}
