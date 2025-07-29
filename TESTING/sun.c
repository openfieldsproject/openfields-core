/*
* Testing stage only - credit to openai chatgpt for code, don't really understand all the math just yet
*/

#include <stdio.h>
#include <time.h>
#include <math.h>

// Constants
#define PI 3.14159265358979323846

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
    } else
    {
        *solar_energy_percent = 0.0;
    }
}

int main()
{
    // Brisbane, Australia
    double latitude = -27.466086;
    double longitude = 153.023359;

    // Use current time as UTC epoch
    time_t current_epoch = time(NULL);

    double elevation, azimuth, solar_energy;

    calculate_solar_position(current_epoch, latitude, longitude, &elevation, &azimuth, &solar_energy);

    printf("Solar Elevation: %.2f°\n", elevation);
    printf("Solar Azimuth: %.2f°\n", azimuth);
    printf("Available Solar Energy: %.1f%% (clear-sky estimate)\n", solar_energy);

    return 0;
}
