#!/bin/bash
gcc -E libs/nuklear/Nuklear.c > python/tmp/NkPreproc.h

sed -i '/^#/d' python/tmp/NkPreproc.h