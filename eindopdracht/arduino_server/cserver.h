#ifndef cserver_h
#define cserver_h

#include "stream.h"
#include <stdbool.h>

enum statuscode {
  INTERNAL_SERVER_ERROR_500, // failed to malloc cbuffers
  BAD_REQUEST_400,           // bad request
  NOT_FOUND_404,             // request target not found
  OK_200_GET_AVG,            // get mean
  OK_200_GET_STDEV,          // get standard deviation
  OK_200_GET_ACTUAL,         // empty cbuffer, get its mean
  CREATED_201_PUT_MODE_ACTIVE,    // start reading sensors
  CREATED_201_PUT_MODE_PASSIVE,   // stop reading sensors
  CREATED_201_PUT_CBUFFSIZE,      // send new cbuffer size
  CREATED_201_POST_MEASUREMENT,   // send a measurement
  CREATED_201_DELETE_MEASUREMENTS // clear collected data
};

struct response {
  enum statuscode code;
  union {
    double get_avg;
    double get_stdev;
    double get_actual;
  };
};
// This func is for unittest
// bool handleRequest(struct stream);
struct response handleRequest(struct stream);

int bufferFull(int buffer);
int addValues(int sensor1, int sensor2);
int returnBufferMode(int buffer);
bool checkBufferInit();
bool resetCB();
#endif
