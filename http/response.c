#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

const char* ext_to_mtype(char* ext) {
    if (ext == NULL) {
        return"text/plain";        
    } else if (strcmp(ext+1, "html") == 0) {
        return"text/html";
    } else if (strcmp(ext+1, "png") == 0) {
        return"image/png";
    } else if (strcmp(ext+1, "svg") == 0) {
        return "image/svg+xml";
    } else if (strcmp(ext+1, "jpeg") == 0) {
        return "image/jpeg";                 
    }

}

void resp_set_ctype(response_t* r, char* ext) {
    if (ext == NULL) {
        resp_add_hdr(r, "Content-Type", "text/plain");        
    } else if (strcmp(ext+1, "html") == 0) {
        resp_add_hdr(r, "Content-Type", "text/html");
    } else if (strcmp(ext+1, "png") == 0) {
        resp_add_hdr(r, "Content-Type", "image/png");
    } else if (strcmp(ext+1, "svg") == 0) {
        resp_add_hdr(r, "Content-Type", "image/svg+xml");
    } else if (strcmp(ext+1, "jpeg") == 0) {
        resp_add_hdr(r, "Content-Type", "image/jpeg");                    
    }
}
