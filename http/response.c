#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../utils/log.h"
#include "../utils/hashmap.h"
#include "../utils/time.h"
#include "response.h"

response_t __resp_new(enum StatusCode code, char* code_name) {
    response_t resp;
    memset(resp.header, 0, 512);    
    sprintf(resp.header, "HTTP/1.1 %d %s\r\n", code, code_name);

    resp_add_hdr(&resp, "Server", "Bunkum/0.0.1");
    
    char datebuf[32];
    time_to_str(time(0), datebuf);
    resp_add_hdr(&resp, "Date", datebuf);
    
    return resp;
}

void resp_add_hdr(response_t* r, char* hdr, char* val) {
    char header[64];
    sprintf(header, "%s: %s\r\n", hdr, val);
    strcat(r->header, header);
}

void resp_add_content(response_t* r, char* content, size_t content_len) {
    char length[32];
    sprintf(length, "%ld", content_len);
    resp_add_hdr(r, "Content-Length", length);
    
    strcat(r->header, "\r\n");
    size_t header_len = strlen(r->header);    

    r->sz = strlen(r->header)+content_len;
    r->content = malloc(r->sz);
    strcpy(r->content, r->header);
    memcpy(r->content+header_len, content, content_len);
}

const char* exts[] = {"html", "css", "js", "png", "gif", "jpeg", "svg", "ttf", "woff", "woff2",
    "pdf", "csv", "gz", "tar", "zip", "json", NULL}; // TODO sentinel sketchy
const char* mtypes[] = {"text/html", "text/css", "text/javascript", "image/png",
    "image/gif", "image/jpeg", "image/svg+xml", "font/ttf", "font/woff", "font/woff2",
    "application/pdf", "text/csv", "application/gzip",  "application/x-tar", "application/zip",
    "application/json"};

const char* ext_to_mtype(char* ext) {
    if (ext == NULL) return"text/plain";        

    for (size_t i = 0; exts[i] != NULL; i++) {
        if (strcmp(ext, exts[i]) == 0) return mtypes[i];
    }

    log_warn("Giving up on mapping %s extension to a MIME type!", ext);
    return "text/plain";
}

void resp_set_ctype(response_t* r, char* ext) {
    resp_add_hdr(r, "Content-Type", (char*)ext_to_mtype(ext));
}
