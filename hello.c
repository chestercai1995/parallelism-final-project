#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv){
  int size = 32 * 1024;
  char* buffer = malloc(size);
  int i;
  for(i = 0; i < size; i++){
    unsigned char r = (rand() % (255 + 1));
    buffer[i] = r;
  }
 // FILE* fd = fopen("/dev/null", "rw");
 // if(fd == NULL){
 //   printf("fopen failed: %s\n", strerror(errno));
 //   exit(1);
 // }
  sleep(1);
  for(i = 0; i < size; i++){
    unsigned char s = buffer[i];
    //if(fwrite(&s, 1, 1, fd) != 1){
    //  printf("fwrite failed: %s\n", strerror(errno));
    //  exit(1);
    //}
  }
  //fclose(fd);
  free(buffer);
}
