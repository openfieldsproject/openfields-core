#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ofp.h"

//------------------------------------------------------------------
int ofp_read(const char * readfile, ofpdata * ofprdata)
{
  FILE * fp = fopen(readfile, "rb");

  if (!fp)
  {
    ofprdata->flags = 1 << 7;
    return 0;
  }

  if (fread(ofprdata, sizeof(ofpdata), 1, fp) != 1) 
  {
    ofprdata->flags = 1 << 6;
    return 0;
  }

  fclose(fp);
  return 1;
}

int ofp_write(const char * writefile, ofpdata * ofpwdata)
{
  char tempfile[520];
  strncpy (tempfile,writefile,512);
  strncat (tempfile,".tmp\0",6);

  FILE * fp = fopen(tempfile, "wb");

  if (fp) 
  {
      fwrite(ofpwdata, sizeof(ofpdata), 1, fp);
      fclose(fp);
      rename(tempfile, writefile);
      return 1;
  }
  return 0;
}

//------------------------------------------------------------------
time_t get_ofptime()
{
  return time(NULL);
}

//------------------------------------------------------------------
time_t ofptime_quality()
{
  return get_lastsyncdelta("/opt/ofp/wdata/monitor/timesync.ofp"); 
}

//------------------------------------------------------------------
time_t get_lastsyncdelta(const char * filename)
{
  ofpdata data; 

  FILE *fp = fopen(filename, "rb");

  if (!fp)
    return -1;
  
  if (fread(&data, sizeof(ofpdata), 1, fp) != 1) 
    return -2;
  
  fclose(fp);

  time_t current_time = time(NULL);      // Getting the current system time
  if ((current_time - data.epoch) < -10) // Detects reboot while taking into account
  {                                      // faster clock between resyncs.
    return -3;                            
  }

  return current_time - data.epoch;
}


//------------------------------------------------------------------
unsigned int ofplog(const char *entry)
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
