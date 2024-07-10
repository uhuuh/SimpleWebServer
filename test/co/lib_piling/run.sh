#!/bin/bash

gcc -DRUNTIME -shared -fpic -o mymalloc.so mymalloc.c -ldl
gcc -o test main.c
LD_PRELOAD="./mymalloc.so" ./test

