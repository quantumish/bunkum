#include <stdlib.h>

#include "zlib.h"

char* zlib_compress(char* buf, size_t bufsize) {
    size_t clen = compressBound(bufsize);
    char* cbuf = malloc(clen);
    compress((Bytef*)cbuf, &clen, (Bytef*)buf, bufsize);
    return cbuf;
}

char* gzip_compress(char* buf, size_t bufsize) {
    size_t clen = compressBound(bufsize);
    char* cbuf = malloc(clen);
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = (uInt)bufsize;
    zs.next_in = (Bytef *)buf;
    zs.avail_out = (uInt)clen;
    zs.next_out = (Bytef *)cbuf;

    // "Add 16 to windowBits to write a simple gzip header and trailer around the compressed data instead of a zlib wrapper"
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    // zs.total_out
    
    return cbuf;
}
