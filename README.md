linuxkb
=======

This library adds support for text and character input for Unified
Remote on Linux. It has been tested with Ubuntu 13.04 and the Linux
(64-bit) version of the Unified Remote Server 3 DP4.

Known issues
============

1) At the moment you can only enter ASCII chars, no UTF8 chars. For
instance, the Euro symbol does not work.

2) Also, the library emulates the keystrokes according to the keyboard
layout "English (US, international with dead keys)" (Ubuntu 13.04),
this is hardcoded into the library. You have to activate exactly this
keyboard layout in the Ubuntu system settings while using the Unified
Remote server for the library to work correctly! Otherwise, the
char-to-key translations will be flawed and many keys will be mixed up
or not working (e.g., typing "z" when you wanted to type "y" etc.).


Installation instructions
=========================

Get the files linuxkb.c and CoreInput.remote from GitHub:
https://github.com/einheitlix/linuxkb

First you need to install some development headers for Lua.

$ sudo apt-get install liblua5.1-dev

Now compile the small library linuxkb.c like so:

$ gcc -c -Wall -Werror -fpic linuxkb.c
$ gcc -shared -o liblinuxkb.so linuxkb.o  

This will create a file liblinuxkb.so that you can copy to a directory
of your liking. For the sake of example, let's copy it to
/usr/local/lib/ (you don't need to use a system directory).

$ sudo cp liblinuxkb.so /usr/local/lib/

Next we need to tell the server application that it should use our
library. Overwrite the Lua script remotes/Core/CoreInput.remote inside
your server directory with the modified CoreInput.remote that is
bundled with linuxkb.c. For the sake of example let's assume you have
your remotes installed in
/opt/Server-DP-4-0-0-137-linux-amd64-Debug/remotes/. Then do:

$ sudo cp CoreInput.remote /opt/Server-DP-4-0-0-137-linux-amd64-Debug/remotes/Core/CoreInput.remote 

Here's a diff:
$ diff CoreInput.remote.orig CoreInput.remote
11a12,13
> local linuxkb = package.loadlib("/usr/local/lib/liblinuxkb.so", "luaopen_linuxkb")();
> 
24c26
<       keyboard.press(unpack({...}));
---
>       linuxkb.press(unpack({...}));
34c36
<       keyboard.text(text);
---
>       linuxkb.text(text);

Obviously if you have your liblinuxkb.so elsewhere than in
/usr/local/lib/, you must edit the file CoreInput.remote accordingly
to reflect that location. Otherwise the library will not be found.

Lastly, start the server:
$ sudo /opt/Server-DP-4-0-0-137-linux-amd64-Debug/engine 

That should do it! Try to see if your Android keyboard can now send
text to be remotely typed on your PC.
