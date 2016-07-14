#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


typedef struct ringbuf_hdr_s {
  uint32_t start;
  uint32_t end;
  uint32_t num_elts;
  uint32_t elt_size;
  uint8_t full;
} ringbuf_hdr_t;

#define RINGBUF_HDR_SIZE sizeof(ringbuf_hdr_t)

typedef struct ringbuf_s {
  ringbuf_hdr_t *hdr;
  char *buf;
} ringbuf_t;

enum ringbuf_err {
  Success = 0,
  Empty,
  Full,
  InvalidMem,
};

int ringbuf_init(ringbuf_t *buf,
                 size_t elt_size,
                 size_t size,
                 char *memory);
int ringbuf_put(ringbuf_t *buf,
                const char *elt);
int ringbuf_put_space(ringbuf_t *buf,
                      char **mem);
int ringbuf_get(ringbuf_t *buf,
                char **mem);
void ringbuf_perror(const char *msg, int err);
