#!/bin/sh
./cc0 <$1.c >$1.sym
./cc1 <$1.sym >$1.x
./cc2.6809 .symtmp <$1.x >$1.s
