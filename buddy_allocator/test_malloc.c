#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern const char** environ;

int main(int argc, char** argv) {
  const char** e=environ;
  while(*e) {
    printf("%s\n",*e);
    ++e;
  }
  exit(0);
  if (argc<2)
    exit(-1);
  
  int mem_size=atoi(argv[1]);
  int num_blocks=atoi(argv[2]);
  void* cosa;
  for (int i=0; i<num_blocks; ++i) {
    cosa=malloc(mem_size);
    memset(cosa, 0, mem_size);
  }

  int cosa_sullo_stac;
  printf("cosa_meno_cosa = %ul\n", (unsigned long int)((char*)&cosa_sullo_stac-(char*)cosa));
}
