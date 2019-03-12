#!/bin/bash

if [ $# -lt 1 ]; then
	echo "Parameters required:"
	echo "1) Cipher text file with integer cipher words"
	exit
fi

# first, check for the file
if ! [ -e $1 ]; then
	echo "File $1 does not exist!"
	exit
fi

# second, backup the file
cp $1 $1.orig

# third, run vim in command mode to do int-to-hex transformation
vim -c ':%s/\d\+/\=printf("%02x", submatch(0))/g' -c ':wq' $1
