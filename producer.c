#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/shm.h>
#include <stdbool.h>
#include "ringbuf.h"

#define MAX_STRLEN 256
#define NSTRS 8

void print_usage(const char *name) {
  fprintf(stderr, "usage: %s -n (shm) -t (text)\n", name);
}

int main(int argc, char** argv) {
  int c, option_index, err;
  char shm_name[MAX_STRLEN];
  char text[MAX_STRLEN];
  key_t shm_key;
  int shmid;
  ringbuf_t ringbuf;
  char *shmdata;
  static struct option long_opts[] = {
    {"shm-name", required_argument, 0, 'n'},
    {"text", required_argument, 0, 't'},
  };

  shm_name[0] = text[0] = '\0';

  while ((c = getopt_long(argc, argv, "n:t:",
                          long_opts, &option_index)) != -1) {
    switch(c) {
    case 'n':
      snprintf(shm_name, MAX_STRLEN, "/tmp/%s", optarg);
      break;
    case 't':
      strncpy(text, optarg, MAX_STRLEN);
      break;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  // allocate shared memory
  if (shm_name[0] == '\0') {
    print_usage(argv[0]);
    return 1;
  }
  if ((shm_key = ftok(shm_name, 'R')) < 0) {
    perror("ftok");
    return 1;
  }
  if ((shmid = shmget(shm_key,
                      MAX_STRLEN * NSTRS + RINGBUF_HDR_SIZE,
                      0644)) < 0) {
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

  // push string onto buffer
  if ((err = ringbuf_put(&ringbuf, text)) != Success) {
    ringbuf_perror("ringbuf_put", err);
    return 1;
  }

  printf("buffer stats: start=%d, end=%d\n",
         ringbuf.hdr->start, ringbuf.hdr->end);

  // detach from the shm
  if (shmdt(shmdata) < 0) {
    perror("shmdt");
    return 1;
  }

  return 0;
}
