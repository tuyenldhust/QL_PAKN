# !/bin/bash
sudo apt install libgtk-3-dev libssl-dev
gcc -DSMTP_OPENSSL smtp.c -c -o smtp.o