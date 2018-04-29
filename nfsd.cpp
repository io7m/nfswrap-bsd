#include <unistd.h>
#include <signal.h>
#include <stdio.h>

static int done = 0;

static void
signal_received(int signal)
{
  printf("nfsd: info: received %d\n", signal);
  done = 1;
}

int
main (void)
{
  signal(SIGHUP, &signal_received);
  signal(SIGINT, &signal_received);
  signal(SIGTERM, &signal_received);
  signal(SIGUSR1, &signal_received);

  while (!done) {
    printf("nfsd: (%d): still alive\n", getpid());
    sleep(3);
  }

  printf("nfsd: info: exiting\n");
  return 0;
}
