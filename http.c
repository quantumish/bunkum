#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils/log.h"
#include "utils/time.h"
#include "utils/shitvec.h"
#include "utils/sync.h"
#include "utils/html.h"
#include "utils/compress.h"
#include "utils/profile.h"

#include "http/response.h"
#include "http/request.h"


response_t serve_error(enum StatusCode c) {
    response_t r = resp_new(c);
    resp_add_hdr(&r, "Content-Type", "text/html");

    char errt[256];
    sprintf(errt, "Error %d", c);

    html_t html = html_new();
    html_body_add(&html.body, html_h1_new(errt));
    html_body_add(&html.body, html_p_new("Hey! Don't do that."));
    char* msg = html_render(&html);

    resp_add_content(&r, msg, strlen(msg));
    return r;
}

shitvec_t paths;
hashmap_t path_redirs;

char* get_file_ext(char* filename) {
    char* ext = strtok(filename, ".");
    char* next;
    while ((next = strtok(NULL, "."))) ext = next;
    return ext;
}

response_t serve_file(request_t* req) {
    char path[128+8] = "./public";
    strcat(path, req->path);
    int fd = open(path, O_RDONLY); // TODO handle
    response_t r = resp_new(OK);

    struct stat st;
    fstat(fd, &st);
    char* fbuf = malloc(st.st_size); // TODO what if file larger than memory?
    for (size_t i = 0; read(fd, fbuf+(i*4096), 4096) > 0; i++);


    char* ext = get_file_ext(req->path);

    resp_set_ctype(&r, ext);

    bool ok = true;
    char* mtype = (char*)ext_to_mtype(ext);
    char* hdr;
    if ((hdr = hashmap_get(&req->headers, "Accept"))) {
        ok = false;
        shitvec_t mtypes = hdr_parse_accept(hdr);
        for (int j = 0; j < mtypes.vec_sz; j++) {
            struct req_mimetype* a_mtype = shitvec_get(&mtypes, j);
            // TODO doesn't handle stuff like image/* (is that even allowed?)
            if (strcmp(a_mtype->item, mtype) == 0 || strcmp(a_mtype->item, "*/*") == 0) {
                ok = true;
                break;
            }
        }
    }

    char* buf = fbuf; // buffer to be written
    size_t bufsize = st.st_size;

    if (ok && (hdr = hashmap_get(&req->headers, "Accept-Encoding"))) {
        ok = false;
        shitvec_t mtypes = hdr_parse_accept(hdr); // abuse of this func
        for (int j = 0; j < mtypes.vec_sz; j++) {
            struct req_mimetype* a_mtype = shitvec_get(&mtypes, j);
            if (strcmp(a_mtype->item, "gzip") == 0) {
                resp_add_hdr(&r, "Content-Encoding", "gzip");
                buf = gzip_compress(buf, &bufsize);
                ok = true;
                break;
            } else if (strcmp(a_mtype->item, "deflate") == 0) {
                resp_add_hdr(&r, "Content-Encoding", "deflate");
                buf = zlib_compress(buf, &bufsize);
                ok = true;
                break;
            } else if (strcmp(a_mtype->item, "identity") == 0) {
                ok = true;
                break;
            }
        }
    }
    /* if (!ok) return serve_error(NotAcceptable); */

#ifndef __APPLE__
    char datebuf[64];
    time_to_str(st.st_mtim.tv_sec, datebuf);
    resp_add_hdr(&r, "Last-Modified", datebuf);
#endif

    resp_add_content(&r, buf, bufsize);
    free(buf);
    return r;
}

// TODO actually respect requests
// TODO list dirs

response_t make_response (request_t* req, int pfd) {
    if (req_parse(req) < 0) {
        log_error("Failed to parse incoming request.");
        return serve_error(BadRequest);
    }

    if (hashmap_get(&req->headers, "Profile") != 0x0) {
        log_debug("Got header");
        int profile = true;
        if (ptrace(PTRACE_TRACEME, NULL) < 0) {
            log_error("PTRACE_TRACME failed with err %d", errno);
        }
        if (write(pfd, "profile", 8) != 8) {
            log_error("write err");
        }
        usleep(100);
    }

    log_info("Got request %s %s", method_name(req->method), req->path);

    char* mapped_path = hashmap_get(&path_redirs, req->path);
    if (mapped_path != NULL) {
        strcpy(req->path, mapped_path);
    }

    if (shitvec_check(&paths, req->path, (sv_cmp_t)strcmp) == -1) {
        return serve_error(NotFound);
    }

    log_debug("method = %d", req->method);
    switch (req->method) {
    case GET: return serve_file(req);
    default: return serve_error(MethodNotAllowed);
    }
}

double diff_timespec(const struct timespec *time1, const struct timespec *time0) {
    return (time1->tv_sec - time0->tv_sec)
        + (time1->tv_nsec - time0->tv_nsec) / 1000000000.0;
}

void* handle_conn(int ns, int pfd) {
    char msgbuf[1024] = {0};
    while(true) {
        struct timespec before, after, tdiff;
        if (recv(ns, msgbuf, 1024, 0) == 0) {
            log_info("Closing connection.");
            return 0x0;
        }
        clock_gettime(CLOCK_MONOTONIC, &before);
        if (msgbuf[0] != 0) {
            request_t req = req_new(msgbuf, 1024);

            response_t r = make_response(&req, pfd);
            send(ns, r.content, r.sz, 0);
            free(r.content);
            write(pfd, "stop", 5);
            clock_gettime(CLOCK_MONOTONIC, &after);
            log_info("Handled request in %f sec", diff_timespec(&after, &before));
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

channel_t listen_chan;

void* listen_for_conns(void* ctxt) {
    int s = *(int*)ctxt;
    int namelen;
    struct sockaddr_in client;
    while (true) {
        listen(s, 1);
        int ns = accept(s, (struct sockaddr*)&client, (socklen_t*)&namelen);
        log_info("Handling connection from %s", inet_ntoa(client.sin_addr));
        channel_push(&listen_chan, &ns);
    }
}

struct conn_ctxt {
    pid_t pid;
    int fd;
};

// TODO some sort of DDOS protection idk
int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) die("socket");

    int portnum = 8080;
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(portnum);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    while(bind(s, (struct sockaddr*)&name, sizeof(name)) < 0) {
        portnum += 1;
        name.sin_port = htons(portnum);
    }

    paths = shitvec_new(MAX_PATH_LEN);
    path_redirs = hashmap_new(MAX_PATH_LEN, MAX_PATH_LEN);
    path_redirs.vark = true;
    hashmap_set(&path_redirs, "/", "/index.html");

    list_files_sv(&paths, "public");

    int nlen;
    getsockname(s, (struct sockaddr*)&name, (socklen_t*)&nlen);
    log_debug("Open on port %d.", htons(name.sin_port));

    listen_chan = channel_new(sizeof(int));

    pthread_t lthread;
    pthread_create(&lthread, NULL, listen_for_conns, &s);

    int namelen;
    struct sockaddr_in client;

    shitvec_t conns = shitvec_new(sizeof(struct conn_ctxt));
    while (true) {
        int* ptr = channel_try_recv(&listen_chan);
        if (ptr != NULL) {
            int pfds[2];
            pipe(pfds);
            int retval = fcntl(pfds[0], F_SETFL, fcntl(pfds[0], F_GETFL) | O_NONBLOCK);
            pid_t pid = fork();
            if (pid == 0) {
                handle_conn(*ptr, pfds[1]);
                close(pfds[0]);
                close(pfds[1]);
                exit(0);
            }
            struct conn_ctxt ctxt = {.fd = pfds[0], .pid = pid };
            shitvec_push(&conns, &ctxt);            
        }
        for (int i = 0; i < conns.vec_sz; i++) {
            struct conn_ctxt* ctxt = shitvec_get(&conns, i);
            char msg[8] = {0};
            if (read(ctxt->fd, &msg, 8) != 8 || strcmp(msg, "profile") != 0) continue;
            struct profile_node prof_res = profile_res_new();
            while(true) {
                shitvec_t stack = profile(ctxt->pid);
                profile_proc_stack(&prof_res, &stack);                
                usleep(3);
                if (read(ctxt->fd, &msg, 5) == 5 && strcmp(msg, "stop") == 0) break;
            }
            profile_dump(&prof_res, -1);
        }
    }
}

// meta todos:
// - TODO true dependencyless (no zlib, no pthread)
// - TODO HTML parsing maybe but that makes me want to cry
// - TODO nice ways of dynamically *generating* html
// - TODO dynamically profile requests



// - TODO some kinda config file parsing
