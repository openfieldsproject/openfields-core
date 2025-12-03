#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ofp.h>   // your header for ofpdata

int main(void)
{
    ofpdata fixdata;

    FILE *fp = fopen("/opt/ofp/wdata/monitor/geo_location.ofp", "rb");
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    size_t n = fread(&fixdata, 1, sizeof(fixdata), fp);
    fclose(fp);

    if (n != sizeof(fixdata)) {
        fprintf(stderr, "Error: read %zu bytes, expected %zu\n", n, sizeof(fixdata));
        return EXIT_FAILURE;
    }

    printf("Version: %u\n", fixdata.version);
    printf("Epoch: %ld\n", fixdata.epoch);
    printf("Source: %s\n", fixdata.source);
    printf("Notes: %s\n", fixdata.notes);

    /* Unpack the 3 doubles from fixdata.data */
    if (sizeof(fixdata.data) >= 3 * sizeof(double)) {
        double lat, lon, alt;
        uint8_t *p = fixdata.data;
        memcpy(&lat, p, sizeof(double));
        memcpy(&lon, p + sizeof(double), sizeof(double));
        memcpy(&alt, p + 2*sizeof(double), sizeof(double));

        printf("Latitude: %.9f\n", lat);
        printf("Longitude: %.9f\n", lon);
        printf("Altitude: %.3f\n", alt);
    } else {
        fprintf(stderr, "Data array too small to unpack GPS coordinates\n");
    }

    return EXIT_SUCCESS;
}
