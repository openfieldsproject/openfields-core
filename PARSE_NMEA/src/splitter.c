#include <stdint.h>
#include "splitter.h"

void strsplitz(char delim, char * incoming, split_st * data)
{
  uint8_t charnum = 0;
  uint8_t charfplace = 0;
  data->numfields = 0;
  char curr_char = '\0';

  while ((curr_char = incoming[charnum]) != '\0')
  {
    if (curr_char == delim)
    {
      if (data->numfields < MAXFIELDS)
      {
        data->fields[data->numfields][charfplace] = '\0';
        data->numfields++;
      }
      charfplace = 0;
    }
    else
    {
      if (charfplace < MAXSPLIT - 1 && data->numfields < MAXFIELDS)
      {
        data->fields[data->numfields][charfplace] = curr_char;
        charfplace++;
      }
      // else: ignore extra chars to prevent overflow
    }

    charnum++;
  }

  // Null-terminate last field
  if (data->numfields < MAXFIELDS)
    data->fields[data->numfields][charfplace] = '\0';


}
