#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <pthread.h>

#include <zlib.h>

#include "utils/log.h"
#include "utils/time.h"
#include "utils/shitvec.h"
#include "http/response.h"

void scuffed_htmlescape(char* dst, char* src, size_t sz) {
    strcpy(dst, "<pre>\n");
    int offset = 6;
    int i;
    for (i = 0; i < sz; i++) {
        switch (src[i]) {
        case '<':
            strcpy(dst+i+offset, "&lt;");
            offset += 3;
            break;
        case '>':
            strcpy(dst+i+offset, "&gt;");
            offset += 3;
            break;
        case '&':
            strcpy(dst+i+offset, "&amp;");
            offset += 4;
            break;
        default:  
            dst[i+offset] = src[i];
        }
    }
    strcpy(dst+i+offset, "\n</pre>");
}

#define MAX_PATH_SZ 32
void* handle_conn(void* ctxt) {
    int ns = *(int*)ctxt;
    char pathbuf[MAX_PATH_SZ];
    char msgbuf[512] = {0};
    while(true) {
        if (recv(ns, msgbuf, 512, 0) == 0) {
            log_info("Closing connection.");
            return 0x0;
        }
        if (strlen(msgbuf) > 0) {
            sscanf(msgbuf, "GET %s HTTP/1.1", (char*)&pathbuf);
            log_info("Got request for %s", pathbuf);

            int fd = open(pathbuf+1, O_RDONLY);
            if (fd != -1) {                
                response_t r = resp_new(OK);

                struct stat st;
                fstat(fd, &st);
                char* fbuf = malloc(st.st_size);
                size_t i;
                for (i = 0; read(fd, fbuf+(i*4096), 4096) > 0; i++);

                bool escape = false;
                bool compression = true;
                char* ext = strchr(pathbuf+1, '.')+1;
                // TODO clean this up some
                if (strcmp(ext, "html") == 0) {
                    resp_add_hdr(&r, "Content-Type", "text/html");
                } else if (strcmp(ext, "png") == 0) {
                    compression = false;
                    resp_add_hdr(&r, "Content-Type", "image/png");
                } else if (strcmp(ext, "svg") == 0) {
                    resp_add_hdr(&r, "Content-Type", "image/svg+xml");
                } else if (strcmp(ext, "jpeg") == 0) {
                    resp_add_hdr(&r, "Content-Type", "image/jpeg");                    
                } else {
                    resp_add_hdr(&r, "Content-Type", "text/html");
                    escape = true;
                }

                char datebuf[32];
                time_to_str(st.st_mtim.tv_sec, datebuf);
                resp_add_hdr(&r, "Last-Modified", datebuf);
                
                if (escape) {
                    char* efbuf = malloc((size_t)(st.st_size * 2)); // FIXME dubious assumption
                    scuffed_htmlescape(efbuf, fbuf, st.st_size);
                    free(fbuf);
                    resp_add_content(&r, efbuf, strlen(efbuf));
                    free(efbuf);
                } else if (compression) {
                    resp_add_hdr(&r, "Content-Encoding", "deflate");
                    size_t clen = compressBound(st.st_size);
                    char* cbuf = malloc(clen);
                    compress((Bytef*)cbuf, &clen, (Bytef*)fbuf, st.st_size);                    
                    free(fbuf);
                    resp_add_content(&r, cbuf, clen);
                    free(cbuf);
                } else {
                    resp_add_content(&r, fbuf, st.st_size);
                }
                send(ns, r.content, r.sz, 0);
                free(r.content);
            } else {
                response_t r = __resp_new(NotFound, "Fuck off.");
                resp_add_hdr(&r, "Content-Type", "text/html");
                resp_add_content(&r, "Fuck off.", 9);                    
                send(ns, r.content, r.sz, 0);
                free(r.content);
            }         
            log_info("Handled request for %s", pathbuf);
        }
    }
}

int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) die("socket");
    
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(8082);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*)&name, sizeof(name)) < 0) {
        name.sin_port = htons(8081);
        if (bind(s, (struct sockaddr*)&name, sizeof(name)) < 0) {
            die("bind");
        }
    }

    int namelen;
    char pathbuf[MAX_PATH_SZ];
    struct sockaddr_in client;
    char msgbuf[512] = {0};    
    while (true) {
        listen(s, 1);
        int ns = accept(s, (struct sockaddr*)&client, (socklen_t*)&namelen);

        log_info("Handling connection from %s", inet_ntoa(client.sin_addr));
        pthread_t thread;
        pthread_create(&thread, NULL, handle_conn, &ns);
    }
}
