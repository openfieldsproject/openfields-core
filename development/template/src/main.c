#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ofp.h>

// Defines
#define MOD_NAME "|<change module name>|"

/**
 * Main function to <desribe the purpose and functiopn of the module>
 *
*/

int main(void)
{
  ofplog(MOD_NAME " started.");
  if (ofptime_quality() < 0)
  {
    ofplog(MOD_NAME "Time is undetermined, aborting");
    return EXIT_FAILURE;
  }

  ofpdata module_data;
  // continue on processing module

  return EXIT_SUCCESS;
}
