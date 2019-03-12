#!/bin/bash

#if [ $# -lt 1 ]; then
#	echo "Parameters required:"
#	echo "1) Build from scratch? (y/n)"
#	exit
#fi

echo "Build from scratch? (y/n)"
read rebuild

# debug build
mkdir -p build_debug && cd $_
if [ $rebuild == "y" ]; then
	rm -r *
fi
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g" ../src/
make

cd ../

# regular build
mkdir -p build && cd $_
if [ $rebuild == "y" ]; then
	rm -r *
fi
cmake ../src/
make
