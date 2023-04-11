
#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>
#include <stdbool.h>

#define INNER_STRINGIZE(s) #s
#define STRINGIZE(s) INNER_STRINGIZE(s)

enum StatusCode {
    Continue = 100,
    SwitchingProtocols = 101,
    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    TemporaryRedirect = 307,
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    InternalServerError = 500,
    NotImplemented = 501,    
};

typedef struct response {
    char header[512];
    char* content;
    size_t sz;
} response_t;

response_t __resp_new(enum StatusCode code, char* code_name);
#define resp_new(code) __resp_new(code, STRINGIZE(code))

void resp_add_hdr(response_t* r, char* hdr, char* val);
void resp_add_content(response_t* r, char* content, size_t content_len);
void resp_set_ctype(response_t* r, char* ext);

#endif
