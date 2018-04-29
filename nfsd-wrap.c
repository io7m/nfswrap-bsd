#include <fcntl.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PROGRAM_NAME "nfsd-wrap"

static int got_signal = 0;

static void
signal_received(int signal)
{
  fprintf(stderr, PROGRAM_NAME ": info: received %d\n", signal);
  got_signal = signal;
}

int
main (int argc, char *argv[])
{
  pid_t wrapper_nfsd_pid = -1;
  int wrapper_status = 0;
  int exit_code = 0;

  if (argc != 2) {
    fprintf(stderr, PROGRAM_NAME ": usage: nfsd.sh\n");
    exit(1);
  }

  char *wrapper_nfsd_name = argv[1];

  wrapper_nfsd_pid = fork();
  if (wrapper_nfsd_pid == -1) {
    exit_code = 1;
    warn(PROGRAM_NAME ": fatal: fork: nfsd: %s", wrapper_nfsd_name);
    goto EXIT_CLEANUP;
  }

  if (wrapper_nfsd_pid == 0) {
    char *nfs_args[2] = { wrapper_nfsd_name, NULL };
    if (execv(nfs_args[0], nfs_args) == -1) {
      err(1, PROGRAM_NAME " (nfsd child): fatal: execv: %s", wrapper_nfsd_name);
    }
  }

  signal(SIGHUP, &signal_received);
  signal(SIGINT, &signal_received);
  signal(SIGTERM, &signal_received);
  signal(SIGUSR1, &signal_received);

  while (1) {
    if (got_signal != 0) {
      got_signal = 0;
      if (kill(wrapper_nfsd_pid, SIGUSR1) == -1) {
        warn(PROGRAM_NAME ": could not signal nfsd (%d)", wrapper_nfsd_pid);
        exit_code = 1;
      }
    }

    fprintf(stderr, PROGRAM_NAME ": info: waiting until nfsd terminates\n");
    int r = waitpid(wrapper_nfsd_pid, &wrapper_status, WEXITED | WNOHANG);
    if (r == wrapper_nfsd_pid) {
      if (WIFEXITED(wrapper_status) || WIFSIGNALED(wrapper_status)) {
        fprintf(stderr, PROGRAM_NAME ": info: nfsd terminated or " PROGRAM_NAME " interrupted\n");
	wrapper_nfsd_pid = -1;
	goto EXIT_CLEANUP;
      }
    }
    sleep(3);
  }

  EXIT_CLEANUP:
  if (wrapper_nfsd_pid != -1) {
    if (kill(wrapper_nfsd_pid, SIGUSR1) == -1) {
      warn(PROGRAM_NAME ": could not signal nfsd (%d)", wrapper_nfsd_pid);
      exit_code = 1;
    }
  }

  return exit_code;
}
