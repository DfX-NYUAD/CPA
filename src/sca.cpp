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
// This file process command line arguments and launches the Side
// Channel Algorithms
//

#include <iostream>
#include <string>
#include <cstring>
#include <ctime>

#include "common/aes-op.hpp"
#include "cpa/cpa.hpp"

int main(int argc, char *argv[])
{
	//	
	// Create error messages
	//
	// -----------------------------------------------------------------------------------
	const char* no_input_msg = "\nIn order to use the SCA Tool, please input a data file\n"
			           "and a cipher text file (in hexadecimal). Use the --help\n"
			           "flag for more information.\n\n";
	
	const char* wrong_input_num_msg = "\nPlease use one data file and one cipher text file\n"
					  "or the --help flag. An analysis type can also be specified.\n\n";

	const char* help_msg = "\nOptions:\n\n"
				"-d           Specify data file\n"
				"-t           Specify cipher text file\n"
				"-perm        Specify the number of permutations to consider; if not provided, only 1 is considered\n"
				"-step        Specify the step size for reducing traces to consider, in percent; if not provided, all traces will be considered\n"
				"-candidates  Print all key candidates, ordered by their correlation values\n"
				"-p           Use parallel analysis (requires GPU)\n"
				"\n\n";

	const char* wrong_input_msg = " is not recognized as an option\n\n";
	
	#if !defined(OPENCL) && !defined(CUDA)
	const char* no_gpu_found = "\nNo drivers have been found for an AMD or NVIDIA GPU.\n"
				   "Please install either OpenCL or CUDA to use this option.\n\n";
	#endif	
	// ------------------------------------------------------------------------------------
	

	int parallel_analysis = 0;
	int data_path_set = 0;
	int ct_path_set = 0;
	int candidates = 0;

	int permutations = 1;
	int step_size = 100;
	
	std::string data_path;
	std::string ct_path;
	std::string aes_path;

	clock_t t = 0;

	// Check number of arguments
	if (argc == 1)
	{
		std::cerr<<no_input_msg;
		return 1;
	}

	// Check command line arguments
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--help"))
		{
			std::cout<<help_msg;
			return 0;
		}
		else if (!strcmp(argv[i], "-d"))
		{
			data_path = argv[i + 1];
			data_path_set = 1;
			i++;
		}
		else if (!strcmp(argv[i], "-t"))
		{
			ct_path = argv[i + 1];
			ct_path_set = 1;
			i++;
		}
		else if (!strcmp(argv[i], "-p"))
		{
			parallel_analysis = 1;
		}
		else if (!strcmp(argv[i], "-candidates"))
		{
			candidates = 1;
		}
		else if (!strcmp(argv[i], "-perm"))
		{
			permutations = std::stoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-step"))
		{
			step_size = std::stoi(argv[i + 1]);
			i++;
		}
		else 
		{
			std::cerr<<std::endl<<argv[i]<<wrong_input_msg;
			return 1;
		}
	}

	// Make sure the user provides a data file and ciphertext file
	if (!data_path_set || !ct_path_set)
	{
		std::cerr<<wrong_input_num_msg;
		return 1;
	}

	// Start timer for benchmarking purposes
	t = clock();

	// Decide whether to use the CPU or GPU algorithm
	if (!parallel_analysis)
	{
		cpa::cpa(data_path, ct_path, candidates, permutations, step_size);
	}
	else
	{
		#if defined(OPENCL) || defined(CUDA)
		cpa::pcpa(data_path, ct_path);
		#else
		std::cerr<<no_gpu_found;
		return 1;
		#endif
	}	

	// Stop clock and report times
	t = clock() - t;
	std::cout<<"The program executed in "<<((float)t)/CLOCKS_PER_SEC<<" seconds\n\n";

	
	return 0;
}
