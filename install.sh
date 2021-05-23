# !/bin/bash
gcc -o main main.c `pkg-config --cflags --libs gtk+-3.0` -rdynamic -w
