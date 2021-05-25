# !/bin/bash
sudo apt install libgtk-3-dev libssl-dev
gcc -DSMTP_OPENSSL smtp.c -c -o smtp.o
gcc -o main main.c smtp.o `pkg-config --cflags --libs gtk+-3.0` -rdynamic -w -lssl -lcrypto
./main