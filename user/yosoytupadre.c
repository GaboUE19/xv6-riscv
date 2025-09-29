#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Soy %d, mi padre es %d\n", getpid(), getppid());

  for (int k = 0; k <= 3; k++) {
    printf("getancestor(%d) = %d\n", k, getancestor(k));
  }

  exit(0);
}
