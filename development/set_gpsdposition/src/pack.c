#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ofp.h>

void pack_gps_with_osm_link(ofpdata *fixdata, double lat, double lon, double alt)
{
    if (!fixdata) return;

    memset(fixdata, 0, sizeof(*fixdata));

    fixdata->version = 1;
    fixdata->epoch = time(NULL);
    strncpy(fixdata->source, "GPSD", sizeof(fixdata->source)-1);

    /* Pack the three doubles */
    uint8_t *p = fixdata->data;
    memcpy(p, &lat, sizeof(double));
    memcpy(p + sizeof(double), &lon, sizeof(double));
    memcpy(p + 2*sizeof(double), &alt, sizeof(double));

    /* Create OpenStreetMap URL */
    char url[256];
    snprintf(url, sizeof(url),
             "https://www.openstreetmap.org/?mlat=%.6f&mlon=%.6f#map=18/%.6f/%.6f",
             lat, lon, lat, lon);

    strncpy(fixdata->notes, url, sizeof(fixdata->notes)-1);
    fixdata->notes[sizeof(fixdata->notes)-1] = '\0';
}