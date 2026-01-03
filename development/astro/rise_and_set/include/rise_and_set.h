// rise_and_set.h
#ifndef RISE_AND_SET_H
#define RISE_AND_SET_H
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


void get_daily_astro(double lat, double lon, time_t nowtime,
                     time_t *sunrise,
                     time_t *sunset,
                     double  *moon_phase);


#endif // RISE_ANDSET_H