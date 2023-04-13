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
#include "utils/compress.h"
#include "http/response.h"
#include "http/request.h"


response_t serve_error(enum StatusCode c) {
    response_t r = resp_new(c);
    resp_add_hdr(&r, "Content-Type", "text/html");
    char msg[32] = {0};
    sprintf(msg, "<h1>Error %d</h1>", c);
    resp_add_content(&r, msg, strlen(msg));
    return r;
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

	bool ok = true;
    char* mtype = (char*)ext_to_mtype(ext);
	char* hdr;    
	if ((hdr = hashmap_get(&req.headers, "Accept"))) {
		log_debug("Handling Accept header");
		ok = false;
		shitvec_t mtypes = hdr_parse_accept(hdr);
		for (int j = 0; j < mtypes.vec_sz; j++) {
			struct req_mimetype* a_mtype = shitvec_get(&mtypes, j);
			log_debug("%s", a_mtype->item);
			// TODO doesn't handle stuff like image/* (is that even allowed?)
			if (strcmp(a_mtype->item, mtype) == 0 || strcmp(a_mtype->item, "*/*") == 0) {
				ok = true;
				break;
			}
		}
    }

    char* buf = fbuf; // buffer to be written
    size_t bufsize = st.st_size;
    
    if (ok && (hdr = hashmap_get(&req.headers, "Accept-Encoding"))) {
		log_debug("Handling Accept-Encoding header");
		ok = false;
		shitvec_t mtypes = hdr_parse_accept(hdr); // abuse of this func
		for (int j = 0; j < mtypes.vec_sz; j++) {
			struct req_mimetype* a_mtype = shitvec_get(&mtypes, j);
			log_debug("%s", a_mtype->item);
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
    if (!ok) return serve_error(NotAcceptable);
    
    /* char datebuf[64]; */
    /* time_to_str(st.st_mtim.tv_sec, datebuf); */
    /* resp_add_hdr(&r, "Last-Modified", datebuf); */   
    
    resp_add_content(&r, buf, bufsize);
    free(buf);
    return r;
}

// TODO actually respect requests
// TODO handle gzip
// TODO list dirs

response_t make_response (request_t req) {
    if (req_parse(&req) < 0) {
        log_error("Failed to parse incoming request.");
        return serve_error(BadRequest);                
    }
    /* hashmap_dump(&req.headers); */
            
    log_info("Got request %s %s", method_name(req.method), req.path);

    if (!shitvec_check(&paths, req.path, (sv_cmp_t)strcmp)) {
        return serve_error(NotFound);
    }
    
    switch (req.method) {
    case GET: return serve_file(req); break;
    default: return serve_error(MethodNotAllowed);
    }
    
    return serve_error(InternalServerError); // should be unreachable?
}

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
            
            response_t r = make_response(req);            
            send(ns, r.content, r.sz, 0);
            free(r.content);
            
            gettimeofday(&after, NULL);
            timersub(&after, &before, &tdiff);
            log_info("Handled request in %ld.%06ld sec", req.path, tdiff.tv_sec, tdiff.tv_usec);
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
// - TODO true dependencyless (no zlib, no pthread)
// - TODO HTML parsing maybe but that makes me want to cry
// - TODO nice ways of dynamically *generating* html
// - TODO dynamically profile requests
// - TODO some kinda config file parsing
