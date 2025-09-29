#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  printf("Mi PID=%d, PPID=%d\n", getpid(), getppid());
  printf("Ancestro 0: %d\n", getancestor(0));
  printf("Ancestro 1: %d\n", getancestor(1));
  printf("Ancestro 2: %d\n", getancestor(2));
  printf("Ancestro 99: %d\n", getancestor(99));
  exit(0);
}
