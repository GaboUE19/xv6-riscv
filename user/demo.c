#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void burn(unsigned iters) {
  volatile uint x = 0;
  for (unsigned i = 0; i < iters; i++)
    x = x * 1664525u + 1013904223u;
}

int main(int argc, char **argv) {
  int N = 10;
  if(argc > 1) N = atoi(argv[1]);

  for (int i = 0; i < N; i++) {
    int pid = fork();
    if(pid == 0){
      settickets(50*(i+1));
      burn(40000000);
      printf("Proceso %d terminado (tickets=%d)\n", getpid(), 50*(i+1));
      exit(0);
    }
  }

  for (int k = 0; k < N; k++)
    wait(0);

  exit(0);
}
