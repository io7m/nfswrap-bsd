#include <fcntl.h>
#include <kvm.h>
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

#include <vector>

#define PROGRAM_NAME "nfswrap"

static char error_buffer[_POSIX2_LINE_MAX];

static int
find_nfsd_pids(std::vector<pid_t> &pids)
{
  kvm_t *kvm = NULL;
  struct kinfo_proc *procs = NULL;
  int procs_count = 0;
  int return_code = 1;

  kvm = kvm_openfiles(NULL, "/dev/null", NULL, O_RDONLY, error_buffer);
  if (kvm == NULL) {
    return_code = 0;
    warnx(PROGRAM_NAME ": fatal: kvm_openfiles: %s", error_buffer);
    goto EXIT_CLEANUP;
  }

  procs = kvm_getprocs(kvm, KERN_PROC_PROC, 0, &procs_count);
  if (procs == NULL) {
    return_code = 0;
    warnx(PROGRAM_NAME ": fatal: kvm_getprocs: %s", error_buffer);
    goto EXIT_CLEANUP;
  }

  for (int index = 0; index < procs_count; ++index) {
    pid_t pid = procs[index].ki_pid;
    const char *name = procs[index].ki_comm;
    if (strcmp("nfsd", name) == 0) {
      fprintf(stderr, PROGRAM_NAME ": info: nfsd %d\n", pid);
      pids.push_back(pid);
    }
  }

  EXIT_CLEANUP:
  if (kvm != NULL) {
    kvm_close(kvm);
  }
  return return_code;
}

static void
signal_received(int signal)
{
  fprintf(stderr, PROGRAM_NAME ": info: received %d\n", signal);
}

int
main (int argc, char *argv[])
{
  pid_t wrapper_rpcbind_pid = -1;
  pid_t wrapper_nfsd_pid = -1;
  int wrapper_status = 0;
  int exit_code = 0;
  std::vector<pid_t> nfsd_pids;
  const unsigned int wrapper_wait_time = 5;

  if (argc != 3) {
    fprintf(stderr, PROGRAM_NAME ": usage: rpcbind.sh nfsd.sh\n");
    exit(1);
  }

  char *wrapper_rpcbind_name = argv[1];
  char *wrapper_nfsd_name = argv[2];

  wrapper_rpcbind_pid = fork();
  if (wrapper_rpcbind_pid == -1) {
    exit_code = 1;
    warn(PROGRAM_NAME ": fatal: fork: rpcbind: %s", wrapper_rpcbind_name);
    goto EXIT_CLEANUP;
  }

  if (wrapper_rpcbind_pid == 0) {
    char *rpc_args[2] = { wrapper_rpcbind_name, NULL };
    if (execv(rpc_args[0], rpc_args) == -1) {
      err(1, "nfswrap (rpcbind child): fatal: execv: %s", wrapper_rpcbind_name);
    }
  }

  for (unsigned int index = 0; index < wrapper_wait_time; ++index) {
    fprintf(stderr, PROGRAM_NAME ": info: waiting %d seconds for rpcbind to start\n", wrapper_wait_time - index);
    int r = waitpid(wrapper_rpcbind_pid, &wrapper_status, WNOHANG);
    if (r == 0) {
      sleep(1);
    }
    if (r == -1) {
      exit_code = 1;
      wrapper_rpcbind_pid = -1;
      warn(PROGRAM_NAME ": fatal: rpcbind not running");
      goto EXIT_CLEANUP;
    }
  }

  wrapper_nfsd_pid = fork();
  if (wrapper_nfsd_pid == -1) {
    exit_code = 1;
    warn(PROGRAM_NAME ": fatal: fork: nfsd: %s", wrapper_nfsd_name);
    goto EXIT_CLEANUP;
  }

  if (wrapper_nfsd_pid == 0) {
    char *nfs_args[2] = { wrapper_nfsd_name, NULL };
    if (execv(nfs_args[0], nfs_args) == -1) {
      err(1, "nfswrap (nfsd child): fatal: execv: %s", wrapper_nfsd_name);
    }
  }

  for (unsigned int index = 0; index < wrapper_wait_time; ++index) {
    fprintf(stderr, PROGRAM_NAME ": info: waiting %d seconds for nfsd wrapper to finish\n", wrapper_wait_time - index);
    int r = waitpid(wrapper_rpcbind_pid, &wrapper_status, WNOHANG | WEXITED);
    if (r == 0) {
      sleep(1);
    }
  }

  if (!find_nfsd_pids(nfsd_pids)) {
    exit_code = 1;
    goto EXIT_CLEANUP;
  } 

  if (nfsd_pids.size() == 0) {
    exit_code = 1;
    warnx(PROGRAM_NAME ": error: could not find any nfsd pids - assuming nfsd is broken!");
    goto EXIT_CLEANUP;
  }

  fprintf(stderr, PROGRAM_NAME ": info: waiting until rpcbind terminates\n");
  signal(SIGHUP, &signal_received);
  signal(SIGINT, &signal_received);
  signal(SIGTERM, &signal_received);
  waitpid(wrapper_rpcbind_pid, &wrapper_status, WEXITED);
  fprintf(stderr, PROGRAM_NAME ": info: rpcbind terminated or nfswrap interrupted\n");

  EXIT_CLEANUP:
  if (wrapper_rpcbind_pid != -1) {
    fprintf(stderr, PROGRAM_NAME ": info: cleanup rpcbind: SIGTERM %d\n", wrapper_rpcbind_pid);
    if (kill(wrapper_rpcbind_pid, SIGTERM) == -1) {
      warn(PROGRAM_NAME ": error: could not send SIGTERM to rpcbind (%d)", wrapper_rpcbind_pid);
      exit_code = 1;
    }
  }

  for (unsigned int index = 0; index < nfsd_pids.size(); ++index) {
    pid_t nfsd_pid = nfsd_pids.at(index);

    fprintf(stderr, PROGRAM_NAME ": info: cleanup nfsd: SIGUSR1 %d\n", nfsd_pid);
    if (kill(nfsd_pid, SIGUSR1) == -1) {
      warn(PROGRAM_NAME ": error: could not send SIGUSR1 to nfsd (%d)", nfsd_pid);
      exit_code = 1;
    }
  }

  return exit_code;
}
