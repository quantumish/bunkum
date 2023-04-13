#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../utils/log.h"
#include "request.h"

const char* method_names[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};

const char* method_name(enum http_method m) {
    return method_names[m];
}

const enum http_method method_codes[] = {DELETE, CONNECT, PUT, POST, TRACE, HEAD, OPTIONS, GET};
enum http_method method_enum(char* p) {
    return method_codes[((*(uint64_t*)p*0x1b8b6e6d) % 0x100000000) >> 28];
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
        if (next != NULL && (qptr = (char*)memchr(start, ';', next-start))) {
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

    req->headers = hashmap_new(MAX_HEADER_NAME, MAX_HEADER_VALUE);
	req->headers.vark = true;
    char* start = memchr(req->buf, '\n', MAX_HEADER_NAME+MAX_HEADER_VALUE)+1;
    while (start+MAX_HEADER_NAME+MAX_HEADER_VALUE < req->buf+req->bufsize) {
		char name[MAX_HEADER_NAME] = {0};
		char value[MAX_HEADER_VALUE] = {0};		
        int matched = sscanf(start, "%32[^:]: %s", name, value);
        if (matched == 0) return 0; // No more headers;
        else if (matched == 1) {
            if (name[0] == '\r') return 0; // TODO sketch
            return -1; // Uhh... half a header.
        }
		hashmap_set(&req->headers, &name, &value);
        start = memchr(start, '\n', MAX_HEADER_NAME+MAX_HEADER_VALUE)+1;
    }
    return 0;
}

void req_free(request_t* req) {
    hashmap_free(&req->headers);
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

void test_hdr_parse_accept() {
    char hdr[MAX_HEADER_VALUE] = "text/html,application/xml;q=0.9,image/webp,*/*;q=0.8";
    shitvec_t mtypes = hdr_parse_accept(hdr);
    assert_size_eq(mtypes.vec_sz, 4);

    struct req_mimetype* mtype = shitvec_get(&mtypes, 0);
    assert_str_eq("text/html", mtype->item);
    assert_float_eq(1.0, mtype->q);

    mtype = shitvec_get(&mtypes, 1);
    assert_str_eq("image/webp", mtype->item);
    assert_float_eq(1.0, mtype->q);

    mtype = shitvec_get(&mtypes, 2);
    assert_str_eq("application/xml", mtype->item);
    assert_float_eq(0.9, mtype->q);

    mtype = shitvec_get(&mtypes, 3);
    assert_str_eq("*/*", mtype->item);
    assert_float_eq(0.8, mtype->q);
}

#endif
