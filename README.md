linuxkb
=======

This library adds support for text and character input for Unified
Remote on Linux. It has been tested with Ubuntu 13.04 and the Linux
(64-bit) version of the Unified Remote Server 3 DP4.

Installation instructions
=========================

install dev headers for lua
$ sudo apt-get install liblua5.1-dev

compile lib:
$ gcc -c -Wall -Werror -fpic linuxkb.c
$ gcc -shared -o liblinuxkb.so linuxkb.o  

copy it to dir of your choice:
$ sudo cp liblinuxkb.so /usr/local/lib/

copy remote
$ sudo cp CoreInput.remote /opt/Server-DP-4-0-0-137-linux-amd64-Debug/remotes/Core/CoreInput.remote 

start server
$ sudo /opt/Server-DP-4-0-0-137-linux-amd64-Debug/engine 

IMPORTANT NOTE: At the moment, the keyboard layout "English (US,
international with dead keys)" (Ubuntu 13.04) is hardcoded into the
library. You have to activate exaclty this keyboard layout in Ubuntu
while using the Unified Remote server for the library to work
correctly! Otherwise, the char-to-key translations will not work
correctly and many keys will be mixed up or not working. I'll look
into that when I have time. :-)



