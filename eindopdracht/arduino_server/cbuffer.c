#include "cbuffer.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * initialize a new buffer, in case NULL is returned,
 * buffer init failed
 */
cbuffer* cbInit(int8_t size, enum cbmode mode) {
  /*Init buffer*/

  cbuffer* cbuffer = malloc(sizeof(cbuffer));
  /*check if malloc succeeded*/
  if (!cbuffer) {
    return NULL;
  }
  /*init buffer storage on all zero's*/
  cbuffer->data = malloc(sizeof(cbtype) * size);
  /*Check if malloc succeeded if not free previous malloc to
   * avoid mem leak*/
  if (cbuffer->data == NULL) {
    free(cbuffer);
    return NULL;
  }

  /*Store relevant data*/
  cbuffer->mode = mode;
  cbuffer->size = size;
  cbuffer->start = 0;
  cbuffer->count = 0;

  return cbuffer;
};

/**
 * free the buffer, a new buffer can be created
 * instead.
 *
 * TODO buffer is only freed rn ADD new buffer creation
 * tool; But unit test seems to work so low prio
 *
 * cbFree returns NULL to allow for b = cbFree(b);
 */
cbuffer* cbFree(cbuffer* buffer) {
  if (buffer == NULL) {
    return NULL;
  }
  free(buffer->data);
  free(buffer);
  return NULL;
};

/**
 * check whether data can be read from the buffer
 * Returns count, if count != 0 you know data has been
 * written this is returned for average calc etc
 */
int cbAvailable(cbuffer* buffer) { return buffer->count; };

/**
 * peek the oldest value in the buffer, value
 * remains available for read.
 */
cbtype cbPeek(cbuffer* buffer) {
  /*See if buffer has data*/
  if (cbAvailable(buffer) == 0) {
    return 0;
  }

  return buffer->data[buffer->start];
};

/**
 * read and remove the oldest value in the buffer.
 */
cbtype cbRead(cbuffer* buffer) {
  /*See if buffer has data*/
  if (cbAvailable(buffer) == 0) {
    return 0;
  }
  cbtype value = buffer->data[buffer->start];
  buffer->count--;
  buffer->start = (buffer->start + 1) % buffer->size;

  return value;
};

/**
 * add a new value to the buffer, adding may
 * fail depending on the buffer mode.
 */
int8_t cbAdd(cbuffer* buffer, cbtype value) {
  switch (buffer->mode) {
  case OVERWRITE_IF_FULL:
    /*Check if buffer is full*/
    if (buffer->count >= buffer->size) {
      /*Overwrite add*/
      buffer->data[buffer->start] = value;
      buffer->start =
          (buffer->start + (unsigned char)1) % buffer->size;
    } else {
      /*Normal add*/
      buffer->data[(buffer->start + buffer->count) %
                   buffer->size] = value;
      buffer->count++;
    }
    return 1;
    break;
  case IGNORE_IF_FULL:
    /*Check if buffer is full*/
    if (buffer->count >= buffer->size) {
      return 0;
    }
    /*Add data to buffer*/
    buffer->data[(buffer->start + buffer->count) %
                 buffer->size] = value;
    buffer->count++;
    return 1;
    break;
  }

  return 0;
};

/*Returns average of buffer*/
double bufferAvg(cbuffer* buffer) {
  if (buffer != NULL && buffer->count == 0) {
    return -1;
  }
  cbtype sum = 0;
  if (buffer != NULL) {
    int i = buffer->start;
    int next = buffer->count > 0;
    while (next) {
      sum += buffer->data[i];
      buffer->data[i] = 0;
      i = (i + 1) % buffer->size;
      next = i !=
             (buffer->start + buffer->count) % buffer->size;
    }
    double actual = (float)sum / (float)buffer->count;
    buffer->count = 0;
    return actual;
  }
  return 0;
};

int cbFull(cbuffer* buffer) {
  return (buffer->count >= buffer->size);
}

int cbMode(cbuffer* buffer) { return buffer->mode; }
