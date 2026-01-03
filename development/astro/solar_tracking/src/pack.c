#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ofp.h>

void pack_solardata(ofpdata *solar_data, double azimuth, double elevation, double percent)
{
  if (!solar_data) return;

  memset(solar_data, 0, sizeof(*solar_data));

  solar_data->version = 1;
  solar_data->epoch = time(NULL);
  strncpy(solar_data->source, "solar_tracking", sizeof(solar_data->source)-1);

  /* Pack the three doubles */
  uint8_t *p = solar_data->data;
  memcpy(p, &azimuth, sizeof(double));
  memcpy(p + sizeof(double), &elevation, sizeof(double));
  memcpy(p + 2*sizeof(double), &percent, sizeof(double));

  /* Create OpenStreetMap URL */
  char these_notes[512];
  snprintf(these_notes,sizeof(these_notes),"3 Doubles in the data filed for 1:Azimuth = %.6lf  2:Elevation = %.6lf 3:Percent = %.6lf\n",azimuth,elevation,percent);

  strncpy(solar_data->notes, these_notes, sizeof(solar_data->notes));
}
