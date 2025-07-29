#ifndef __ASTRO_H_
#define __ASTRO_H_

#include <time.h>
#include <stdint.h>

#define MAXSIZE 32

typedef struct
{
  time_t epoch;
  int day_of_year;

  double latitude;
  double longitude;

  double sunrise_utc;
  double sunset_utc;
  double sun_elevation;
  double sun_azimuth;
  double solar_percent;

  double phase;
  char phase_name[MAXSIZE];
  double lunar_illumination;

  uint8_t ok_pub;

}astro_info;

int lunar_data(astro_info * ad);

int solar_data(astro_info * ad);


#endif