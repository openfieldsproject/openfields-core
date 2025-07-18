/*
* Testing stage only - credit to openai chatgpt for code, don't really understand all the math just yet
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

#define PI 3.14159265358979323846

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
    long c = 365.25 * year;
    long e = 30.6 * month;
    double jd = c + e + day - 694039.09;  // jd is total days since known new moon (Dec 31 1999)
    jd /= 29.5305882;                    // divide by lunar cycle (29.53 days)
    double phase = jd - floor(jd);       // get fractional part of the cycle
    return phase;
}

const char* getPhaseName(double phase)
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

void printTime(double hour, const char *label, int offset)
{
    int h = ((int)hour + offset) % 24;
    int m = (int)round((hour - floor(hour)) * 60);
    if (m == 60) {
        m = 0;
        h = (h + 1) % 24;
    }
    printf("%s: %02d:%02d\n", label, h, m);
}

int main()
{
    // Brisbane, Australia
    double latitude = -27.466086;
    double longitude = 153.023359;

    // Get current date (UTC)
    time_t now = time(NULL);
    struct tm *utc = gmtime(&now);
    int day = dayOfYear(utc);

    double sunriseUTC, sunsetUTC;
    calculateSunriseSunsetUTC(latitude, longitude, day, &sunriseUTC, &sunsetUTC);

    if (sunriseUTC < 0 || sunsetUTC < 0) {
        printf("Sun never rises or sets on this location/date.\n");
        return 1;
    }

    // Print UTC
    printTime(sunriseUTC, "Sunrise (UTC)", 0);
    printTime(sunsetUTC,  "Sunset  (UTC)", 0);

    // Print Brisbane time (UTC+10)
    printTime(sunriseUTC, "Sunrise (Brisbane)", 10);
    printTime(sunsetUTC,  "Sunset  (Brisbane)", 10);

    double phase = getMoonPhase(utc);
    printf("Moon phase: %.2f (%s)\n", phase, getPhaseName(phase));

    return 0;
}


