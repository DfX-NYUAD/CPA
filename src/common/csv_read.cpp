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

		tmp = std::string("0x" + std::string(byte, 0, 2));
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

bool csv::read_perm_file(std::string path,
		int steps,
		int steps_start,
		int permutations,
		size_t num_traces,
		std::vector< std::vector< std::vector<unsigned> > >& out
	)
{
	std::ifstream file;
	std::string word;
	int data_pts;
	// s has to be float, to properly calculate data_pts
	float s;
	int perm_index;
	int data_pt_index;
	int step_index;

	file.open(path.c_str(), std::ifstream::in);
	if (!file.is_open()) {
		return false;
	}

	s = steps_start;
	step_index = perm_index = data_pt_index = -1;
	while (!file.eof()) {

		file >> word;

		// a new step begins, comprising a set of permutations
		//
		if (word == "STEP_START") {
			step_index++;
			perm_index = -1;

			data_pts = num_traces * (s / steps);

			std::cout << "Parsing " << std::dec << permutations << " permutations for step " << s << "..." << std::endl;

			// for each step, init a 2D vector capable of holding [permutations][data_pts] unsigned data
			//
			// note that data_pts is different for different steps
			out.push_back(
					std::vector< std::vector<unsigned> > (permutations,
						std::vector<unsigned> (data_pts)
					)
				);

			s++;
		}
		// a new permutation begins
		else if (word == "PERM_START") {
			perm_index++;
			data_pt_index = -1;
		}
		// a regular word, i.e., an indices as part of a permutation
		else {
			data_pt_index++;

			out[step_index][perm_index][data_pt_index] = std::stoi(word);

			// sanity check for index being in proper range
			if ((out[step_index][perm_index][data_pt_index] < 0) || (out[step_index][perm_index][data_pt_index] >= num_traces)) {
				std::cerr<<"\nError: the permutations file " << path << " contains an out-of-range index\n";
				std::cerr<<"The violating index is: " << out[step_index][perm_index][data_pt_index];
				std::cerr<<"; the upper limit for the index is: " << num_traces - 1 << std::endl;
				exit(1);
			}
		}
	}

	file.close();

	return true;
}
