#ifndef __ASTRO_H_
#define __ASTRO_H_

#include <time.h>
#include <stdint.h>

#define MAXSIZE 32

// Constants
#define PI 3.14159265358979323846
#define LUNA_REV 29.5305882

typedef struct
{
  time_t epoch;
  double tz_offset;
  int day_of_year;

  double latitude;
  double longitude;

  char sunrise_local[MAXSIZE];
  char sunset_local[MAXSIZE];
  double sun_elevation;
  double sun_azimuth;
  double solar_percent;

  double phase;
  double days_next_phase;
  char phase_name[MAXSIZE];
  double lunar_illumination;

  uint8_t ok_pub;

}astro_info;

int lunar_data(astro_info * ad);

int solar_data(astro_info * ad);


#endif