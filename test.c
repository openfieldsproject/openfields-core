#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>


typedef struct 
{
  char type[128];
  char data[1024];
  time_t timestamp;
  uint32_t flags; 
  uint32_t crc;
} topic_msg_t;

int main(int argc, char *argv[]) 
{
  int fd = inotify_init1(0);
  if (fd < 0) 
  { 
    perror("inotify_init"); return 1; 
  }

  int wd = inotify_add_watch(fd, argv[1], IN_MODIFY);
  if (wd < 0) 
  { 
    perror("inotify_add_watch"); 
    return 1; 
  }

  printf("Watching %s for updates...\n", argv[1]);

  while (1) 
  {
    char buf[sizeof(struct inotify_event) + 256];
    int length = read(fd, buf, sizeof(buf));
    if (length < 0) 
    { 
      perror("read"); 
      break; 
    }

    topic_msg_t msg;
    FILE *f = fopen(argv[1], "rb");
    if (!f) 
    { 
      perror("fopen"); 
      continue; 
    }

    if (fread(&msg, sizeof(topic_msg_t), 1, f) == 1) 
    {
      printf("[%ld] %s: %s (CRC: %08x)\n", (long)msg.timestamp, msg.type, msg.data, msg.crc);
    }
    
    fclose(f);
  }

  inotify_rm_watch(fd, wd);
  close(fd);
  return 0;
}

