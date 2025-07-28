#ifndef __SYS_STAT_H_
#define __SYS_STAT_H_

#define MAXCHAR 32
#define IFACE "wlan0"
#include <stdint.h>

typedef struct
{
  char mem_total[MAXCHAR];
  char mem_free[MAXCHAR];
  char mem_avail[MAXCHAR];
  char mem_buffers[MAXCHAR];
  char mem_cached[MAXCHAR];

  char disk_total[MAXCHAR];
  char disk_used[MAXCHAR];
  char disk_free[MAXCHAR];

  char cpu_1[MAXCHAR];
  char cpu_5[MAXCHAR];
  char cpu_15[MAXCHAR];

  char wifi_sig_strength[MAXCHAR];

  uint8_t ok_pub;

}sys_info;

void get_memory_info(sys_info * data);

void get_disk_usage(sys_info * data);

void get_wifi_signal(sys_info * data);

void get_cpu_load(sys_info * data);



#endif