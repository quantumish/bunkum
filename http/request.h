#ifndef REQUEST_H
#define REQUEST_H

#include "../utils/shitvec.h"
#include "../utils/hashmap.h"

enum http_method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
};
const char* method_name(enum http_method m);

#define MAX_PATH_LEN 128
#define MAX_HEADER_NAME 32
#define MAX_HEADER_VALUE 512
#define MAX_MIMETYPE_LEN 32

struct req_mimetype {
    float q;
    char item[MAX_MIMETYPE_LEN];
};

/* typedef struct header_line { */
/*     char name[MAX_HEADER_NAME]; */
/*     char value[MAX_HEADER_VALUE]; */
/* } header_line_t; */

shitvec_t hdr_parse_accept(char* val);

typedef struct request {
    char* buf;
    size_t bufsize;
    enum http_method method;
    char path[MAX_PATH_LEN];
    float ver;
    hashmap_t headers;
} request_t;

request_t req_new(char* reqbuf, size_t bufsize);
int req_parse(request_t* req);
void req_free(request_t* req);

#endif
