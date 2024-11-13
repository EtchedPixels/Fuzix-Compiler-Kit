#!/bin/sh
# Temporary until we get our own cpp
#
if [ -e /lib/cpp ] ; then
	/lib/cpp -undef -nostdinc $*
else
	/bin/cpp -undef -nostdinc $*
fi

