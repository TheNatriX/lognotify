# LOGNOTIFY

LOGNOTIFY is a very lightweight daemon that monitors log files and displays
content when they change.
It uses inotify to watch files and Xlib to communicate with X server
in order to display notifications on screen.


# BUILD

Requirements
============

- Standard libc headers.
- X11 headers.
- Xlib


To Build
========

Just type 'make' or 'make debug' to build LOGNOTIFY.
If you want to clean up shared objects and LOGNOTIFY binary from the current
directory, type 'make clean'


