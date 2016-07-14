#include "ringbuf.h"
#include <stdio.h>

int ringbuf_init(ringbuf_t *buf,
                 size_t elt_size,
                 size_t size,
                 char *memory) {
  if (!memory) {
    return InvalidMem;
  }
  buf->hdr = (ringbuf_hdr_t*) memory;

  buf->hdr->elt_size = elt_size;
  buf->hdr->num_elts = size;
  buf->buf = &memory[RINGBUF_HDR_SIZE];

  return Success;
}

int ringbuf_put_space(ringbuf_t *buf,
                      char **mem) {
  size_t elt_idx = 0;
  if (buf->hdr->start == buf->hdr->end && buf->hdr->full) {
    return Full;
  }
  elt_idx = buf->hdr->end * buf->hdr->elt_size;
  *mem = &buf->buf[elt_idx];
  buf->hdr->end = (buf->hdr->end + 1) % buf->hdr->num_elts;
  buf->hdr->full = (buf->hdr->start == buf->hdr->end);
  return Success;
}

int ringbuf_put(ringbuf_t *buf,
                const char *elt) {
  int err = Success;
  char *mem = NULL;
  if ((err = ringbuf_put_space(buf, &mem)) != Success) {
    return err;
  }

  memcpy(mem, elt, buf->hdr->elt_size);

  return Success;
}

int ringbuf_get(ringbuf_t *buf,
                char **mem) {
  size_t elt_idx = 0;
  if (buf->hdr->start == buf->hdr->end && !buf->hdr->full) {
    return Empty;
  }
  elt_idx = buf->hdr->start * buf->hdr->elt_size;
  *mem = &buf->buf[elt_idx];
  buf->hdr->start = (buf->hdr->start + 1) % buf->hdr->num_elts;
  buf->hdr->full = false;
  return Success;
}

void ringbuf_perror(const char *msg, int err) {
  fprintf(stderr, "error: %s: ", msg);
  switch (err) {
  case Success:
    fprintf(stderr, "Success\n");
    break;
  case Empty:
    fprintf(stderr, "Empty\n");
    break;
  case Full:
    fprintf(stderr, "Full\n");
    break;
  case InvalidMem:
    fprintf(stderr, "InvalidMem\n");
    break;
  default:
    fprintf(stderr, "Unknown error code\n");
    break;
  }
}
