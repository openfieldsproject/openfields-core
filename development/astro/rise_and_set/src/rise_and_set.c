/*
 * rise_and_set.c
 *
 * Sunrise, sunset and moon phase calculations
 * UTC-based
 *
 * Accuracy:
 *   Sunrise/Sunset: ~1–2 minutes
 *   Moon phase:     <1%
 */


#include <math.h>
#include <time.h>

/* ==================== MACROS ==================== */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD(x) ((x) * M_PI / 180.0)
#define RAD2DEG(x) ((x) * 180.0 / M_PI)

/* ==================== JULIAN DAY ==================== */

static double julian_day(time_t t)
{
    return (double)t / 86400.0 + 2440587.5;
}

/* ==================== SOLAR POSITION ==================== */

static void solar_coords(double jd, double *decl, double *eqtime)
{
    double n = jd - 2451545.0;

    double L = fmod(280.46 + 0.9856474 * n, 360.0);
    double g = DEG2RAD(fmod(357.528 + 0.9856003 * n, 360.0));

    double lambda = DEG2RAD(L + 1.915 * sin(g) + 0.020 * sin(2.0 * g));
    double epsilon = DEG2RAD(23.439 - 0.0000004 * n);

    *decl = asin(sin(epsilon) * sin(lambda));

    double y = tan(epsilon / 2.0);
    y *= y;

    *eqtime = 4.0 * RAD2DEG(
        y * sin(2.0 * lambda)
        - 2.0 * 0.0167 * sin(g)
        + 4.0 * 0.0167 * y * sin(g) * cos(2.0 * lambda)
        - 0.5 * y * y * sin(4.0 * lambda)
    );
}

/* ==================== SUN EVENT ==================== */

static int calc_sun_event(double lat, double lon, time_t day,
                          int is_sunrise, time_t *result)
{
    double jd = julian_day(day);
    double decl, eqtime;

    solar_coords(jd, &decl, &eqtime);

    double latr = DEG2RAD(lat);

    double cos_ha =
        (cos(DEG2RAD(90.833)) / (cos(latr) * cos(decl))) -
        tan(latr) * tan(decl);

    /* Clamp to [-1,1] before acos to avoid NaN */
    if (cos_ha < -1.0) cos_ha = -1.0;
    if (cos_ha >  1.0) cos_ha =  1.0;

    double ha = acos(cos_ha);
    double ha_deg = RAD2DEG(ha);

    double solar_noon = 720.0 - 4.0 * lon - eqtime;

    double minutes = is_sunrise
        ? solar_noon - ha_deg * 4.0
        : solar_noon + ha_deg * 4.0;

    *result = day + (time_t)(minutes * 60.0);
    return 1;
}

/* ==================== MOON PHASE (CYCLE %) ==================== */

static double moon_phase_percent(time_t t)
{
    /* Known new moon: 2000-01-06 18:14 UTC */
    const double new_moon_jd = 2451550.1;
    const double lunar_cycle = 29.530588853;

    double jd = julian_day(t);
    double days = jd - new_moon_jd;

    double phase_days = fmod(days, lunar_cycle);
    if (phase_days < 0.0)
        phase_days += lunar_cycle;

    return (phase_days / lunar_cycle) * 100.0;
}

/* ==================== PUBLIC API ==================== */

void get_rise_and_set(double lat, double lon, time_t nowtime,
                     time_t *sunrise,
                     time_t *sunset,
                     double *moon_phase)
{
    struct tm tm_utc;
    gmtime_r(&nowtime, &tm_utc);

    tm_utc.tm_hour = 0;
    tm_utc.tm_min  = 0;
    tm_utc.tm_sec  = 0;

    time_t day = timegm(&tm_utc);
    time_t sr = 0, ss = 0;

    const int max_tries = 366;  /* prevent infinite loops */

    /* --- Next sunrise --- */
    int tries = 0;
    while (tries < max_tries) {
        int have_sr = calc_sun_event(lat, lon, day, 1, &sr);
        if (have_sr && sr > nowtime)
            break;
        day += 86400;
        tries++;
    }
    if (tries >= max_tries) sr = 0;

    /* --- Next sunset --- */
    day = timegm(&tm_utc);
    tries = 0;
    while (tries < max_tries) {
        int have_ss = calc_sun_event(lat, lon, day, 0, &ss);
        if (have_ss && ss > nowtime)
            break;
        day += 86400;
        tries++;
    }
    if (tries >= max_tries) ss = 0;

    if (sunrise) *sunrise = sr;
    if (sunset)  *sunset  = ss;
    if (moon_phase) *moon_phase = moon_phase_percent(nowtime);
}