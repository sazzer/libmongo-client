#! /bin/sh

libtoolize --force
aclocal -I m4 --install
autoheader
automake --foreign --add-missing
autoconf
