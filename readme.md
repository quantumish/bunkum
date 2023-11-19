# bunkum

A silly personal webserver with the goal to hand-roll basically whatever I can (only dependencies are pthread, zlib, and libunwind). 

Fun features include 
- **Self-profiling**. If you pass a "Profile" header in any request, `bunkum` will automatically profile itself as it serves the request and report the profile info. This was remarkably painful to implement given weird quirks of the ptrace() API for child threads.
- A **custom testing framework** with **automatic test discovery**. After compiling the code as a dynamic library, It works by parsing the ELF file to extract symbol names and find all of those that start with "test". As there's an enforced test interface, it can then load the dynamic library and run the tests.
- A custom **channel type** for inter-thread communications that is directly implemented with atomic intrinsics 
- A silly **PHF** for rapidly converting HTTP methods to their enum form 
```c
const enum http_method method_codes[] = {DELETE, CONNECT, PUT, POST, TRACE, HEAD, OPTIONS, GET};
enum http_method method_enum(char* p) {
    return method_codes[((*(uint64_t*)p*0x1b8b6e6d) % 0x100000000) >> 28];
}
```
alongside others (custom logging, data structures, html generation utilities, etc.)

