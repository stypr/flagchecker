#!/bin/sh

rmmod flagchecker
make clean
make
insmod flagchecker.ko
