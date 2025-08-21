#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "utils.h"


void log_error(const char *context, const char *msg)
{
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char timestr[20];
  if (t)
  {
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", t);
  }
  else
  {
    snprintf(timestr, sizeof(timestr), "unknown-time");
  }

  fprintf(stderr, "[%s] ERROR in %s: %s (errno=%d: %s)\n",
    timestr,
    context ? context : "unknown",
    msg ? msg : "no message",
    errno,
    strerror(errno));
}

/*
Example Use

FILE *f = fopen("file.txt", "r");
if (!f) {
    log_error("fopen", "Failed to open file");
}


*/