#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../utils/shitvec.h"
#include "request.h"

const char* method_names[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};

const char* method_name(enum http_method m) {
    return method_names[m];
}

const enum http_method method_codes[] = {DELETE, CONNECT, PUT, POST, TRACE, HEAD, OPTIONS, GET};
enum http_method method_enum(char* p) {    
    return method_codes[((*(uint64_t*)p*0x1b8b6e6d) % 0x100000000) >> 28];
}

header_line_t header_line_new() {
    header_line_t hl;
    memset(hl.name, 0, MAX_HEADER_NAME);
    memset(hl.value, 0, MAX_HEADER_VALUE);
    return hl;
}

request_t req_new(char* reqbuf, size_t bufsize) {
    request_t req;
    req.buf = reqbuf;
    req.bufsize = bufsize;
    return req;
}

int req_parse(request_t* req) {
    char method[8] = {0};
    int matched = sscanf(req->buf, "%s %s HTTP/%f\r\n", (char*)method, (char*)req->path, &req->ver);
    if (matched < 3 || matched == EOF) return -1;
    req->method = method_enum((char*)method);
    
    req->headers = shitvec_new(sizeof(header_line_t));
    char* start = memchr(req->buf, '\n', MAX_HEADER_NAME+MAX_HEADER_VALUE)+1;
    while (start+MAX_HEADER_NAME+MAX_HEADER_VALUE < req->buf+req->bufsize) {
        header_line_t hdr;
        int matched = sscanf(start, "%32[^:]: %s", hdr.name, hdr.value);
        if (matched == 0) return 0; // No more headers;        
        else if (matched == 1) {
            if (hdr.name[0] == '\r') return 0; // TODO sketch
            return -1; // Uhh... half a header.
        }
        shitvec_push(&req->headers, &hdr);
        start = memchr(start, '\n', MAX_HEADER_NAME+MAX_HEADER_VALUE)+1;
    }

    return 0;
}
