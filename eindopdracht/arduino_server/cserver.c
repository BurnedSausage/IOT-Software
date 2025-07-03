#include "cserver.h"
#include "abnf.h"
#include "calculations.h"
#include "cbuffer.h"

/*Create buffers and calc*/
cbuffer* b1 = NULL;
cbuffer* b2 = NULL;
calculations* c1 = NULL;
calculations* c2 = NULL;

// This func is for unittest
//  bool handleRequest(struct stream stream) {
//    struct request request;
//    struct response response;
//    return httpmessage(stream, &request);
//  };

// cppcheck-suppress unusedFunction
struct response handleRequest(struct stream stream) {
  struct request request;
  struct response response;

  /*Init buffers and calc if not done yet*/
  if (b1 == NULL) {
    b1 = cbInit(12, OVERWRITE_IF_FULL);
  }
  if (b2 == NULL) {
    b2 = cbInit(12, OVERWRITE_IF_FULL);
  }
  if (c1 == NULL) {
    c1 = calculationsInit();
  }
  if (c2 == NULL) {
    c2 = calculationsInit();
  }

  httpmessage(stream, &request);

  /*Massive nested switch case to handle all requests
    First switch case handles method
    Nested switch cases handle targets
  */
  /*Handles PUT DELETE POST GET*/
  switch (request.method) {
  case METHOD_PUT:
    /*Handles Config others can be added in future*/
    switch (request.target[0]) {
    case TARGET_CONFIG:
      /*handles mode and cbuffsize other can de added in
       * future*/
      switch (request.target[1]) {
      case TARGET_MODE: {
        if (request.body == BODY_ACTIVE) {
          response.code = CREATED_201_PUT_MODE_ACTIVE;
          return response;
        } else if (request.body == BODY_PASSIVE) {
          response.code = CREATED_201_PUT_MODE_PASSIVE;
          return response;
        }
        break;
      }
      case TARGET_CBUFFSIZE: {
        response.code = CREATED_201_PUT_CBUFFSIZE;
        enum cbmode m1 = b1->mode;
        enum cbmode m2 = b2->mode;
        b1 = cbFree(b1);
        b2 = cbFree(b2);
        b1 = cbInit(request.body_int, m1);
        b2 = cbInit(request.body_int, m2);
        break;
      }
      default:
        response.code = BAD_REQUEST_400;
        break;
      }
      break;
    default:
      response.code = BAD_REQUEST_400;
      break;
    }
    break;
  case METHOD_DELETE:
    /*handles sensors more can be added in future*/
    switch (request.target[0]) {
    case TARGET_SENSORS:;
      /*1 2 and unrecognized are handled*/
      int8_t bSize = 0;
      enum cbmode bMode;
      switch (request.target[1]) {
      case TARGET_1:
        calcReset(c1);
        bSize = b1->size;
        bMode = b1->mode;
        b1 = cbFree(b1);
        b1 = cbInit(bSize, bMode);
        response.code = CREATED_201_DELETE_MEASUREMENTS;
        break;
      case TARGET_2:
        calcReset(c2);
        bSize = b2->size;
        bMode = b2->mode;
        b2 = cbFree(b2);
        b2 = cbInit(bSize, bMode);
        response.code = CREATED_201_DELETE_MEASUREMENTS;
        break;
      case TARGET_UNRECOGNIZED:
        response.code = NOT_FOUND_404;
        break;
      default:
        response.code = BAD_REQUEST_400;
        break;
      }
      break;
    default:
      response.code = BAD_REQUEST_400;
      break;
    }
    break;
  case METHOD_POST:
    /*handles sensors more can be added in future*/
    switch (request.target[0]) {
    case TARGET_SENSORS:
      /*handles 1 2 unrecognized*/
      switch (request.target[1]) {
      case TARGET_1:
        cbAdd(b1, request.body_int);
        calcAddValue(c1, request.body_int);
        response.code = CREATED_201_POST_MEASUREMENT;
        break;
      case TARGET_2:
        cbAdd(b2, request.body_int);
        calcAddValue(c2, request.body_int);
        response.code = CREATED_201_POST_MEASUREMENT;
        break;
      case TARGET_UNRECOGNIZED:
        response.code = NOT_FOUND_404;
        break;
      default:
        response.code = BAD_REQUEST_400;
        break;
      }
      break;
    default:
      response.code = BAD_REQUEST_400;
      break;
    }
    break;
  case METHOD_GET:
    /*handles sensors more can be added in future*/
    switch (request.target[0]) {
    case TARGET_SENSORS:;
      cbuffer* tb;
      calculations* tc;
      /*handles 1 2 unrecognized*/
      switch (request.target[1]) {
      case TARGET_1:
        tb = b1;
        tc = c1;
        break;
      case TARGET_2:
        tb = b2;
        tc = c2;
        break;
      case TARGET_UNRECOGNIZED:
        response.code = NOT_FOUND_404;
        break;
      default:
        response.code = BAD_REQUEST_400;
        break;
      }
      /*handles avg stdev and actual*/
      switch (request.target[2]) {
      case TARGET_AVG:
        if (response.code == NOT_FOUND_404 ||
            response.code == BAD_REQUEST_400) {
          break;
        }
        response.code = OK_200_GET_AVG;
        response.get_avg = runningAvg(tc);
        break;
      case TARGET_STDEV:
        if (response.code == NOT_FOUND_404 ||
            response.code == BAD_REQUEST_400) {
          break;
        }
        response.code = OK_200_GET_STDEV;
        response.get_stdev = deviation(tc);
        break;
      case TARGET_ACTUAL:
        if (response.code == NOT_FOUND_404 ||
            response.code == BAD_REQUEST_400) {
          break;
        }
        response.code = OK_200_GET_ACTUAL;
        response.get_actual = bufferAvg(tb);
        break;
      }
    }
    break;

  case METHOD_UNRECOGNIZED:
    response.code = BAD_REQUEST_400;
    break;
  }
  return response;
}

int bufferFull(int buffer) {
  if (buffer == 0) {
    return cbFull(b1);
  } else if (buffer == 1) {
    return cbFull(b2);
  }
  return -1;
}

int addValues(int sensor1, int sensor2) {
  cbAdd(b1, sensor1);
  return cbAdd(b2, sensor2);
}

int returnBufferMode(int buffer) {
  if (buffer == 0) {
    return cbMode(b1);
  } else if (buffer == 1) {
    return cbMode(b2);
  }
  return -1;
}

bool checkBufferInit() { return b1 != NULL && b2 != NULL; }

bool resetCB() {
  b1 = cbFree(b1);
  b1 = cbInit(12, OVERWRITE_IF_FULL);
  b2 = cbFree(b2);
  b2 = cbInit(12, OVERWRITE_IF_FULL);
  calcReset(c1);
  calcReset(c2);

  return true;
}
