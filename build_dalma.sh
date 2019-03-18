#!/bin/bash

#if [ $# -lt 1 ]; then
#	echo "Parameters required:"
#	echo "1) Build from scratch? (y/n)"
#	exit
#fi

echo "Build from scratch? (y/n)"
read -n 1 rebuild
echo ""

echo "Build debug as well? (y/n)"
read -n 1 debug
echo ""

module load gcc

# debug build
if [ $debug == "y" ]; then
	mkdir -p build_debug && cd $_
	if [ $rebuild == "y" ]; then
		rm -r *
	fi
	cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g" ../src/
	make

	cd ../
fi

# regular build
mkdir -p build && cd $_
if [ $rebuild == "y" ]; then
	rm -r *
fi
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -mavx2 -mfma -march=haswell -funroll-loops" ../src/
make
