#ifndef abnf_h
#define abnf_h

#include "request.h"
#include "stream.h"
#include <stdbool.h>

bool httpmessage(struct stream stream,
                 struct request* request);

#endif