#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../utils/log.h"
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

int cmp_r_mimetype(const void* a, const void* b) {
    struct req_mimetype* aa = (struct req_mimetype*)a;
    struct req_mimetype* bb = (struct req_mimetype*)b;
    if (aa->q > bb->q) return -1;
    else if (aa->q == bb->q) return 0;
    else if (aa->q < bb->q) return 1;
}

// TODO handle parse errors
shitvec_t hdr_parse_accept(char* val) {
    shitvec_t mimetypes = shitvec_new(sizeof(struct req_mimetype));
    char* start = strtok(val, ",");
    char* next;
    do {
        next = strtok(NULL, ",");
        float q = 1;
        char* qptr = NULL; 
        if ((qptr = (char*)memchr(start, ';', next-start))) {
            sscanf(qptr, ";q=%f", &q);
        }
        struct req_mimetype mtype;
        mtype.q = q;
        memset(mtype.item, 0, MAX_MIMETYPE_LEN);
        if (qptr != NULL) strncpy(mtype.item, start, qptr-start);
        else if (next != NULL) strncpy(mtype.item, start, next-start);
        else strcpy(mtype.item, start);
        shitvec_push(&mimetypes, &mtype);
    } while ((start = next));
    shitvec_sort(&mimetypes, cmp_r_mimetype);
    return mimetypes;
}

int req_parse(request_t* req) {
    char method[8] = {0};
    int matched = sscanf(req->buf, "%s %s HTTP/%f\r\n", (char*)method, (char*)req->path, &req->ver);
    if (matched < 3 || matched == EOF) return -1;
    req->method = method_enum((char*)method);
    
    req->headers = shitvec_new(sizeof(header_line_t));
    char* start = memchr(req->buf, '\n', MAX_HEADER_NAME+MAX_HEADER_VALUE)+1;
    while (start+MAX_HEADER_NAME+MAX_HEADER_VALUE < req->buf+req->bufsize) {
        header_line_t hdr = header_line_new();
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

void req_free(request_t* req) {
    shitvec_free(&req->headers);
}

#ifdef TEST
#include <stdlib.h>
#include "../test.h"

static void assert_method_eq(enum http_method a, enum http_method b) {
    if (a != b) {
        printf("(%s != %s)", method_name(a), method_name(b));
        exit(1);
    }
}

void test_method_str_to_enum() {
    assert_method_eq(method_enum("GET"), GET);
    assert_method_eq(method_enum("HEAD"), HEAD);
    assert_method_eq(method_enum("POST"), POST);
    assert_method_eq(method_enum("PUT"), PUT);
    assert_method_eq(method_enum("DELETE"), DELETE);
    assert_method_eq(method_enum("CONNECT"), CONNECT);
    assert_method_eq(method_enum("OPTIONS"), OPTIONS);
    assert_method_eq(method_enum("TRACE"), TRACE);
};
#endif
