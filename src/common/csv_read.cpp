/* Copyright (c) 2013 Tescase
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "csv_read.hpp"


void csv::split_string(std::string str, std::vector<float>& out)
{
	std::string tmp;
	std::istringstream linestream(str);
	float val;
	

	while (!linestream.eof()) {
		linestream >> tmp;

		val = atof(tmp.c_str());
		out.push_back(val);
	}
}

void csv::split_string_hex(std::string str, std::vector<unsigned char>& out)
{
	char byte[3];
	std::string tmp;
	std::istringstream linestream(str);
	float val;

	while (!linestream.eof()) {

		// read 4 - 1 = 3 characters
		//
		// first 2 characters contain the byte, whereas the 3rd is to be checked for space (that applies to files with bytes
		// separated by files)
		linestream.get(byte, 4);

		// in case the end of line isn't reached yet, and in case the 3rd character is not a space, we have to ``unget'' that
		// character as it belongs to the next byte already
		if (!linestream.eof() && byte[2] != ' ')
			linestream.unget();

		// NOTE any re-use of tmp like the commented-out cases below, when being declared outside of this loop, leads to glibc
		// errors: *** glibc detected *** ../../../build/sca: free(): invalid pointer: [...] ***
		//
		//tmp = "test";
		//tmp = byte;
		//tmp = std::string("0x" + std::string(byte, 0, 2));
		//
		// that is, for g++-7 (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5) 
		// 	for the following compile commands:
		// 
		// cmake -DCMAKE_CXX_COMPILER=g++-7 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O2" ../src/
		// 	that is, for CMake Debug builds, from -O2 onward, but not for -O1 or -O0
		// cmake -DCMAKE_CXX_COMPILER=g++-7 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O0" ../src/
		// 	versus, for CMake Release builds, already from -O0 onward
		//
		//	and for the following libraries:
		//
		// linux-vdso.so.1 =>  (0x00007ffdc91db000)
		// libstdc++.so.6 => /usr/lib64/libstdc++.so.6 (0x00007f0d92b7a000)
		// libm.so.6 => /lib64/libm.so.6 (0x00007f0d928f6000)
		// libgomp.so.1 => /usr/lib64/libgomp.so.1 (0x00007f0d926e0000)
		// libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f0d924ca000)
		// libpthread.so.0 => /lib64/libpthread.so.0 (0x00007f0d922ad000)
		// libc.so.6 => /lib64/libc.so.6 (0x00007f0d91f18000)
		// /lib64/ld-linux-x86-64.so.2 (0x000055f99ac0c000)
		// librt.so.1 => /lib64/librt.so.1 (0x00007f0d91d10000)
		//
		// Version information:
		// build/sca:
		// 	libm.so.6 (GLIBC_2.2.5) => /lib64/libm.so.6
		// 	libgcc_s.so.1 (GCC_3.0) => /lib64/libgcc_s.so.1
		// 	ld-linux-x86-64.so.2 (GLIBC_2.3) => /lib64/ld-linux-x86-64.so.2
		// 	libgomp.so.1 (GOMP_4.0) => /usr/lib64/libgomp.so.1
		// 	libgomp.so.1 (OMP_1.0) => /usr/lib64/libgomp.so.1
		// 	libc.so.6 (GLIBC_2.4) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// 	libpthread.so.0 (GLIBC_2.2.5) => /lib64/libpthread.so.0
		// 	libstdc++.so.6 (GLIBCXX_3.4.9) => /usr/lib64/libstdc++.so.6
		// 	libstdc++.so.6 (CXXABI_1.3.3) => /usr/lib64/libstdc++.so.6
		// 	libstdc++.so.6 (CXXABI_1.3) => /usr/lib64/libstdc++.so.6
		// 	libstdc++.so.6 (GLIBCXX_3.4) => /usr/lib64/libstdc++.so.6
		// 	libstdc++.so.6 (GLIBCXX_3.4.11) => /usr/lib64/libstdc++.so.6
		// /usr/lib64/libstdc++.so.6:
		// 	libm.so.6 (GLIBC_2.2.5) => /lib64/libm.so.6
		// 	ld-linux-x86-64.so.2 (GLIBC_2.3) => /lib64/ld-linux-x86-64.so.2
		// 	libgcc_s.so.1 (GCC_4.2.0) => /lib64/libgcc_s.so.1
		// 	libgcc_s.so.1 (GCC_3.3) => /lib64/libgcc_s.so.1
		// 	libgcc_s.so.1 (GCC_3.0) => /lib64/libgcc_s.so.1
		// 	libc.so.6 (GLIBC_2.4) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.3) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.3.2) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// /lib64/libm.so.6:
		// 	libc.so.6 (GLIBC_PRIVATE) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// /usr/lib64/libgomp.so.1:
		// 	librt.so.1 (GLIBC_2.2.5) => /lib64/librt.so.1
		// 	libpthread.so.0 (GLIBC_2.3.4) => /lib64/libpthread.so.0
		// 	libpthread.so.0 (GLIBC_2.2.5) => /lib64/libpthread.so.0
		// 	libc.so.6 (GLIBC_2.6) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.4) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.3) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// /lib64/libgcc_s.so.1:
		// 	libc.so.6 (GLIBC_2.4) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// /lib64/libpthread.so.0:
		// 	ld-linux-x86-64.so.2 (GLIBC_2.3) => /lib64/ld-linux-x86-64.so.2
		// 	ld-linux-x86-64.so.2 (GLIBC_2.2.5) => /lib64/ld-linux-x86-64.so.2
		// 	ld-linux-x86-64.so.2 (GLIBC_PRIVATE) => /lib64/ld-linux-x86-64.so.2
		// 	libc.so.6 (GLIBC_2.3.2) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_PRIVATE) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// /lib64/libc.so.6:
		// 	ld-linux-x86-64.so.2 (GLIBC_PRIVATE) => /lib64/ld-linux-x86-64.so.2
		// 	ld-linux-x86-64.so.2 (GLIBC_2.3) => /lib64/ld-linux-x86-64.so.2
		// /lib64/librt.so.1:
		// 	libpthread.so.0 (GLIBC_2.2.5) => /lib64/libpthread.so.0
		// 	libpthread.so.0 (GLIBC_PRIVATE) => /lib64/libpthread.so.0
		// 	libc.so.6 (GLIBC_2.3.2) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_2.2.5) => /lib64/libc.so.6
		// 	libc.so.6 (GLIBC_PRIVATE) => /lib64/libc.so.6

		std::string tmp = std::string("0x" + std::string(byte, 0, 2));
		//std::cout << tmp << std::endl;

		val = atof(tmp.c_str());
		out.push_back(val);
	}
	//std::cout << std::endl;
}

void csv::read_data(std::string path, std::vector< std::vector<float> >& out)
{
	std::ifstream file;
	std::string line;
	std::vector<float> line_vec;

	file.open(path.c_str(), std::ifstream::in);
	if (!file.is_open())
	{
		std::cerr<<"\nCould not open: "<<path<<"\n\n";
		exit(1);
	}

	
	while(file.good())
	{
		getline(file, line);
		split_string(line, line_vec);
		out.push_back(line_vec);
		line_vec.clear();
	}

	file.close();

	out.pop_back();
}


void csv::read_hex(std::string path, std::vector< std::vector<unsigned char> >& out)
{
	std::ifstream file;
	std::string line;
	std::vector<unsigned char> line_vec;

	file.open(path.c_str(), std::ifstream::in);
	if (!file.is_open())
	{
		std::cerr<<"\nCould not open: "<<path<<"\n\n";
		exit(1);
	}

	
	while(file.good())
	{
		getline(file, line);
		split_string_hex(line, line_vec);
		out.push_back(line_vec);
		line_vec.clear();
	}

	file.close();

	out.pop_back();
}

bool csv::read_perm_file(std::fstream& file,
		int s,
		int data_pts,
		int permutations,
		size_t num_traces,
		std::vector< std::vector<unsigned> >& out
	)
{
	std::string word;
	int perm_index;
	int data_pt_index;
	bool new_line_log = true;

	if (s == 0) {
		std::cout << "Initializing permutations file ...";
		new_line_log = false;
	}
	else if (permutations == 0) {
		std::cout << "Dropping permutations for step " << std::dec << s << "...";
		new_line_log = false;
	}
	else {
		std::cout << "Parsing " << std::dec << permutations << " permutations for step " << s << "...";
	}

	// for each step, init a new 2D vector capable of holding [permutations][data_pts] unsigned data
	//
	// note that data_pts is different for different steps
	out.clear();
	out = std::vector< std::vector<unsigned> > (permutations, std::vector<unsigned> (data_pts));

	perm_index = data_pt_index = -1;

	while (!file.eof()) {

		file >> word;

		// a new step begins, so stop parsing for now
		//
		if (word == "STEP_START") {
			break;
		}
		// a new permutation begins
		else if (word == "PERM_START") {
			perm_index++;
			data_pt_index = -1;
		}
		// a regular word, i.e., an indices as part of a permutation
		else {
			data_pt_index++;

			// ignore permutations which are going beyond the number of permutations required in current call
			if (perm_index >= permutations) {
				continue;
			}
			// ignore data points which are going beyond the number of points required in current call
			if (data_pt_index >= data_pts) {
				continue;
			}

			out[perm_index][data_pt_index] = std::stoi(word);

			// sanity checks
			if ((out[perm_index][data_pt_index] < 0) || (out[perm_index][data_pt_index] >= num_traces)) {
				std::cerr<<"\nError: the permutations file contains an out-of-range index\n";
				std::cerr<<"The violating index is: " << out[perm_index][data_pt_index];
				std::cerr<<"; the upper limit for the index is: " << num_traces - 1 << std::endl;

				exit(1);
			}
		}
	}

	if (!new_line_log) {
		std::cout << " done" << std::endl;
	}
	else {
		std::cout << " done" << std::endl;
		std::cout << std::endl;
	}

	// sanity check on currently finished step
	if (perm_index < (permutations - 1)) {
		std::cerr << "Error: the permutations file does not contain enough permutations for current step " << s << std::endl;
		std::cerr << " perm_index=" << perm_index << "; permutations=" << permutations << std::endl;

		exit(1);
	}

	// sanity check on data_pts
	if (data_pt_index < (data_pts - 1)) {
		std::cerr << "Error: the permutations file does not contain enough data points for current step " << s << std::endl;
		std::cerr << " perm_index=" << perm_index << "; data_pt_index=" << data_pt_index << "; data_pts=" << data_pts << std::endl;

		exit(1);
	}

	return true;
}
