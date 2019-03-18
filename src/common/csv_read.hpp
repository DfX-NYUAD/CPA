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

//
// This file contains the I/O functions required
// to process input data files
//

#ifndef SCA_CPA_CSV_READ
#define SCA_CPA_CSV_READ

#include <vector>
#include <string>
#include <sstream>

namespace csv
{

// Function to data point lines into their individual data points.
void split_string(std::string str, std::vector<float>& out);

// Function to break hex lines into their respective bytes.
void split_string_hex(std::string str, std::vector<unsigned char>& out);

// Function to read in the power data file.
void read_data(std::string path, std::vector< std::vector<float> >& out);

// Function to read in the hex files (cypher/key)
void read_hex(std::string path, std::vector< std::vector<unsigned char> >& out);

// Function to read in permutations file
bool read_perm_file(std::string path,
		int steps,
		int steps_start,
		int permutations,
		size_t num_traces,
		std::vector< std::vector< std::vector<unsigned> > >& out
	);

} //end namespace

#endif
