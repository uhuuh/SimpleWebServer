// #ifdef RUNTIME
// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void *malloc(size_t size){
    void *(*mallocp)(size_t size);
    char* error;

    mallocp=dlsym(RTLD_NEXT,"malloc");
    if((error=dlerror()) != NULL){
        fputs(error,stderr);
        exit(1);
    }

    char* ptr = mallocp(size);
    //对一些linux版本，使用printf的话由于printf本身需要使用malloc，来开辟缓存（因为printf使用stdout方式），会导致段错误，因此这里打印通过stderr的方式
    // fprintf(stderr,"malloc(%d)=%p\n",(int)size,ptr);
    fputs("malloc(%d)=%p \n",stderr);
    // printf("malloc(%d)=%p \n",(int)size,ptr);
    return ptr;
}

void free(void* ptr){
    void (*freep)(void*) = NULL;
    char* error;
    if(!ptr){
        return;
    }

    freep=dlsym(RTLD_NEXT,"free");
    if((error=dlerror()) != NULL){
        fputs(error,stderr);
        exit(1);
    }

    freep(ptr);
    fprintf(stderr,"free(%p)\n",ptr);
}


// #endif

