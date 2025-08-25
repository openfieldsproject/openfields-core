#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "astro.h"

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

// Main solar position calculation function
void calculate_solar_position(time_t epoch, double latitude_deg, double longitude_deg,
                              double *elevation_deg, double *azimuth_deg, double *solar_energy_percent)
{

  // Convert epoch to UTC time structure
  struct tm *utc = gmtime(&epoch);

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
  double time_offset = eq_time + 4.0 * longitude_deg;

  // True solar time in minutes
  double tst = utc_hours * 60.0 + time_offset;

  // Solar hour angle in degrees
  double ha_deg = (tst / 4.0) - 180.0;
  double ha_rad = deg2rad(ha_deg);

  // Convert latitude to radians
  double lat_rad = deg2rad(latitude_deg);

  // Solar elevation angle (radians)
  double elevation_rad = asin(sin(lat_rad) * sin(decl) + cos(lat_rad) * cos(decl) * cos(ha_rad));
  *elevation_deg = rad2deg(elevation_rad);

  // Corrected azimuth angle (true north = 0°, clockwise)
  double sin_az = -sin(ha_rad) * cos(decl) / cos(elevation_rad);
  double cos_az = (sin(decl) - sin(lat_rad) * sin(elevation_rad)) / (cos(lat_rad) * cos(elevation_rad));
  double az_rad = atan2(sin_az, cos_az);

  if (az_rad < 0)
    az_rad += 2 * PI;

  *azimuth_deg = rad2deg(az_rad);

  // Simple clear-sky solar energy estimate (%)
  // Based purely on solar elevation angle, zero if sun below horizon
  if (*elevation_deg > 0)
  {
    *solar_energy_percent = 100.0 * sin(elevation_rad);
  }
  else
  {
    *solar_energy_percent = 0.0;
  }
}

int solar_data(astro_info * ad)
{
  double latitude = ad->latitude;
  double longitude = ad->longitude;
  double elevation =  0.0;
  double azimuth = 0.0;
  double solar_energy = 0.0;

  // Use current time as UTC epoch
  time_t current_epoch = ad->epoch;

  calculate_solar_position(current_epoch, latitude, longitude, &elevation, &azimuth, &solar_energy);

  ad->sun_elevation = elevation;
  ad->sun_azimuth = azimuth;
  ad->solar_percent = solar_energy;
  return EXIT_SUCCESS;
}

/************************************************************************************************/

double getMoonPhase(struct tm *date)
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
  long c = (long)  (365.25 * year);
  long e = (long) (30.6 * month);
  double jd = (double) (c + e + day) - 694039.09;  // jd is total days since known new moon (Dec 31 1999)
  jd /= 29.5305882;                    // divide by lunar cycle (29.53 days)
  double phase = jd - floor(jd);       // get fractional part of the cycle
  return phase;
}

void getPhaseName(astro_info * ad)
{
  if (ad->phase < 0.03 || ad->phase > 0.97)
    strncpy(ad->phase_name, "New Moon", MAXSIZE);
  else if (ad->phase < 0.22)
    strncpy(ad->phase_name, "Waxing Crescent", MAXSIZE);
  else if (ad->phase < 0.28)
    strncpy(ad->phase_name, "First Quarter", MAXSIZE);
  else if (ad->phase < 0.47)
    strncpy(ad->phase_name, "Waxing Gibbous", MAXSIZE);
  else if (ad->phase < 0.53)
    strncpy(ad->phase_name, "Full Moon", MAXSIZE);
  else if (ad->phase < 0.72)
    strncpy(ad->phase_name, "Waning Gibbous", MAXSIZE);
  else if (ad->phase < 0.78)
    strncpy(ad->phase_name, "Last Quarter", MAXSIZE);
  else
    strncpy(ad->phase_name, "Waning Crescent", MAXSIZE);
}


double degToRad(double degrees)
{
  return degrees * PI / 180.0;
}

double radToDeg(double radians)
{
  return radians * 180.0 / PI;
}

int dayOfYear(struct tm *date)
{
  return date->tm_yday + 1;  // tm_yday is 0-based
}

void calculateSunriseSunsetUTC(double latitude, double longitude, int dayOfYear,
                               double *sunriseUTC, double *sunsetUTC)
{
  double zenith = 90.833; // official zenith for sunrise/sunset
  double D2R = PI / 180.0;
  double R2D = 180.0 / PI;

  // convert longitude to hour value
  double lngHour = longitude / 15.0;

  // approximate time
  double tRise = dayOfYear + ((6.0 - lngHour) / 24.0);
  double tSet  = dayOfYear + ((18.0 - lngHour) / 24.0);

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
  double cosHRise = (cos(D2R * zenith) - (sinDecRise * sin(D2R * latitude))) /
                    (cosDecRise * cos(D2R * latitude));
  double cosHSet = (cos(D2R * zenith) - (sinDecSet * sin(D2R * latitude))) /
                   (cosDecSet * cos(D2R * latitude));

  if (cosHRise > 1 || cosHSet < -1)
  {
    *sunriseUTC = -1;
    *sunsetUTC = -1;
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
  *sunriseUTC = fmod((TRise - lngHour + 24.0), 24.0);
  *sunsetUTC  = fmod((TSet  - lngHour + 24.0), 24.0);
}

void localize_time(double hour, double offset, char * loctime)
{
  char am_pm[3];
  int h = ((int) (hour + offset)) % 24;

  if (h < 12)
    strncpy(am_pm,"AM",sizeof(am_pm));
  else
  {
    strncpy(am_pm,"PM",sizeof(am_pm));
    if (h > 12) h -=12;
  }

  int m = (int)round((hour - floor(hour)) * 60);
  if (m == 60)
  {
    m = 0;
    h = (h + 1) % 24;
  }
  snprintf(loctime,MAXSIZE,"%02d:%02d %s", h, m,am_pm);
}

/***** FIX up */
int lunar_data(astro_info * ad)
{
  double latitude = ad->latitude;
  double longitude = ad->longitude;
  double tz_data = ad->tz_offset;

  // Get current date (UTC)
  time_t now = ad->epoch;
  struct tm *utc = gmtime(&now);
  int day = dayOfYear(utc);


  // Get localized time
  double sunriseUTC = 0.0;
  double sunsetUTC = 0.0;

  calculateSunriseSunsetUTC(latitude, longitude, day, &sunriseUTC, &sunsetUTC);
  if (sunriseUTC < 0 || sunsetUTC < 0)
  {
    strncpy (ad->sunrise_local,"N/A",MAXSIZE);
    strncpy (ad->sunset_local,"N/A",MAXSIZE);
    return 1;
  }
  else
  {
    localize_time(sunriseUTC, tz_data, ad->sunrise_local);
    localize_time(sunsetUTC,  tz_data, ad->sunset_local);
  }

  ad->phase = getMoonPhase(utc);
  getPhaseName(ad);
  ad->lunar_illumination = (1.0 - cos(2 * PI * ad->phase)) / 2.0;
  return 0;
}
