#include <stdio.h>
#include "ofp.h"

int main()
{
   int a;
   //ofplog("Just a test");
   //a = ofplog("Another");
   printf("%d --- %d\n",ofptime_quality(),get_ofptime());


    
   return 0;
}
