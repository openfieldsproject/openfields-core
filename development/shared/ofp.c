#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ofp.h"

#define MOD_NAME "libofp : "

//------------------------------------------------------------------
int get_ofptime()
{
  return (int) time(NULL);
}

//------------------------------------------------------------------
int ofptime_quality()
{
/**
* @brief Used by other modules to assess system time quality relative to GPS sync.
*
* This function returns quality of the system time while taking into account
* the last known GPS time stored in the timesync file.  Required at boot time and 
* long periods between syncs to test time drift on systems with no RTC.
*
* timesync module runs via cron job every 10 mins.
*
* @return
* * 0: GPSD not polled yet or timesync file missing/corrupt
* * 1 – 6: Trust score indicating how out-of-sync the system clock is with the last GPS fix
*  
*     - 0 : GPSD not polled or missing/corrupt timesync ofp file
*     - 1 : last polled 1+ days
*     - 2 : last polled 6 hrs - 1 day
*     - 3 : last polled 1 hrs - 6 hrs
*     - 4 : last polled 30-59 mins
*     - 5 : last polled 20-30 mins
*     - 6 : last polled 11-20 mins
*     - system time : if sync within 10 mins 59 seconds
*     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*     - (-1) : means gpsd sync has not occured since boot. (System time should never be trusted)
*     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~     
*
*  if system time is greater than last known time sync then assumed that time is current 
*  and current system epoch is returned. 
* 
*
* @note
* The return value allows other modules to determine time trustworthiness and
* take action (e.g., delay time-sensitive operations) until GPS sync is reliable.
  */

  ofpdata data; 

  FILE *fp = fopen("/opt/ofp/wdata/monitor/timesync.ofp", "rb");

  if (!fp)
  {
    ofplog(MOD_NAME "timesync file is not found.");
    return 0;
  }
  
  if (fread(&data, sizeof(ofpdata), 1, fp) != 1) 
  {
    ofplog(MOD_NAME "timesync file is corrupted.");
    return 0;
  }
  
  fclose(fp);

  time_t current_time = time(NULL);       // Getting the current system time

  if ((current_time < data.epoch))       // Time requires resync after reboot.(***No Processing should occur.***)
  {
    ofplog(MOD_NAME "System restart detected, timesync is required after (re)boot.");
    return -1;                            
  }

  if ((current_time - data.epoch) > (24 * 3600))
    return 1;
 
  if ((current_time - data.epoch) > (6 * 3600))
    return 2;   

  if ((current_time - data.epoch) > 3600)
    return 3;

  if ((current_time - data.epoch) > 1800)
    return 4;

  if ((current_time - data.epoch) > 1200)
    return 5;

  if ((current_time - data.epoch) > 660)
    return 6;

  return (int)current_time;
}


//------------------------------------------------------------------
int get_lastsyncdelta()
{
  ofpdata data; 

  FILE *fp = fopen("/opt/ofp/wdata/monitor/timesync.ofp", "rb");

  if (!fp)
  {
    ofplog(MOD_NAME "get_lastsyncdelta - timesync file is not found.");
    return 0;
  }
  
  if (fread(&data, sizeof(ofpdata), 1, fp) != 1) 
  {
    ofplog(MOD_NAME "get_lastsyncdelta - timesync file is corrupted.");
    return 0;
  }
  
  fclose(fp);

  time_t current_time = time(NULL);       // Getting the current system time

  if ((current_time < data.epoch))       // Time requires resync after reboot.(***No Processing should occur.***)
  {
    ofplog(MOD_NAME "System restart detected, timesync is required after (re)boot.");
    return -1;                            
  }

  return (int) current_time - data.epoch;
}


//------------------------------------------------------------------
int ofplog(const char *entry)
/**
 * @brief Append a message to the OFP log file
 *
 * This function adds the given message to the log file with a local timestamp.
 *
 * @param entry A null-terminated string containing the log message.
 * @return int Returns 1 on success, 0 on failure.
 *
 * @note Automatically adds a newline if missing.
 */
{
  char buf[512];      // Buffer for timestamp + message
  char timestr[64];   // Buffer for formatted time
  FILE *fp = fopen("/opt/ofp/wdata/ofp.log", "a");
  if (!fp)
  {
    perror("ofplog: fopen");
    return 0;
  }

  // Get local time
  time_t t = time(NULL);
  struct tm local_tm;
  localtime_r(&t, &local_tm);  // thread-safe localtime

  // Format time as "YYYY-MM-DD HH:MM:SS"
  if (!strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &local_tm))
  {
    fclose(fp);
    return 0; 
  }

  // Combine timestamp and entry
  if (entry[strlen(entry)-1] == '\n')
    snprintf(buf, sizeof(buf), "[%s] %s", timestr, entry);
  else
    snprintf(buf, sizeof(buf), "[%s] %s\n", timestr, entry);

  if (fputs(buf, fp) == EOF)
  {
    fclose(fp);
    return 0;
  }

  fclose(fp);
  return 1;
}
