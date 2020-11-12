#!/bin/sh

rmmod chall-example
make clean
make
insmod chall-example.ko
