#! /bin/sh --
set -ex

CFLAGS="${*:-}"
nasm-0.98.39 $CFLAGS -O999999999 -w+orphan-labels -f elf -o log_i586.o log_i586.nasm
nasm-0.98.39 $CFLAGS -O999999999 -w+orphan-labels -f bin -o log_i586.bin log_i586.nasm
ndisasm -b 32 log_i586.bin | tail  # For the size.
qq xstatic gcc -m32 -Os -W -Wall -s -o test_log_i586.prog test_log.c log_i586.o
./test_log_i586.prog

: "$0" OK.
