#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <unistd.h>

void print_memory_info()
{
  char line[256];
  long mem_total = 0, mem_free = 0, mem_available = 0;
  long buffers = 0, cached = 0;

  FILE *fp = fopen("/proc/meminfo", "r");
  if (!fp)
  {
    perror("Unable to open /proc/meminfo");
    return;
  }

  while (fgets(line, sizeof(line), fp))
  {
    if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) continue;
    if (sscanf(line, "MemFree: %ld kB", &mem_free) == 1) continue;
    if (sscanf(line, "MemAvailable: %ld kB", &mem_available) == 1) continue;
    if (sscanf(line, "Buffers: %ld kB", &buffers) == 1) continue;
    if (sscanf(line, "Cached: %ld kB", &cached) == 1) continue;
  }

  fclose(fp);

  printf("Memory Stats (MB):\n");
  printf("  Total      : %ld MB\n", mem_total / 1024);
  printf("  Free       : %ld MB\n", mem_free / 1024);
  printf("  Available  : %ld MB\n", mem_available / 1024);
  printf("  Buffers    : %ld MB\n", buffers / 1024);
  printf("  Cached     : %ld MB\n", cached / 1024);
}

void print_cpu_load()
{
  double loadavg[3];
  if (getloadavg(loadavg, 3) != -1)
  {
    printf("CPU Load Avg (1/5/15 min): %.2f, %.2f, %.2f\n", loadavg[0], loadavg[1], loadavg[2]);
  }
}

void print_disk_usage(const char *path)
{
  struct statvfs stat;
  if (statvfs(path, &stat) == 0)
  {
    unsigned long total = stat.f_blocks * stat.f_frsize / 1024 / 1024;
    unsigned long free  = stat.f_bfree  * stat.f_frsize / 1024 / 1024;
    unsigned long used  = total - free;

    printf("Disk (%s): Total: %lu MB, Used: %lu MB, Free: %lu MB\n", path, total, used, free);
  }
}

void print_wifi_signal(const char *iface)
{
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "awk '/%s/ {print $4}' /proc/net/wireless", iface);

  FILE *fp = popen(cmd, "r");
  if (fp)
  {
    char signal[16];
    if (fgets(signal, sizeof(signal), fp) != NULL)
      printf("Wi-Fi Signal (%s): %s dBm\n", iface, strtok(signal, "."));

    else printf("Wi-Fi Signal (%s): Not found\n", iface);

    pclose(fp);
  }
  else
  {
    printf("Failed to read Wi-Fi signal strength.\n");
  }
}

int main() {
    print_memory_info();
    print_cpu_load();
    print_disk_usage("/");
    print_wifi_signal("wlan0");  // Change iface if needed

    return 0;
}
