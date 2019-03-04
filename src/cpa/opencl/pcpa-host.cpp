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

#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <CL/cl.h>
#include <omp.h>

#include "../../common/aes-op.hpp"
#include "../../common/csv_read.hpp"
#include "../cpa.hpp"
#include "../power-models.hpp"
#include "../stats.hpp"

// Simple function to print OpenCL errors
void cl_check_errors(cl_int ret, const char* function_call)
{
	if (ret != CL_SUCCESS)
	{
		std::cerr<<"error: call to '"<<function_call<<"' failed\n";
		exit(1);
	}
}

// Simple function to read in the '.cl' file
unsigned char *read_buffer(const char *file_name, size_t *size_ptr)
{
        FILE *f;
        unsigned char *buf;
        size_t size;

        /* Open file */
        f = fopen(file_name, "rb");
        if (!f)
                return NULL;

        /* Obtain file size */
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);

        /* Allocate and read buffer */
        buf = (unsigned char*)malloc(size + 1);
        fread(buf, 1, size, f);
        buf[size] = '\0';

        /* Return size of buffer */
        if (size_ptr)
                *size_ptr = size;

        /* Return buffer */
        return buf;
}

void cpa::pcpa(std::string data_path, std::string ct_path)
{
	const int num_bytes = 16;
	const int num_keys = 256;	
	
	int max_index;	
	int gridSize;
	int blockSize;
	
	unsigned int num_traces;
	unsigned int num_pts;
	unsigned int round_size;
	unsigned int trace_start;

	unsigned char max_byte;
	
	clock_t t = 0;
	
	// Prepare vectors
	std::vector< std::vector<float> > data;
	std::vector< std::vector<unsigned char> > ciphertext;
	std::vector<unsigned char> round_key (16);
	std::vector<unsigned char> full_key (16);
	std::vector< std::vector<unsigned char> > cipher (4, std::vector<unsigned char> (4));

	// Print information to terminal
	std::cout<<"\n\nMethod of Analysis: CPA";
	std::cout<<"\nUsing GPU: OpenCL";
	std::cout<<"\nReading data from: "<<data_path;
	std::cout<<"\nReading ciphertext from: "<<ct_path<<"\n\n";

	// Read in ciphertext and power data
	csv::read_data(data_path, data);
	csv::read_cipher(ct_path, ciphertext);

	// Record the number of traces and the
	// number of points per trace
	num_traces = data.size();
	num_pts = data.at(0).size();
	
	// Set dimensions for GPU kernels
	blockSize = 16;
	gridSize = num_traces;

	// Calcute a rough size of an AES round
	// and mark a point to start looking for a
	// maximum power
	round_size = num_pts / 5;
	trace_start = num_pts - round_size;

	std::cout<<"Allocating memory...\n";
	
	

	///////////////////////////////// OpenCL Setup /////////////////////////////////////////

	cl_int ret;
	
	/* Get platform */
	cl_platform_id platform;
	cl_uint num_platforms;
	ret = clGetPlatformIDs(1, &platform, &num_platforms);
	
	cl_check_errors(ret, "clGetPlatformIDs");

	printf("Number of platforms: %d\n", num_platforms);
	printf("platform=%p\n", platform);

	/* Get platform name */
	char platform_name[100];
	ret = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name),         
			platform_name, NULL);
	
	cl_check_errors(ret, "clGetPlatformInfo");
	
	printf("platform.name='%s'\n", platform_name);
	printf("\n");

	/*
	 * Device
	 */

	/* Get device */
	cl_device_id device;
	cl_uint num_devices;
	ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);
        
	cl_check_errors(ret, "clGetDeviceIDs");

	printf("Number of devices: %d\n", num_devices);
	printf("device=%p\n", device);

	/* Get device name */
	char device_name[100];
	ret = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name,    
			NULL);
	
	cl_check_errors(ret, "clGetDeviceInfo");

	printf("device.name='%s'\n", device_name);
	printf("\n");


	/*
	 * Context
	 */

	/* Create context */
	cl_context context;
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
	
	cl_check_errors(ret, "clCreateContext");

	printf("context=%p\n", context);


	/*
	 * Command Queue
	 */

	/* Create command queue */
	cl_command_queue command_queue;
	command_queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &ret);
	
	cl_check_errors(ret, "clCreateCommandQueue");
	
	printf("command_queue=%p\n", command_queue);
	printf("\n");



	/*
	 * Program
	 */

	/* Program source */
	const unsigned char *source_code;
	size_t source_length;
	//const unsigned char *binary;
	//size_t binary_length;
	const char* file_path = "../src/cpa/opencl/pcpa-kernel.cl";

	/* Read program from 'pcpa-kernel.cl' */
	source_code = read_buffer(file_path, &source_length);
	if (!source_code)
	{
		printf("error: cannot open %s\n", file_path);
		exit(1);
	}

	/* Create a program */
	cl_program program;
	//program = clCreateProgramWithBinary(context, 1, &device, &binary_length,
	//	&binary, NULL, &ret);
	program = clCreateProgramWithSource(context, 1, (const char**)&source_code, 
		&source_length, &ret);
	
	cl_check_errors(ret, "clCreateProgramWithSource");
	
	printf("program=%p\n", program);

	/* Build program */
	ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (ret != CL_SUCCESS )
	{
		size_t size;
		char *log;

		/* Get log size */
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 
				0, NULL,      &size);

		/* Allocate log and print */
		log = (char*)malloc(size);
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 
				size, log,    NULL);
		printf("error: call to 'clBuildProgram' failed:\n%s\n", log);

		/* Free log and exit */
		free(log);
		exit(1);
	}
	printf("program built\n\n");


	/*
 	 * Kernel
 	 */

	/* Create kernels */

	// Reduction kernel to find maximum power points
	cl_kernel pcpa_reduce;
	pcpa_reduce = clCreateKernel(program, "pcpa_reduce", &ret);
	cl_check_errors(ret, "clCreateKernel(pcpa_reduce)");
	printf("kernel[0]=%p\n", pcpa_reduce);

	// Kernel to calculate hamming distances
	cl_kernel pcpa_kernel;
	pcpa_kernel = clCreateKernel(program, "pcpa", &ret);
	cl_check_errors(ret, "clCreateKernel(pcpa_kernel)");
	printf("kernel[1]=%p\n", pcpa_kernel);


	//////////////////////// Open CL Memory Setup /////////////////////////////

	/*
 	 * Buffers
 	 */

	cl_float* h_data = (cl_float*)malloc(sizeof(cl_float) * num_pts * num_traces);

	#pragma omp parallel for
	for (unsigned int i = 0; i < num_traces; i++)
	{
		for ( unsigned int j = 0; j < num_pts; j++)
		{
			h_data[i * num_pts + j] = data.at(i).at(j);
		}
	}

	/* Create buffer */
	cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE, 
		(sizeof(cl_float) * num_pts * num_traces), NULL, &ret);
		
	cl_check_errors(ret, "clCreateBuffer(d_data)");
        
	/* Copy buffer */
        ret = clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE,
                0, num_pts * num_traces * sizeof(cl_float), h_data, 0, NULL, NULL);

	cl_check_errors(ret, "clEnqueueWriteBuffer");



	cl_uchar* h_ciphertext = (cl_uchar*)malloc(sizeof(cl_uchar) * num_bytes * num_traces);

	#pragma omp parallel for
	for (unsigned int i = 0; i < num_traces; i++)
	{
		for (int j = 0; j < num_bytes; j++)
		{
			h_ciphertext[i * num_bytes + j] = ciphertext.at(i).at(j);
		}
	}

	/* Create buffer */
	cl_mem d_ciphertext = clCreateBuffer(context, CL_MEM_READ_WRITE, 
		(sizeof(cl_uchar) * num_bytes * num_traces), NULL, &ret);
		
	cl_check_errors(ret, "clCreateBuffer(d_ciphertext)");
        
	/* Copy buffer */
        ret = clEnqueueWriteBuffer(command_queue, d_ciphertext, CL_TRUE,
                0, num_bytes * num_traces * sizeof(cl_uchar), h_ciphertext, 0, NULL, NULL);

	cl_check_errors(ret, "clEnqueueWriteBuffer");
		

	/* Create buffer */
	cl_mem d_inv_sbox = clCreateBuffer(context, CL_MEM_READ_WRITE, 
		(sizeof(cl_uchar) * num_keys), NULL, &ret);
		
	cl_check_errors(ret, "clCreateBuffer(d_inv_sbox)");
        
	/* Copy buffer */
        ret = clEnqueueWriteBuffer(command_queue, d_inv_sbox, CL_TRUE,
                0, num_keys * sizeof(cl_uchar), &aes::inv_sbox, 0, NULL, NULL);

	cl_check_errors(ret, "clEnqueueWriteBuffer");


	/* Create buffer */
	cl_mem d_hamming_pts = clCreateBuffer(context, CL_MEM_READ_WRITE,
		(sizeof(cl_uchar) * num_bytes * num_keys * num_traces), NULL, &ret);

	cl_check_errors(ret, "clCreateBuffer(d_hamming_pts)");

	/* Create buffer */
	cl_mem d_power_pts = clCreateBuffer(context, CL_MEM_READ_WRITE, 
		(sizeof(cl_float) * num_traces), NULL, &ret);

	cl_check_errors(ret, "clCreateBuffer(d_power_pts)");

		
	cl_uint cl_trace_start = trace_start;
	cl_uint cl_num_pts = num_pts;
	cl_uint cl_num_traces = num_traces;

	// Set each argument for the reduction kernel
	ret = clSetKernelArg(pcpa_reduce, 0, sizeof(cl_mem), &d_data);
	cl_check_errors(ret, "clSetKernelArg(data)");
	ret = clSetKernelArg(pcpa_reduce, 1, sizeof(cl_mem), &d_power_pts);
	cl_check_errors(ret, "clSetKernelArg(power_pts)");
	ret = clSetKernelArg(pcpa_reduce, 2, sizeof(cl_uint), &cl_trace_start);
	cl_check_errors(ret, "clSetKernelArg(trace_start)");
	ret = clSetKernelArg(pcpa_reduce, 3, sizeof(cl_uint), &cl_num_pts);
	cl_check_errors(ret, "clSetKernelArg(num_pts)");
	ret = clSetKernelArg(pcpa_reduce, 4, sizeof(cl_uint), &cl_num_traces);
	cl_check_errors(ret, "clSetKernelArg(num_traces)");
	ret = clSetKernelArg(pcpa_reduce, 5, sizeof(cl_float) * (num_pts - trace_start), NULL);
	cl_check_errors(ret, "clSetKernelArg(sdata)");


	// Set each argument for the hamming distance kernel
	ret = clSetKernelArg(pcpa_kernel, 0, sizeof(cl_mem), &d_ciphertext);
	cl_check_errors(ret, "clSetKernelArg(ciphertext)");
	ret |= clSetKernelArg(pcpa_kernel, 1, sizeof(cl_mem), &d_power_pts);
	cl_check_errors(ret, "clSetKernelArg(power_pts)");
	ret |= clSetKernelArg(pcpa_kernel, 2, sizeof(cl_mem), &d_hamming_pts);
	cl_check_errors(ret, "clSetKernelArg(d_hamming_pts)");
	ret |= clSetKernelArg(pcpa_kernel, 3, sizeof(cl_uint), &cl_num_pts);
	cl_check_errors(ret, "clSetKernelArg(num_pts)");
	ret |= clSetKernelArg(pcpa_kernel, 4, sizeof(cl_uint), &cl_num_traces);
	cl_check_errors(ret, "clSetKernelArg(num_traces)");
	ret |= clSetKernelArg(pcpa_kernel, 5, sizeof(cl_mem), &d_inv_sbox);
	ret |= clSetKernelArg(pcpa_kernel, 6, sizeof(unsigned char) * 16, NULL);

	cl_check_errors(ret, "clSetKernelArg(d_inv_sbox)");
	
	/*
	 * Launch Kernel
	 */

	size_t global_work_size = num_traces * (num_pts - trace_start);
	size_t local_work_size = num_pts - trace_start;

	
	cl_event eventProfile;
	cl_ulong start, end;

	/* Launch the kernel */
	ret = clEnqueueNDRangeKernel(command_queue, pcpa_reduce, 1, NULL,
			&global_work_size, &local_work_size, 0, NULL, &eventProfile);
	
	cl_check_errors(ret, "clEnqueueNDRangeKernel(pcpa_reduce)");

	/* Wait for it to finish */
	clFinish(command_queue);

	ret = clGetEventProfilingInfo(eventProfile, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, 0);
	ret |= clGetEventProfilingInfo(eventProfile, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, 0);
	cl_check_errors(ret, "clGetEventProfilingInfo(reduce)");
	std::cout<<"\nThe reduction kernel executed in " << (end - start)*1.0e-6f <<" (ms)\n";


	global_work_size = num_traces * 16;
	local_work_size = 16;


	/* Launch the kernel */
	ret = clEnqueueNDRangeKernel(command_queue, pcpa_kernel, 1, NULL,
			&global_work_size, &local_work_size, 0, NULL, &eventProfile);
	
	cl_check_errors(ret, "clEnqueueNDRangeKernel(pcpa_kernel)");

	/* Wait for it to finish */
	clFinish(command_queue);
	
	ret = clGetEventProfilingInfo(eventProfile, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, 0);
	ret |= clGetEventProfilingInfo(eventProfile, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, 0);
	cl_check_errors(ret, "clGetEventProfilingInfo(reduce)");
	std::cout<<"\nThe hamming distance kernel executed in " << (end - start)*1.0e-6f <<" (ms)\n";


	////////////////////////////////////////////////////////////////////////////////////////
	
	std::vector<float> power_pts (num_traces, 0.0f);
	std::vector< std::vector<float> > r_pts ( num_bytes, std::vector<float> (num_keys, 0.0f) );
	std::vector< std::vector< std::vector<float> > > hamming_pts 
		( num_bytes, std::vector< std::vector<float> > 
		(num_keys, std::vector<float> (num_traces ,0.0f) ) );

	cl_uchar* h_hamming_pts = (cl_uchar*)calloc(num_bytes * num_keys * num_traces * sizeof(cl_uchar), 1);
	
	/////////////////////////  Read Buffers  ///////////////////////////////////////////////
	
	ret = clEnqueueReadBuffer(command_queue, d_hamming_pts, CL_TRUE,
                0, num_bytes * num_keys * num_traces * sizeof(cl_uchar), h_hamming_pts, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, d_power_pts, CL_TRUE,
                0, num_traces * sizeof(cl_float), &power_pts[0], 0, NULL, NULL);

	//////////////////// Resume serial operation /////////////////////////////////////////

	// Copy array into 3D vector
	#pragma omp parallel for
	for (int i= 0; i < num_bytes; i++)
		for (int j = 0; j < num_keys; j++)
			for (unsigned int k = 0; k < num_traces; k++)
				hamming_pts.at(i).at(j).at(k) = (float)h_hamming_pts[(i * num_keys * num_traces) + (j * num_traces) + k ];



	
	//Start clock
	t = clock();
	
	// Perform Pearson r correlation to find the hamming distance set
	// with the highest correlation to the actual data
	#pragma omp parallel for
	for (int i = 0; i < num_bytes; i++)
	{
		for (int j = 0; j < num_keys; j++)
		{
			// Pearson r correlation with power data
			r_pts.at(i).at(j) = stats::pearsonr(power_pts, hamming_pts.at(i).at(j));
		}
	}

	// Find the hamming distance set with the highest correlation
	for (int i = 0; i < num_bytes; i++)
	{
		max_index = 0;
		
		for (unsigned int j = 0; j < r_pts.at(i).size(); j++)
		{
			if (r_pts.at(i).at(max_index) < r_pts.at(i).at(j))
				max_index = j;
		}
		
		max_byte = static_cast<unsigned char> (max_index);
		round_key.at(i) = max_byte;
	}

	// Report time
	t = clock() - t;
	std::cout<<"\nThe Pearson Correlation functions executed in " << (float(t))/CLOCKS_PER_SEC << " seconds\n";

	// Reverse the AES key scheduling to retrieve the orginal key
	aes::inv_key_expand(round_key, full_key);

	// Report the key
	std::cout<<"\nKey is ";
	for (unsigned int i = 0; i < full_key.size(); i++)
		std::cout<< static_cast<int> (full_key.at(i)) <<", ";

	std::cout<<"\n\n";
}



