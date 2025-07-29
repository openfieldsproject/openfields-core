#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "astro.h"

#define PI 3.14159265358979323846

double get_moon_phase(struct tm *date)
{
    int year = date->tm_year + 1900;
    int month = date->tm_mon + 1;
    int day = date->tm_mday;

    if (month < 3)
    {
        year--;
        month += 12;
    }

    ++month;
    long c = (long) (365.25 * year);
    long e = (long) (30.6 * month);
    double jd = (double) c + (double)e + (double) day - 694039.09;  // jd is total days since known new moon (Dec 31 1999)
    jd /= 29.5305882;                    // divide by lunar cycle (29.53 days)
    double phase = jd - floor(jd);       // get fractional part of the cycle
    return phase;
}

const char* get_phase_name(double phase)
{
    if (phase < 0.03 || phase > 0.97)
        return "New Moon";
    else if (phase < 0.22)
        return "Waxing Crescent";
    else if (phase < 0.28)
        return "First Quarter";
    else if (phase < 0.47)
        return "Waxing Gibbous";
    else if (phase < 0.53)
        return "Full Moon";
    else if (phase < 0.72)
        return "Waning Gibbous";
    else if (phase < 0.78)
        return "Last Quarter";
    else
        return "Waning Crescent";
}

// Convert degrees to radians
double deg2rad(double deg)
{
    return deg * PI / 180.0;
}

// Convert radians to degrees
double rad2deg(double rad)
{
    return rad * 180.0 / PI;
}

// Calculate day of year from struct tm (UTC)
int day_of_year(struct tm *utc)
{
    return utc->tm_yday + 1; // tm_yday starts from 0
}

void calculate_sunrise_sunset_utc(astro_info * ad)
{
    double zenith = 90.833; // official zenith for sunrise/sunset
    double D2R = PI / 180.0;
    double R2D = 180.0 / PI;

    // convert longitude to hour value
    double lngHour = ad->longitude / 15.0;

    // approximate time
    double tRise = ad->day_of_year + ((6.0 - lngHour) / 24.0);
    double tSet  = ad->day_of_year + ((18.0 - lngHour) / 24.0);

    // mean anomaly
    double MRise = (0.9856 * tRise) - 3.289;
    double MSet  = (0.9856 * tSet) - 3.289;

    // true longitude
    double LRise = MRise + (1.916 * sin(D2R * MRise)) + (0.020 * sin(2 * D2R * MRise)) + 282.634;
    double LSet  = MSet + (1.916 * sin(D2R * MSet)) + (0.020 * sin(2 * D2R * MSet)) + 282.634;

    if (LRise >= 360.0) LRise -= 360.0;
    if (LSet >= 360.0)  LSet -= 360.0;

    // right ascension
    double RARise = R2D * atan(0.91764 * tan(D2R * LRise));
    double RASet  = R2D * atan(0.91764 * tan(D2R * LSet));

    // quadrant adjustment
    double LquadrantRise  = floor(LRise / 90.0) * 90.0;
    double RAquadrantRise = floor(RARise / 90.0) * 90.0;
    RARise = RARise + (LquadrantRise - RAquadrantRise);

    double LquadrantSet  = floor(LSet / 90.0) * 90.0;
    double RAquadrantSet = floor(RASet / 90.0) * 90.0;
    RASet = RASet + (LquadrantSet - RAquadrantSet);

    // convert to hours
    RARise /= 15.0;
    RASet  /= 15.0;

    // declination of the sun
    double sinDecRise = 0.39782 * sin(D2R * LRise);
    double cosDecRise = cos(asin(sinDecRise));

    double sinDecSet = 0.39782 * sin(D2R * LSet);
    double cosDecSet = cos(asin(sinDecSet));

    // local hour angle
    double cosHRise = (cos(D2R * zenith) - (sinDecRise * sin(D2R * ad->latitude))) /
                      (cosDecRise * cos(D2R * ad->latitude));
    double cosHSet = (cos(D2R * zenith) - (sinDecSet * sin(D2R * ad->latitude))) /
                     (cosDecSet * cos(D2R * ad->latitude));

    if (cosHRise > 1 || cosHSet < -1)
    {
        ad->sunrise_utc = -1;
        ad->sunset_utc = -1;
        return;
    }

    double HRise = 360.0 - R2D * acos(cosHRise);
    double HSet  = R2D * acos(cosHSet);

    HRise /= 15.0;
    HSet  /= 15.0;

    // local mean time of rising/setting
    double TRise = HRise + RARise - (0.06571 * tRise) - 6.622;
    double TSet  = HSet + RASet  - (0.06571 * tSet)  - 6.622;

    // adjust back to UTC
    ad->sunrise_utc = fmod((TRise - lngHour + 24.0), 24.0);
    ad->sunset_utc  = fmod((TSet  - lngHour + 24.0), 24.0);
}

// Main solar position calculation function
void calculate_solar_position(astro_info * ad)
{

    // Convert epoch to UTC time structure
    struct tm *utc = gmtime(&ad->epoch);

    // Extract day of year and UTC hour
    int N = day_of_year(utc);
    double utc_hours = utc->tm_hour + utc->tm_min / 60.0 + utc->tm_sec / 3600.0;

    // Calculate fractional year in radians (approximate)
    double gamma = 2.0 * PI / 365.0 * (N - 1 + (utc_hours - 12) / 24.0);

    // Declination of the sun (radians)
    double decl = 0.006918 - 0.399912 * cos(gamma) + 0.070257 * sin(gamma)
                  - 0.006758 * cos(2 * gamma) + 0.000907 * sin(2 * gamma)
                  - 0.002697 * cos(3 * gamma) + 0.00148 * sin(3 * gamma);

    // Equation of time in minutes
    double eq_time = 229.18 * (0.000075 + 0.001868 * cos(gamma) - 0.032077 * sin(gamma)
                    - 0.014615 * cos(2 * gamma) - 0.040849 * sin(2 * gamma));

    // Time offset in minutes
    double time_offset = eq_time + 4.0 * ad->longitude;

    // True solar time in minutes
    double tst = utc_hours * 60.0 + time_offset;

    // Solar hour angle in degrees
    double ha_deg = (tst / 4.0) - 180.0;
    double ha_rad = deg2rad(ha_deg);

    // Convert latitude to radians
    double lat_rad = deg2rad(ad->latitude);

    // Solar elevation angle (radians)
    double elevation_rad = asin(sin(lat_rad) * sin(decl) + cos(lat_rad) * cos(decl) * cos(ha_rad));
    ad->sun_elevation = rad2deg(elevation_rad);

    // Corrected azimuth angle (true north = 0°, clockwise)
    double sin_az = -sin(ha_rad) * cos(decl) / cos(elevation_rad);
    double cos_az = (sin(decl) - sin(lat_rad) * sin(elevation_rad)) / (cos(lat_rad) * cos(elevation_rad));
    double az_rad = atan2(sin_az, cos_az);

    if (az_rad < 0)
        az_rad += 2 * PI;

    ad->sun_azimuth = rad2deg(az_rad);

    // Simple clear-sky solar energy estimate (%)
    // Based purely on solar elevation angle, zero if sun below horizon
    if (ad->sun_elevation > 0)
    {
        ad->solar_percent = 100.0 * sin(elevation_rad);
    } else
    {
        ad->solar_percent = 0.0;
    }
}

void printTime(double hour, int offset)
{
    int h = ((int)hour + offset) % 24;
    int m = (int)round((hour - floor(hour)) * 60);
    if (m == 60) {
        m = 0;
        h = (h + 1) % 24;
    }
    printf("%02d:%02d\n",h, m);
}

int lunar_data(astro_info * ad)
{
  double phase = 0.0;
  struct tm *utc = gmtime(&ad->epoch);
  calculate_sunrise_sunset_utc(ad);

  if (ad->sunrise_utc < 0 || ad->sunset_utc < 0)
  {
      printf("Sun never rises or sets on this location/date.\n");
      return 1;
  }

  // Print UTC
  printTime(ad->sunrise_utc, 0);
  printTime(ad->sunset_utc, 0);

  phase = get_moon_phase(utc);
  printf("Moon phase: %.2f (%s)\n", phase, get_phase_name(phase));

  return EXIT_SUCCESS;
}

int solar_data(astro_info * ad)
{
    calculate_solar_position(ad);

    printf("Solar Elevation: %.2f°\n", ad->sun_elevation);
    printf("Solar Azimuth: %.2f°\n", ad->sun_azimuth);
    printf("Available Solar Energy: %.1f%% (clear-sky estimate)\n", ad->solar_percent);

    return EXIT_SUCCESS;
}

