#include <stdio.h>
#include <ofp.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "daily_astro.h"


int get_daily_data(ofpdata * output_data) 
{
      
  // Apply Timezone offset manually
  time_t local_now = time(NULL) + (TIMEZONE * 3600);
  const struct tm *lt = gmtime(&local_now);

  // Basic Date Info (Corrected for 1-indexing)
  int day_of_week  = lt->tm_wday;
  int day_of_month = lt->tm_mday;
  int day_of_year  = lt->tm_yday + 1; // Jan 4th is now 4

  int year = lt->tm_year + 1900;
  int is_leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));

  // Event markers (Approximate Day of Year)
  // Mar Equinox (~80), Jun Solstice (~172), Sep Equinox (~264), Dec Solstice (~355)
  
  int event_days[] = {80, 172, 264, 355};
  
  if (is_leap) { for(int i=0; i<4; i++) event_days[i]++; }

  int found = 0;
  int days_to_event = 0;
  int event_number = 0;
  
  for (int i = 0; i < 4; i++) 
  {
    if (day_of_year < event_days[i]) 
    {
      days_to_event = event_days[i] - day_of_year;
      event_number = i;
      found = 1;
      break;
    }
  }

  // Wrap around to next year's March Equinox
  if (!found) {
      int days_in_year = is_leap ? 366 : 365;
      days_to_event = (days_in_year - day_of_year) + event_days[0];
      
  }

  output_data->version = 1;
  output_data->epoch = time(NULL);
  strncpy(output_data->source, "ASTRO daily", sizeof(output_data->source)-1);
  
  /* Pack the calculated data */
  uint8_t *p = output_data->data;
  memcpy(p, &day_of_week, sizeof(int));
  memcpy(p + sizeof(int), &day_of_month, sizeof(int));
  memcpy(p + 2*sizeof(int), &day_of_year, sizeof(int));
  memcpy(p + 3*sizeof(int) , &days_to_event, sizeof(int));
  memcpy(p + 4*sizeof(int), &event_number,sizeof(int));

  snprintf(output_data->notes,sizeof(output_data->notes),"Day of the week/month/year along with number of days until equinox/solstice of the year.");

  return 0;
}