#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../utils/hashmap.h"
#include "response.h"

response_t __resp_new(enum StatusCode code, char* code_name) {
    response_t resp;
    memset(resp.header, 0, 512);    
    sprintf(resp.header, "HTTP/1.1 %d %s\r\n", code, code_name);
    return resp;
}

void resp_add_hdr(response_t* r, char* hdr, char* val) {
    char header[64];
    sprintf(header, "%s: %s\r\n", hdr, val);
    strcat(r->header, header);
}

void resp_add_content(response_t* r, char* content, size_t content_len) {
    char length[8];
    sprintf(length, "%ld", content_len);
    resp_add_hdr(r, "Content-Length", length);
    
    strcat(r->header, "\r\n");
    size_t header_len = strlen(r->header);    

    r->sz = strlen(r->header)+content_len;
    r->content = malloc(r->sz);
    strcpy(r->content, r->header);
    memcpy(r->content+header_len, content, content_len);
}
