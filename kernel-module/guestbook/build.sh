#!/bin/sh

rmmod chall-guestbook
make clean
make
insmod chall-guestbook.ko
