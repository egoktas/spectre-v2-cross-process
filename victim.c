/*
  C socket server example
*/

#include<stdio.h>
#include<string.h>  //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>  //write

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

#include <x86intrin.h>

typedef struct data_obj_t {
  char fill1[64];
  char (*fptr)(struct data_obj_t *obj, int read_offset);
  char fill2[512-64-8];
  char buf[512];
} data_obj_t;

typedef struct {
  data_obj_t *obj1;
  data_obj_t *obj2;
} obj_array_t;

__attribute__((aligned(256)))
char *shared_map = NULL;
__attribute__((aligned(256)))
char *priv_map = NULL;

__attribute__((aligned(256)))
int glob_read_offset = 0;

char func_normal(data_obj_t *obj, int read_offset);
char func_priv(data_obj_t *obj, int read_offset);

__attribute__((aligned(256)))
data_obj_t obj_normal = {.fptr = func_normal};

__attribute__((aligned(256)))
data_obj_t obj_priv = {.fptr = func_priv};

__attribute__((aligned(256)))
obj_array_t objs = {.obj1 = &obj_normal, .obj2 = &obj_priv};

__attribute__((aligned(256)))
char buf1[256];

__attribute__((aligned(256)))
char (*glob_fptr)(data_obj_t *obj, int read_offset) = NULL;

__attribute__((aligned(256)))
char buf2[256];

// access shared memory based on byte in buf
char func_normal(data_obj_t *obj, int read_offset) {
  //printf("func_normal. read_mem=0x%lx, read_offset=%d\n", read_mem, read_offset);
  return shared_map[obj->buf[read_offset] * 512];
}

// access priv memory based on byte in buf
char func_priv(data_obj_t *obj, int read_offset) {
  //printf("func_priv. read_mem=0x%lx, read_offset=%d\n", read_mem, read_offset);
  return priv_map[obj->buf[read_offset] * 512];
}

int process_cmd(char c) {
  int junk;
  int input = c;

  if (c != '0' && c != '1' && c != '+' && c != '-') return 0;

  if (c == '+') {
    //printf("increment pointer\n"); 
    if (glob_read_offset < 31) glob_read_offset++;
    return 0;
  } else if (c == '-') {
    //printf("decrement pointer\n");
    if (glob_read_offset > 0) glob_read_offset--;
    return 0;
  }

  data_obj_t *obj = (data_obj_t*) *( ((unsigned long*)&objs) + (c-'0'));
  glob_fptr = obj->fptr;

  for (int i = 1; i <= 100; i++) {
    input += i;
    junk += input & i;
  }

#if 1
  _mm_mfence();
  _mm_clflush(&glob_fptr);
  _mm_mfence();
  junk ^= glob_fptr(obj, glob_read_offset);
#else
  asm (
    "mfence\n"
    "mov %0, %%rdi\n"
    "mov (%1), %%rsi\n"
    "mfence\n"
    "clflushopt (%2)\n"
    "mfence\n"
    "call *(%2)"
  ::"r"(obj), "r" (&glob_read_offset), "r" (&glob_fptr):"rdi", "rsi");
#endif

  return junk;
}

void prepare_vars() {
  // open shared file
  int fd = open("shared_file", O_RDWR);
  shared_map = mmap((void*)0x12300000UL, 0x20000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  priv_map = mmap((void*)0x43210000UL, 0x20000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

#if 0
  printf("*shared_map = 0x%lx\n", *(unsigned long*) shared_map);
  printf("*priv_map = 0x%lx\n", *(unsigned long*) priv_map);
#endif

  strcpy(obj_normal.buf, "aaaaaa");
  obj_normal.buf[6] = '\0';
  strcpy(obj_priv.buf, "secret");
  obj_priv.buf[6] = '\0';

  printf("obj_normal.buf = %s\n", obj_normal.buf);
  printf("obj_priv.buf = %s\n", obj_priv.buf);
}

int main(int argc , char *argv[])
{
  int socket_desc , client_sock , c , read_size;
  struct sockaddr_in server , client;
  char client_message[2000];
  
  prepare_vars();

  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }
  puts("Socket created");
  
  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8888 );
  
  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
    //print the error message
    perror("bind failed. Error");
    return 1;
  }
  puts("bind done");
  
  //Listen
  listen(socket_desc , 3);
  
  //Accept and incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
  
  //accept connection from an incoming client
  client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
  if (client_sock < 0)
  {
    perror("accept failed");
    return 1;
  }
  puts("Connection accepted");
  
  //Receive a message from client
  while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
  {
    process_cmd(client_message[0]);
    //Send the message back to client
    _mm_mfence();
    write(client_sock , client_message , strlen(client_message));
  }
  
  if(read_size == 0)
  {
    puts("Client disconnected");
    fflush(stdout);
  }
  else if(read_size == -1)
  {
    perror("recv failed");
  }
  
  return 0;
}
