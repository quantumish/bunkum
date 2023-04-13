#ifndef COMPRESS_H
#define COMPRESS_H

#include <stddef.h>

char* zlib_compress(char* buf, size_t* bufsize);
char* gzip_compress(char* buf, size_t* bufsize);

#endif
