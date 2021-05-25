# !/bin/bash
gcc -o main main.c smtp.o `pkg-config --cflags --libs gtk+-3.0` -rdynamic -w -lssl -lcrypto -DSMTP_OPENSSL
./main