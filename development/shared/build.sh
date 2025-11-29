#!/bin/bash
gcc -fPIC -c ofp.c -o ofp.o
gcc -shared -o libofp.so ofp.o
sudo cp ofp.h /usr/local/include
sudo cp libofp.so /usr/local/lib 
sudo ldconfig
rm *.o *.so