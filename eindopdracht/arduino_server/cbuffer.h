#include <stdbool.h> // ik denk dat deze hier overbodig is
#include <stdint.h>

/**
 * define the variable type to store in the buffer
 */
typedef unsigned long cbtype;

/**
 * buffers come in two variants
 */
enum cbmode { OVERWRITE_IF_FULL = 0, IGNORE_IF_FULL = 1 };

/**
 * buffer data, do not modify in client!
 */
typedef struct {
  cbtype* data;
  enum cbmode mode;
  int8_t size;
  int8_t start;
  int8_t count;
} cbuffer;

/**
 * initialize a new buffer, in case NULL is returned,
 * buffer init failed
 */
cbuffer* cbInit(int8_t size, enum cbmode mode);

/**
 * free the buffer, a new buffer can be created
 * instead.
 *
 * cbFree returns NULL to allow for b = cbFree(b);
 */

cbuffer* cbFree(cbuffer* buffer);
/**
 * check whether data can be read from the buffer
 */

int cbAvailable(cbuffer* buffer);
/**
 * peek the oldest value in the buffer, value
 * remains available for read.
 */

cbtype cbPeek(cbuffer* buffer);
/**
 * read and remove the oldest value in the buffer.
 */

cbtype cbRead(cbuffer* buffer);
/**
 * add a new value to the buffer, adding may
 * fail depending on the buffer mode.
 */
int8_t cbAdd(cbuffer* buffer, cbtype value);

/*Returns average of buffer*/
double bufferAvg(cbuffer* buffer);

int cbFull(cbuffer* buffer);

int cbMode(cbuffer* buffer);