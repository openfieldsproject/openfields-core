#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include "sys_stat.h"


void get_memory_info(sys_info * data)
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


  snprintf(data->mem_total, MAXCHAR-1, "%ld", mem_total / 1024);
  snprintf(data->mem_free, MAXCHAR-1, "%ld", mem_free / 1024);
  snprintf(data->mem_avail, MAXCHAR-1, "%ld", mem_available / 1024);
  snprintf(data->mem_buffers, MAXCHAR-1, "%ld", buffers / 1024);
  snprintf(data->mem_cached, MAXCHAR-1, "%ld", cached / 1024);
  data->ok_pub |= 1<<0;
}

void get_disk_usage(sys_info * data)
{
{
  struct statvfs stat;
  if (statvfs("/", &stat) == 0)
  {
    unsigned long total = stat.f_blocks * stat.f_frsize / 1024 / 1024;
    unsigned long free  = stat.f_bfree  * stat.f_frsize / 1024 / 1024;
    unsigned long used  = total - free;

    snprintf(data->disk_total, MAXCHAR-1, "%ld", total);
    snprintf(data->disk_used, MAXCHAR-1, "%ld", used);
    snprintf(data->disk_free, MAXCHAR-1, "%ld", free);

    data->ok_pub |= 1<<1;
  }
}
}

void get_wifi_signal(sys_info * data)
{
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "awk '/%s/ {print $4}' /proc/net/wireless", IFACE);

  FILE *fp = popen(cmd, "r");
  if (fp)
  {
    char signal[16];
    if (fgets(signal, sizeof(signal), fp) != NULL)
    {  //printf("Wi-Fi Signal (%s): %s dBm\n", IFACE, strtok(signal, "."));
      snprintf(data->wifi_sig_strength, MAXCHAR-1, "%s", strtok(signal, "."));

      data->ok_pub |= 1<<2;
    }
    else printf("Wi-Fi Signal (%s): Not found\n", IFACE);

    pclose(fp);
  }
  else
  {
    printf("Failed to read Wi-Fi signal strength.\n");
  }
}

void get_cpu_load(sys_info * data)
{
  double loadavg[3];
  if (getloadavg(loadavg, 3) != -1)
  {
    //printf("CPU Load Avg (1/5/15 min): %.2f, %.2f, %.2f\n", loadavg[0], loadavg[1], loadavg[2]);
    snprintf(data->cpu_1, MAXCHAR-1, "%.2f", loadavg[0]);
    snprintf(data->cpu_5, MAXCHAR-1, "%.2f", loadavg[1]);
    snprintf(data->cpu_15, MAXCHAR-1, "%.2f", loadavg[2]);

    data->ok_pub |= 1<<3;
  }
}