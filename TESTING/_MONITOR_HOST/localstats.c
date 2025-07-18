/*
* Testing stage only - credit to openai chatgpt for code
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

// ------------------ Memory ------------------
void get_memory_stats() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("fopen /proc/meminfo");
        return;
    }

    char line[256];
    long mem_total = 0, mem_free = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) continue;
        if (sscanf(line, "MemFree: %ld kB", &mem_free) == 1) break;
    }
    fclose(fp);

    printf("[Memory]\n");
    printf("  Total: %.2f MB\n", mem_total / 1024.0);
    printf("  Free : %.2f MB\n\n", mem_free / 1024.0);
}

// ------------------ Load Avg ------------------
void get_load_average() {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) {
        perror("fopen /proc/loadavg");
        return;
    }

    double one, five, fifteen;
    if (fscanf(fp, "%lf %lf %lf", &one, &five, &fifteen) == 3) {
        printf("[Load Average]\n");
        printf("  1 min : %.2f\n", one);
        printf("  5 min : %.2f\n", five);
        printf("  15 min: %.2f\n\n", fifteen);
    }
    fclose(fp);
}

// ------------------ Uptime ------------------
void get_uptime() {
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) {
        perror("fopen /proc/uptime");
        return;
    }

    double uptime_seconds;
    if (fscanf(fp, "%lf", &uptime_seconds) == 1) {
        int days = uptime_seconds / 86400;
        int hours = ((int)uptime_seconds % 86400) / 3600;
        int minutes = ((int)uptime_seconds % 3600) / 60;
        printf("[Uptime]\n");
        printf("  %d days, %d hours, %d minutes\n\n", days, hours, minutes);
    }
    fclose(fp);
}

// ------------------ Disk ------------------
void get_disk_stats(const char *path) {
    struct statvfs stat;
    if (statvfs(path, &stat) != 0) {
        perror("statvfs");
        return;
    }

    unsigned long total = stat.f_blocks * stat.f_frsize;
    unsigned long free = stat.f_bfree * stat.f_frsize;
    printf("[Disk: %s]\n", path);
    printf("  Total: %.2f GB\n", total / (1024.0 * 1024 * 1024));
    printf("  Free : %.2f GB\n\n", free / (1024.0 * 1024 * 1024));
}

// ------------------ Wi-Fi ------------------
void get_wifi_stats() {
    FILE *fp = fopen("/proc/net/wireless", "r");
    if (!fp) {
        perror("fopen /proc/net/wireless");
        return;
    }

    char line[256];
    int line_num = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        if (line_num <= 2) continue;

        char iface[16];
        float link, level, noise;
        if (sscanf(line, " %[^:]: %*d %f %f %f", iface, &link, &level, &noise) == 4) {
            printf("[Wi-Fi: %s]\n", iface);
            printf("  Link Quality: %.0f\n", link);
            printf("  Signal Level: %.0f dBm\n", level);
            printf("  Noise Level : %.0f dBm\n\n", noise);
            found = 1;
        }
    }

    if (!found) {
        printf("[Wi-Fi]\n  No wireless interfaces found.\n\n");
    }

    fclose(fp);
}

// ------------------ Main ------------------
int main() {
    printf("=== Linux System Stats ===\n\n");
    get_memory_stats();
    get_load_average();
    get_uptime();
    get_disk_stats("/"); // root disk
    get_wifi_stats();

    return 0;
}
