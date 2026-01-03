// astro.h
#ifndef ASTRO_H
#define ASTRO_H
#include <math.h>
#include <time.h>
#include <ofp.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Fully tested NOAA-style solar tracker function
* Inputs:
* lat, lon: GPS coordinates in degrees (north positive, east positive)
* t: UTC time (time_t)
* Outputs:
* az_deg: azimuth, 0° = North, clockwise
* el_deg: elevation above horizon
*/
void sun_az_el(double lat, double lon, double * az_deg, double * el_deg, double * en_pcnt);
void pack_solardata(ofpdata * solar_data, double azimuth, double elevation, double percent);

#endif // ASTRO_H