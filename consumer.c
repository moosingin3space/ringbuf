#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include "ringbuf.h"

#define MAX_STRLEN 256
#define NSTRS 8

static int shmid;
static char *shmdata;

void print_usage(const char *name) {
  fprintf(stderr, "usage: %s (shm)\n", name);
}

void sig_handler(int signo) {
  if (signo == SIGINT) {
    printf("exiting...\n");
    // detach from the shm
    if (shmdt(shmdata) < 0) {
      perror("shmdt");
      exit(1);
    }

    // delete the shm
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
      perror("shmctl");
      exit(1);
    }
    exit(0);
  }
}

int main(int argc, char **argv) {
  int err;
  char shm_name[MAX_STRLEN];
  char *text;
  key_t shm_key;
  ringbuf_t ringbuf;

  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    fprintf(stderr, "can't catch sigint\n");
    return 1;
  }
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }
  snprintf(shm_name, MAX_STRLEN, "/tmp/%s", argv[1]);

  // allocate shared memory
  if (shm_name[0] == '\0') {
    print_usage(argv[0]);
    return 1;
  }
  printf("%s\n", shm_name);
  if ((shm_key = ftok(shm_name, 'R')) < 0) {
    perror("ftok");
    return 1;
  }
  if ((shmid = shmget(shm_key,
                      MAX_STRLEN * NSTRS + RINGBUF_HDR_SIZE,
                      0644 | IPC_CREAT)) < 0) {
    perror("shmget");
    return 1;
  }
  if ((shmdata = shmat(shmid, (void*)0, 0)) == NULL) {
    perror("shmat");
    return 1;
  }

  // attach ring buffer
  if ((err = ringbuf_init(&ringbuf,
                          MAX_STRLEN,
                          NSTRS,
                          shmdata)) != Success) {
    fprintf(stderr, "failed to make ringbuf in shm\n");
    return 1;
  }

  ringbuf.hdr->start = 0;
  ringbuf.hdr->end = 0;
  ringbuf.hdr->full = false;

  // pull string from buffer
  while (1) {
    // see if I can fetch something
    if ((err = ringbuf_get(&ringbuf, &text)) != Success) {
      // can I recover
      if (err == Empty) {
        printf("ringbuf is empty, will try again in 1 second\n");
        if (sleep(1) != 0) {
          break;
        }
        continue;
      } else {
        ringbuf_perror("ringbuf_get", err);
        return 1;
      }
    }

    printf("buffer stats: start=%d, end=%d\n",
           ringbuf.hdr->start, ringbuf.hdr->end);
    printf("received `%s` from buffer\n", text);
  }

  return 0;
}
