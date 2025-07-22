#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "base.h"
#include "utils.h"
#include "splitter.h"

time_t utc_to_epoch(int year, int month, int day, int hour, int minute, int second)
{
    struct tm t;

    /* Adjust year and month for struct tm */
    t.tm_year = year + 100;     /* struct tm expects years since 1900 */
    t.tm_mon  = month - 1;      /* struct tm expects months in [0, 11] */
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;
    t.tm_isdst = 0;             /* no daylight savings time */

    /* Convert to time_t assuming UTC using timegm (POSIX). Fallback if not available */
#ifdef _GNU_SOURCE
    return timegm(&t);
#else
    /* Portable timegm replacement */
    char *tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
    time_t epoch = mktime(&t);
    if (tz) setenv("TZ", tz, 1); else unsetenv("TZ");
    tzset();
    return epoch;
#endif
}

void parse_gpgga(application_data * app_data)
{
  const char delim = ',';
  split_st nmea;

  strsplitz(delim,app_data->incoming_message,&nmea);

  if (strlen(nmea.fields[9]) > 0)
  {
    double altitude = atof(nmea.fields[9]);
    if (strstr(nmea.fields[10],"F") != NULL)
      altitude /= 3.3;
    ADMQ++;
    strncpy (app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/altitude",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,MAXBUF-1,"%.1lf",altitude);
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  if (strlen(nmea.fields[6]) > 0)
  {
    ADMQ++;
    strncpy (app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/fix_quality",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,MAXBUF-1,"%d",atoi(nmea.fields[6]));
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  if (strlen(nmea.fields[7]) > 0)
  {
    ADMQ++;
    strncpy (app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/number_of_satellites",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,MAXBUF-1,"%d",atoi(nmea.fields[7]));
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  return;
}


void parse_gprmc(application_data * app_data)
{
  const char delim = ',';
  split_st nmea;
  uint32_t utcfulltime = 0;
  uint32_t utcseconds = 0;
  uint32_t utcminutes = 0;
  uint32_t utchours = 0;

  uint32_t utcfulldate = 0;
  uint32_t utcday = 0;
  uint32_t utcmonth = 0;
  uint32_t utcyear = 0;

  double latitude = 999.9;
  double longitude = 0;

  uint8_t perf_epoch = 0;  //determins if we process epoch

  strsplitz(delim,app_data->incoming_message,&nmea);


  //utc clock time
  if(strlen(nmea.fields[1]) > 0)
  {
    perf_epoch ++;
    utcfulltime = (uint32_t) atoi(nmea.fields[1]);
    //printf ("%d\n",utcfulltime);
    utcseconds =  utcfulltime % 100;
    utcminutes = (utcfulltime / 100) % 100;
    utchours =    utcfulltime / 10000;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_hour",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%d",utchours);
    app_data->parsed_data[ADMQ].pub_persist = 0;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_minutes",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%d",utcminutes);
    app_data->parsed_data[ADMQ].pub_persist = 0;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_seconds",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%d",utcseconds);
    app_data->parsed_data[ADMQ].pub_persist = 0;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_time",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%02d:%02d:%02d",utchours,utcminutes,utcseconds);
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  //latitude
  if(strlen(nmea.fields[3]) > 0)
  {
    int tempfl = (atoi(nmea.fields[3])) / 100;
    latitude = (atof(nmea.fields[3]) - (tempfl*100)) / 60.0;
    latitude += tempfl;
  }

  //north or south latitude
  if (strlen(nmea.fields[4]) > 0)
  {
    if (strstr (nmea.fields[4],"S") != NULL) latitude *= -1;
    if (abs((int) latitude) < 200)
    {
      ADMQ++;
      strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/latitude",MAXBUF-1);
      snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%lf",latitude);
      app_data->parsed_data[ADMQ].pub_persist = 0;
    }
  }

  //longitude
  if (strlen(nmea.fields[5]) > 0)
  {
    int tempfl = (atoi(nmea.fields[5])) / 100;
    longitude = (atof(nmea.fields[5]) - (tempfl*100)) / 60.0;
    longitude += tempfl;
  }

  //east or west longitude
  if (strlen(nmea.fields[6]) > 0)
  {
    if (strstr (nmea.fields[6],"W") != NULL) latitude *= -1;
    if (abs((int) longitude) < 200)
    {
      ADMQ++;
      strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/longitude",MAXBUF-1);
      snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%lf",longitude);
      app_data->parsed_data[ADMQ].pub_persist = 0;
    }
  }

  //speed over ground
  if (strlen(nmea.fields[7]) > 0)
  {
    double sog = (atof(nmea.fields[7])) * 1850 / 3600;
    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/sog",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%lf",sog);
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  //course over ground
  if (strlen(nmea.fields[8]) > 0)
  {
    double cog = atof(nmea.fields[8]);
    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/cog",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%lf",cog);
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  //utc clock date
  if (strlen(nmea.fields[9]) > 0)
  {
    perf_epoch ++;
    utcfulldate = (uint32_t) atoi(nmea.fields[9]);
    utcyear = utcfulldate % 100;
    utcmonth = ((utcfulldate - utcyear) / 100) % 100;
    utcday = utcfulldate / 10000;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_day",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%d",utcday);
    app_data->parsed_data[ADMQ].pub_persist = 0;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_month",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%d",utcmonth);
    app_data->parsed_data[ADMQ].pub_persist = 0;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_year",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%d",utcyear);
    app_data->parsed_data[ADMQ].pub_persist = 0;

    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/utc_date",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%02d/%02d/%02d",utcday,utcmonth,utcyear);
    app_data->parsed_data[ADMQ].pub_persist = 0;


  }

  //epoch from utc
  if(perf_epoch == 2)
  {
    time_t epoch = utc_to_epoch((int) utcyear, (int) utcmonth, (int) utcday, (int) utchours, (int) utcminutes, (int) utcseconds);
    ADMQ++;
    strncpy(app_data->parsed_data[ADMQ].pub_topic,"DATA/GPS/epoch",MAXBUF-1);
    snprintf(app_data->parsed_data[ADMQ].pub_payload,15,"%ld",epoch);
    app_data->parsed_data[ADMQ].pub_persist = 0;
  }

  return;
}


void parse_message (application_data * app_data)
{
  ADMQ = -1;
  if (strstr(app_data->incoming_message,"$GPGGA") != NULL) parse_gpgga(app_data);
  else if (strstr(app_data->incoming_message,"$GPRMC") != NULL) parse_gprmc(app_data);
  else return;
}

