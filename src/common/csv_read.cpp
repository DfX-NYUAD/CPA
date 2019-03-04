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
	float val;
	size_t pos;
	int start;
	std::string tmp;
	
	start = 0;
	pos = str.find(" ");

	while(pos != str.npos)
	{
		tmp = str.substr(start, pos - start);
		val = atof(tmp.c_str());
		out.push_back(val);
		start = pos + 1;
		pos = str.find(" ", start);
	}
}

void csv::split_string_hex(std::string str, std::vector<unsigned char>& out)
{
	float val;
	size_t pos;
	int start;
	std::string tmp;
	
	start = 0;
	pos = str.find(" ");

	while(pos != str.npos)
	{
		tmp = str.substr(start, pos - start);
		tmp = "0x" + tmp;
		val = atof(tmp.c_str());
		out.push_back(val);
		start = pos + 1;
		pos = str.find(" ", start);
	}
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


void csv::read_cipher(std::string path, std::vector< std::vector<unsigned char> >& out)
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
