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

// Function to power data lines into individual points.
// Users must provide the output vector.
void split_string(std::string str, std::vector<float>& out);

// Function to break ciphertext lines into their respective bytes.
// Users must provide the output vector.
void split_string_hex(std::string str, std::vector<unsigned char>& out);

// Function to read in the power data file.
// Users must provide the output vector of vectors.
void read_data(std::string path, std::vector< std::vector<float> >& out);

// Function to read in the ciphertext file.
// Users must provide the output vector of vectors.
void read_cipher(std::string path, std::vector< std::vector<unsigned char> >& out);

} //end namespace

#endif
