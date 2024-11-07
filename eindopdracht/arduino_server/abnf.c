#include "abnf.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "print.h"

static bool field_line(struct stream);
static bool field_name(struct stream);
static bool field_value(struct stream);
static bool http_token(struct stream);
static bool http_version(struct stream);
static bool message_body(struct stream);
static bool method(struct stream);
static bool origin_form(struct stream);
static bool request_line(struct stream);
static bool request_target(struct stream);
static bool start_line(struct stream);
static bool tchar(struct stream, char[], size_t);
static bool isTchar(struct stream);

static bool readTokenType(struct stream, enum tokentype);
static bool readTokenValue(struct stream, enum tokentype,
                           const char*);
// catValue is not static to allow for an informative test
void catValue(const char*, char[], size_t);

bool http_message(struct stream stream) {
  initTokenizer();

  // ABNF for http_message is an AND construction, so early
  // exit on error

  if (!start_line(stream)) {
    return false;
  }

  if (!readTokenType(stream, CRLF)) {
    return false;
  }

  while (field_line(stream) &&
         readTokenType(stream, CRLF)) {
    ;
  }

  if (!readTokenType(stream, CRLF)) {
    return false;
  }

  if (message_body(stream)) {
    ;
  }

  return true;
}

// remaining ABFN functions are stubs
// ABFN funcs are responssible for reading their own tokens that are linked with their function
// Funcs only return true if what they need to check is true, otherwise they always return false

static bool field_line(struct stream stream) {
  // field-name ":" *SP field-value *SP
  if(field_name(stream)){
    //check for ::
    struct token next = peekToken(stream);
    if(next.value[0] == ":"){
      //read SP
      while(readTokenType(stream, SP)){}
      if(field_value(stream)){
        //read SP
        while(readTokenType(stream, SP)){}
        return true;
      }
    }
  }
  return false;
}

static bool field_name(struct stream stream) {
  //http_token
  return http_token(stream);
}

static bool field_value(struct stream stream) {
  // VCHAR *( SP / HTAB / VCHAR )
  //Char followed by any amount of SP tabs and chars stop when next is a enter
  struct token next = peekToken(stream);
  int i = 0; //this will save field value one day
  if(next.type == VCHAR){
    while(next.type == VCHAR || next.type == SP || next.type == HTAB){
      i++;
      readToken(stream);
      next = peekToken(stream);
    }
  }
  if(i != 0){
    return true;
  }
  return false;
}

static bool http_token(struct stream stream) { //probs gonna need some pointer magic to get the values up to original caller
  //any amount of tchar
  int i = 0; //this will save the token one day
  while(isTchar(stream)){
    i++;
    readToken(stream);
  }
  if(i != 0){
    return true;
  }
  return false;
}

static bool http_version(struct stream stream) {
  //"HTTP" "/" DIGIT "." DIGIT
  return false;
}

static bool message_body(struct stream stream) {
  //*octet
  return false;
}

static bool method(struct stream stream) {
  //HTTP token
  return http_token(stream);
}

static bool request_line(struct stream stream) {
  //Request line =  method SP request-target SP http-version
  if(method(stream)){
    //Check for space
    if(readTokenType(stream, SP)){
      if(request_target(stream)){
        //check for space
        if(readTokenType(stream, SP)){
          //Last check returns true if last check passes
          return http_version(stream);
        } 
      } 
    } 
  }
  return false;
}

static bool request_target(struct stream stream) {
  //Origin form
  return false;
}

static bool origin_form(struct stream stream) {
  //origin-form = "/" [ http-token [ origin-form ] ]

  return false;
}

static bool start_line(struct stream stream) {
  // Start line == requestline. If requestline == true return true else return false
  print(peekToken(stream).value[0]);
  readToken(stream);
  print(peekToken(stream).value[0]);

  return request_line(stream);
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

static bool isTchar(struct stream stream) {
  struct token next = peekToken(stream);

  if (next.type == DIGIT) {
    return true;
  }

  if (next.type == ALPHA) {
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
      return true;
    }
  }
  return false;
}

static bool readTokenType(struct stream stream,
                          enum tokentype type) {
  struct token next = peekToken(stream);

  if (!hasTokenType(next, type)) {
    return false;
  }

  readToken(stream);
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