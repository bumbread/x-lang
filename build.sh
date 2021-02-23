#!/bin/sh
export main_full_path=$(readlink -f src/main.c)
gcc $main_full_path -O0 -ggdb -std=c99 -fno-builtin -Wall -Wno-unused-function -Wno-unused-variable -Wno-switch -Wno-char-subscripts
