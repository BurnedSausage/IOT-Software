#include "abnf.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool field_line(struct stream, struct request*);
static bool field_name(struct stream, struct request*);
static bool field_value(struct stream, struct request*);
static bool http_token(struct stream, char[]);
static bool http_version(struct stream, struct request*);
static bool message_body(struct stream, struct request*);
static bool method(struct stream, struct request*);
static bool origin_form(struct stream, struct request*);
static bool request_line(struct stream, struct request*);
static bool request_target(struct stream, struct request*);
static bool start_line(struct stream, struct request*);

static bool tchar(struct stream, char[], size_t);
static bool readTchars(struct stream stream, char[]);

static bool peekTokenType(struct stream, enum tokentype);
static bool readTokenType(struct stream, enum tokentype);
static bool readTokenValue(struct stream, enum tokentype,
                           const char*);
// catValue is not static to allow for an informative test
void catValue(const char*, char[], size_t);

bool httpmessage(struct stream stream,
                 struct request* request) {
  initTokenizer();

  // ABNF for http_message is an AND construction, so early
  // exit on error
  // Reset request to all false
  request->success = false;
  request->method = METHOD_UNRECOGNIZED;
  for (int i = 0; i < 3; ++i) {
    request->target[i] = TARGET_UNRECOGNIZED;
  }
  request->field = FIELD_UNRECOGNIZED;
  request->body = BODY_EMPTY;
  request->content_length = 0;
  request->body_int = 0;

  if (!start_line(stream, request)) {
    return false;
  }

  if (!readTokenType(stream, CRLF)) {
    return false;
  }

  while (field_line(stream, request) &&
         readTokenType(stream, CRLF)) {
    ;
  }

  if (!readTokenType(stream, CRLF)) {
    // REad tokentype returns false
    return false;
  }

  if (message_body(stream, request)) {
    ;
  }

  request->success = true;
  return true;
}

static bool field_line(struct stream stream,
                       struct request* request) {
  /* field-name ":" *SP field-value *SP*/

  if (peekTokenType(stream, CRLF)) {
    return false;
  }

  if (!field_name(stream, request)) {
    return false;
  }

  if (!readTokenValue(stream, VCHAR, ":")) {
    return false;
  }

  while (readTokenType(stream, SP)) {
  }

  if (!field_value(stream, request)) {
    return false;
  }

  while (readTokenType(stream, SP)) {
  }

  return true;
}

static bool field_name(struct stream stream,
                       struct request* request) {
  // http_token
  char tok[TOKEN_LENGTH];
  http_token(stream, tok);
  // See if field name = Content-Length if so field is
  // recognized
  if (strncmp(tok, "Content-Length", 14) == 0) {
    request->field = FIELD_CONTENT_LENGTH;
  } else {
    return false;
  }
  return true;
}

static bool field_value(struct stream stream,
                        struct request* request) {
  // VCHAR *( SP / HTAB / VCHAR )
  // Char followed by any amount of SP tabs and chars stop
  // read token, put value as int in request
  char tok[TOKEN_LENGTH];
  if (!http_token(stream, tok)) {
    return false;
  }
  request->content_length = atoi(tok);
  return true;
}

static bool http_token(struct stream stream, char tok[]) {
  // any amount of tchar
  if (readTchars(stream, tok)) {
    return true;
  }
  return false;
}

static bool http_version(struct stream stream,
                         struct request* request) {
  // "HTTP" "/" DIGIT "." DIGIT

  static char tok[TOKEN_LENGTH];
  http_token(stream, tok);
  if (strncmp(tok, "HTTP", 4) != 0) {
    return false;
  }
  if (!readTokenValue(stream, VCHAR, "/")) {
    return false;
  }
  if (readTokenType(stream, DIGIT)) {
  } else {
    return false;
  }
  if (!readTokenValue(stream, VCHAR, ".")) {
    return false;
  }
  if (readTokenType(stream, DIGIT)) {
  } else {
    return false;
  }
  return true;
}

static bool message_body(struct stream stream,
                         struct request* request) {
  // *octet (all the remaining data)
  char tok[TOKEN_LENGTH];
  tok[0] = '\0';

  // Test is token (see if can remove)
  if (!http_token(stream, tok)) {
    return false;
  }

  // Test of body active/passive is edge case, eerste waarde
  if (atoi(tok) == 0) {
    if (strncmp(tok, "passive", 7) == 0) {
      request->body = BODY_PASSIVE;
      return true;
    } else if (strcmp(tok, "active") == 0) {
      request->body = BODY_ACTIVE;
      return true;
    } else {
      request->body = BODY_UNRECOGNIZED;
      return true;
    }
  } else {
    // Sla alles op
    request->body = BODY_INT;
    request->body_int = atoi(tok);
  }

  return true;
}

static bool method(struct stream stream,
                   struct request* request) {
  // HTTP token
  static char tok[TOKEN_LENGTH];
  tok[0] = '\0';
  http_token(stream, tok);
  if (strncmp(tok, "GET", 3) == 0) {
    request->method = METHOD_GET;
    return true;
  } else if (strncmp(tok, "PUT", 3) == 0) {
    request->method = METHOD_PUT;
    return true;
  } else if (strncmp(tok, "DELETE", 6) == 0) {
    request->method = METHOD_DELETE;
    return true;
  } else if (strncmp(tok, "POST", 4) == 0) {
    request->method = METHOD_POST;
    return true;
  } else {
    request->method = METHOD_UNRECOGNIZED;
  }
  return false;
}

static bool request_line(struct stream stream,
                         struct request* request) {
  if (!method(stream, request)) {
    return false;
  }
  if (!readTokenType(stream, SP)) {
    return false;
  }
  if (!request_target(stream, request)) {
    return false;
  }
  if (!readTokenType(stream, SP)) {
    return false;
  }
  if (!http_version(stream, request)) {
    return false;
  }
  return true;
}

static bool request_target(struct stream stream,
                           struct request* request) {
  // Origin form
  return origin_form(stream, request);
}

static bool origin_form(struct stream stream,
                        struct request* request) {
  // origin-form = "/" [ http-token [ origin-form ] ]

  static char tok1[TOKEN_LENGTH];
  tok1[0] = '\0';

  static char tok2[TOKEN_LENGTH];
  tok2[0] = '\0';

  static char tok3[TOKEN_LENGTH];
  tok3[0] = '\0';

  if (!readTokenValue(stream, VCHAR, "/")) {
    return false;
  }
  if (http_token(stream, tok1)) {
    if (strncmp(tok1, "config", 6) == 0) {
      request->target[0] = TARGET_CONFIG;
    } else if (strncmp(tok1, "sensors", 7) == 0) {
      request->target[0] = TARGET_SENSORS;
    } else {
      request->target[0] = TARGET_UNRECOGNIZED;
    }

    if (readTokenValue(stream, VCHAR, "/")) {
      if (http_token(stream, tok2)) {
        if (request->target[0] == TARGET_CONFIG) {
          if (strncmp(tok2, "mode", 4) == 0) {
            request->target[1] = TARGET_MODE;
          } else if (strncmp(tok2, "cbuffsize", 9) == 0) {
            request->target[1] = TARGET_CBUFFSIZE;
          }
        } else if (request->target[0] == TARGET_SENSORS) {
          switch (atoi(tok2)) {
          // case 0:
          //   request->target[0] = TARGET_UNRECOGNIZED;
          //   break;
          case 1:
            request->target[1] = TARGET_1;
            break;
          case 2:
            request->target[1] = TARGET_2;
            break;
          default:
            request->target[1] = TARGET_UNRECOGNIZED;
            break;
          }
        }
        if (readTokenValue(stream, VCHAR, "/")) {
          if (http_token(stream, tok3)) {
            if (strncmp(tok3, "avg", 3) == 0) {
              request->target[2] = TARGET_AVG;
            } else if (strncmp(tok3, "stdev", 3) == 0) {
              request->target[2] = TARGET_STDEV;
            } else if (strncmp(tok3, "actual", 3) == 0) {
              request->target[2] = TARGET_ACTUAL;
            }
          }
        }
      }
    }
  }
  return true;
}

static bool start_line(struct stream stream,
                       struct request* request) {
  // Start line == requestline. If requestline == true
  // return true else return false
  return request_line(stream, request);
}

static bool tchar(struct stream stream, char result[],
                  size_t len) {
  struct token next = peekToken(stream);

  if (next.type == DIGIT) {
    next = readToken(stream);
    catValue(next.value, result, len);
    return true;
  }

  if (next.type == ALPHA) {
    next = readToken(stream);
    catValue(next.value, result, len);
    return true;
  }

  if (next.type == VCHAR) {
    switch (next.value[0]) {
    case '!': // falls through on purpose
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '.':
    case '^':
    case '`':
    case '|':
    case '~':
      next = readToken(stream);
      catValue(next.value, result, len);
      return true;
    }
  }

  return false;
}

static bool readTchars(struct stream stream,
                       char results[]) {
  static bool answer = false;
  char result[TOKEN_LENGTH];
  // strncpy(result, "", 0);
  //  if the type stays the same we keep reading and copying
  while (tchar(stream, results, TOKEN_LENGTH)) {
    answer = true;
  }
  // if it was not the type, we return false
  return answer;
}

static bool readTokenType(struct stream stream,
                          enum tokentype type) {
  bool rtrn;
  rtrn = peekTokenType(stream, type);
  // remove token from stream
  if (rtrn) {
    readToken(stream);
  }
  return rtrn;
}

static bool peekTokenType(struct stream stream,
                          enum tokentype type) {
  struct token next = peekToken(stream);

  if (!hasTokenType(next, type)) {
    return false;
  }
  return true;
}

static bool readTokenValue(struct stream stream,
                           enum tokentype type,
                           const char* value) {
  struct token next = peekToken(stream);

  if (!hasTokenValue(next, type, value)) {
    return false;
  }

  readToken(stream);
  return true;
}

void catValue(const char* value, char result[],
              size_t len) {
  if (len == 0) {
    return;
  }
  strncat(result, value, len - strlen(result) - 1);
}