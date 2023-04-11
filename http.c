#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <pthread.h>

#include <zlib.h>

#include "utils/log.h"
#include "utils/time.h"
#include "utils/shitvec.h"
#include "http/response.h"
#include "http/request.h"

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

shitvec_t paths;

response_t serve_file(request_t req) {
    char path[128+8] = "./public";
    strcat(path, req.path);
    int fd = open(path, O_RDONLY); // TODO handle
    response_t r = resp_new(OK);
 
    struct stat st;
    fstat(fd, &st);
    char* fbuf = malloc(st.st_size); // TODO what if file larger than memory?
    for (size_t i = 0; read(fd, fbuf+(i*4096), 4096) > 0; i++);

    char* ext = strchr(req.path, '.'); // NOTE breaks if there's a dir with a dot...
    resp_set_ctype(&r, ext);
                
    char datebuf[32];
    time_to_str(st.st_mtim.tv_sec, datebuf);
    resp_add_hdr(&r, "Last-Modified", datebuf);

    char* buf = fbuf; // buffer to be written
    size_t bufsize = st.st_size;
                
    if (ext == NULL) {
        bufsize = st.st_size * 2;
        buf = malloc(bufsize); // FIXME dubious assumption
        scuffed_htmlescape(buf, fbuf, st.st_size);
        free(fbuf);            
    }
    if (ext == NULL || strcmp(ext, ".png") != 0) {
        resp_add_hdr(&r, "Content-Encoding", "deflate");
        size_t clen = compressBound(st.st_size);
        char* cbuf = malloc(clen);
        compress((Bytef*)cbuf, &clen, (Bytef*)buf, bufsize);
        free(buf);

        buf = cbuf;
        bufsize = clen;
    }
    resp_add_content(&r, buf, bufsize);
    free(buf);
    return r;
}

// TODO actually respect requests
// TODO handle gzip
// TODO list dirs

void* handle_conn(void* ctxt) {
    int ns = *(int*)ctxt;
    char msgbuf[1024] = {0};
    while(true) {
        struct timeval before, after, tdiff;
        if (recv(ns, msgbuf, 1024, 0) == 0) {
            log_info("Closing connection.");
            return 0x0;
        }
        gettimeofday(&before, NULL);
        if (msgbuf[0] != 0) {
            request_t req = req_new(msgbuf, 1024);
            int err = req_parse(&req);
            if (err < 0) log_error("Failed to parse incoming request.");

            log_debug("Req: %s, %s, %f", method_name(req.method), req.path, req.ver);
            
            for (size_t i = 0; i < req.headers.vec_sz; i++) {
                header_line_t hdr = *(header_line_t*)shitvec_get(&req.headers, i);
                log_debug("Header '%s: %s'", hdr.name, hdr.value);
            }
            
            log_info("Got request for %s", req.path);
            
            if (shitvec_check(&paths, req.path, (int(*)(void*,void*))strcmp)) {
                response_t r = serve_file(req);
                send(ns, r.content, r.sz, 0);
                free(r.content);
            } else {
                response_t r = __resp_new(NotFound, "Fuck off.");
                resp_add_hdr(&r, "Content-Type", "text/html");
                resp_add_content(&r, "Fuck off.", 9);                    
                send(ns, r.content, r.sz, 0);
                free(r.content);
            }
            
            gettimeofday(&after, NULL);
            timersub(&after, &before, &tdiff);
            log_info("Handled request for %s in %ld.%06ld sec", req.path, tdiff.tv_sec, tdiff.tv_usec);
            req_free(&req);
        }
    }
}

int list_files_sv(shitvec_t* sv, char* base) {    
    char path[MAX_PATH_LEN] = "/";
    struct dirent* dp;
    DIR* dir = opendir(base);
    if (!dir) return -1;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            strcpy(path+1, base);
            strcat(path, "/");
            strcat(path, dp->d_name);
            shitvec_push(sv, strchr(path+1, '/'));
            list_files_sv(sv, path+1);
        }
    }
    
    closedir(dir);    
    return 0;
}

// TODO some sort of DDOS protection idk
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

    paths = shitvec_new(MAX_PATH_LEN);
    list_files_sv(&paths, "public");
    // TODO handle special non-file paths
    
    log_debug("Open on port 8082.");    
    
    int namelen;
    char pathbuf[MAX_PATH_LEN];
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

// meta todos:
// - TODO some sort of testing framework?
// - TODO true dependencyless (no zlib, no pthread)
// - TODO HTML parsing maybe but that makes me want to cry
