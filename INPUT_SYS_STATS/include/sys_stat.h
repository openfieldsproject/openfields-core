#ifndef __SYS_STAT_H_
#define __SYS_STAT_H_

#define MAXCHAR 32

typedef struct
{
  char mem_total[MAXCHAR];
  char mem_free[MAXCHAR];
  char mem_avail[MAXCHAR];

  double load_avg[3];

}sys_info;





#endif