#include <unistd.h>
#include <stdio.h>

int
main (void)
{
  while (1) {
    printf("nfsd: (%d): still alive\n", getpid());
    sleep(3);
  }

  return 0;
}
