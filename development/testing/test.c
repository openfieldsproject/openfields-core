#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ofp.h>

int main()
{
   //****** LOG FILE TEST */
   //int a;
   //ofplog("Just a test");
   //a = ofplog("Another");
   
   //****** READ/WRITE OFP FILES */
   // ofpdata wtest;
   // ofpdata rtest;

   // wtest.version = 1;
   // wtest.epoch = 654653;
   // strcpy(wtest.source,"hey source");
   // strcpy(wtest.data,"this is data");
   // strcpy(wtest.notes,"here's the notes");
   // wtest.flags = 44;
   // wtest.crc = 55;

   // if ((ofp_write("/opt/ofp/wdata/test.ofp",&wtest)) == 1)
   //   printf("Written success!\n");

   // if ((ofp_read("/opt/ofp/wdata/test.ofp",&rtest)) == 1)
   //   printf ("V: %d\nE: %ld\nS: %s\nD: %s\nN: %s\nF: %d\nC: %d\n",rtest.version,rtest.epoch,rtest.source,rtest.data,rtest.notes,rtest.flags,rtest.crc);
 
   // return 0;

   // ******** TIMESYNC TEST */
   printf("%d --- %d\n",ofptime_quality(),get_ofptime());

}


