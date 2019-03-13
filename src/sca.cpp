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
	const char* input_msg = "\nIn order to use the SCA Tool, please provide a power traces data\n"
			          "file and a cipher texts file (in hexadecimal). Use the --help\n"
			          "flag for more information.\n\n";
	
	const char* help_msg = "\nOptions:\n\n"
				"-d           Mandatory: power traces file\n"
				"-t           Mandatory: cipher texts file\n"
				"-candidates  Optional: print all key candidates, ordered by their correlation values,\n"
				"                with the highest-correlation candidate coming last\n"
				"-steps       Optional: specify the steps size, i.e., the count of subsets of traces to consider;\n"
				"                if not provided, all the traces will be considered, but no subsets\n"
				"-steps_stop  Optional: stop exploration of subsets of traces once 100% success rate has been reached\n"
				"-perm        Optional: specify the number of permutations to explore per subset of traces;\n"
				"                if not provided, only one permutation is explored per subset\n"
				"-k           Optional: correct key file\n"
				"                if not provided, the success rate across the subsets and permutations cannot be calculated\n"
				//"-p           Optional: use parallel analysis (requires GPU)\n"
				"-nv          Optional: non-verbose logging\n"
				"\n\n";

	const char* wrong_input_msg = " is not recognized as an option\n\n";
	
	//#if !defined(OPENCL) && !defined(CUDA)
	//const char* no_gpu_found = "\nNo drivers have been found for an AMD or NVIDIA GPU.\n"
	//			   "Please install either OpenCL or CUDA to use this option.\n\n";
	//#endif	
	//// ------------------------------------------------------------------------------------
	

	//int parallel_analysis = 0;
	int data_path_set = 0;
	int ct_path_set = 0;

	bool candidates = false;
	bool verbose = true;
	bool steps_stop = false;

	int permutations = 1;
	int steps = 1;
	
	std::string data_path;
	std::string ct_path;
	std::string key_path = "";

	clock_t t = 0;

	// Check number of arguments
	if (argc == 1)
	{
		std::cerr<<input_msg;
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
		else if (!strcmp(argv[i], "-k"))
		{
			key_path = argv[i + 1];
			i++;
		}
		//else if (!strcmp(argv[i], "-p"))
		//{
		//	parallel_analysis = 1;
		//}
		else if (!strcmp(argv[i], "-candidates"))
		{
			candidates = true;
		}
		else if (!strcmp(argv[i], "-nv"))
		{
			verbose = false;
		}
		else if (!strcmp(argv[i], "-perm"))
		{
			permutations = std::stoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-steps"))
		{
			steps = std::stoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-steps_stop"))
		{
			steps_stop = true;
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
		std::cerr<<input_msg;
		return 1;
	}

	// Start timer for benchmarking purposes
	t = clock();

	//// Decide whether to use the CPU or GPU algorithm
	//if (!parallel_analysis)
	//{
		cpa::cpa(data_path, ct_path, key_path, candidates, permutations, steps, steps_stop, verbose);
	//}
	//else
	//{
	//	#if defined(OPENCL) || defined(CUDA)
	//	cpa::pcpa(data_path, ct_path);
	//	#else
	//	std::cerr<<no_gpu_found;
	//	return 1;
	//	#endif
	//}	

	// Stop clock and report times
	t = clock() - t;
	std::cout<<"The program executed in "<<((float)t)/CLOCKS_PER_SEC<<" CPU seconds\n\n";

	
	return 0;
}
