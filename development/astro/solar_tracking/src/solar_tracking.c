#include <math.h>
#include <time.h>
#include <ofp.h>
#include "astro.h"


// Convert degrees to radians
double deg2rad(double deg)
{
  return deg * M_PI / 180.0;
}

// Convert radians to degrees
double rad2deg(double rad)
{
  return rad * 180.0 / M_PI;
}

// Calculate day of year from struct tm (UTC)
int day_of_year(struct tm *utc)
{
  return utc->tm_yday + 1; // tm_yday starts from 0
}

void sun_az_el(double lat, double lon, double * az_deg, double * el_deg, double * en_pcnt)
{
//

// Convert epoch to UTC time structure
  time_t now_time = get_ofptime();
  struct tm *utc = gmtime(&now_time);

  // Extract day of year and UTC hour
  int N = day_of_year(utc);
  double utc_hours = utc->tm_hour + utc->tm_min / 60.0 + utc->tm_sec / 3600.0;

  // Calculate fractional year in radians (approximate)
  double gamma = 2.0 * M_PI / 365.0 * (N - 1 + (utc_hours - 12) / 24.0);

  // Declination of the sun (radians)
  double decl = 0.006918 - 0.399912 * cos(gamma) + 0.070257 * sin(gamma)
                - 0.006758 * cos(2 * gamma) + 0.000907 * sin(2 * gamma)
                - 0.002697 * cos(3 * gamma) + 0.00148 * sin(3 * gamma);

  // Equation of time in minutes
  double eq_time = 229.18 * (0.000075 + 0.001868 * cos(gamma) - 0.032077 * sin(gamma)
                - 0.014615 * cos(2 * gamma) - 0.040849 * sin(2 * gamma));

  // Time offset in minutes
  double time_offset = eq_time + 4.0 * lon;

  // True solar time in minutes
  double tst = utc_hours * 60.0 + time_offset;

  // Solar hour angle in degrees
  double ha_deg = (tst / 4.0) - 180.0;
  double ha_rad = deg2rad(ha_deg);

  // Convert latitude to radians
  double lat_rad = deg2rad(lat);

  // Solar elevation angle (radians)
  double elevation_rad = asin(sin(lat_rad) * sin(decl) + cos(lat_rad) * cos(decl) * cos(ha_rad));
  *el_deg = rad2deg(elevation_rad);

  // Corrected azimuth angle (true north = 0°, clockwise)
  double sin_az = -sin(ha_rad) * cos(decl) / cos(elevation_rad);
  double cos_az = (sin(decl) - sin(lat_rad) * sin(elevation_rad)) / (cos(lat_rad) * cos(elevation_rad));
  double az_rad = atan2(sin_az, cos_az);

  if (az_rad < 0)
    az_rad += 2 * M_PI;

  *az_deg = rad2deg(az_rad);

  // Simple clear-sky solar energy estimate (%)
  // Based purely on solar elevation angle, zero if sun below horizon
  if (*el_deg > 0)
  {
    *en_pcnt = 100.0 * sin(elevation_rad);
  }
  else
  {
    *en_pcnt = 0.0;
  }


// 
}