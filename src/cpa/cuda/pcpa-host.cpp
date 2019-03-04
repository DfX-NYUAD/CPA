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

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cuda.h>
#include <cuda_runtime.h>
#include <ctime>
#include <omp.h>

#include "../../common/aes-op.hpp"
#include "../../common/csv_read.hpp"
#include "../cpa.hpp"
#include "pcpa-kernel.hpp"
#include "pcpa-reduction.hpp"
#include "../power-models.hpp"
#include "../stats.hpp"


void cpa::pcpa(std::string data_path, std::string ct_path)
{
	const int num_bytes = 16;
	const int num_keys = 256;	
	
	int max_index;	
	int gridSize;
	int blockSize;
	float* d_power_pts;
	float* d_data;
	unsigned char* d_ciphertext;
	unsigned char* d_hamming_pts;
	
	unsigned int num_traces;
	unsigned int num_pts;
	unsigned int round_size;
	unsigned int trace_start;

	unsigned char max_byte;

	unsigned char* d_inv_sbox;
	
	//clock_t t = 0;

	std::vector< std::vector<float> > data;
	std::vector< std::vector<unsigned char> > ciphertext;
	std::vector<unsigned char> round_key (16);
	std::vector<unsigned char> full_key (16);
	std::vector< std::vector<unsigned char> > cipher (4, std::vector<unsigned char> (4));

	std::cout<<"\n\nMethod of Analysis: CPA";
	std::cout<<"\nUsing GPU: CUDA";
	std::cout<<"\nReading data from: "<<data_path;
	std::cout<<"\nReading ciphertext from: "<<ct_path<<"\n\n";

	csv::read_data(data_path, data);
	csv::read_cipher(ct_path, ciphertext);

	num_traces = data.size();
	num_pts = data.at(0).size();
	
	blockSize = 16;
	gridSize = num_traces;

	round_size = num_pts / 5;
	trace_start = num_pts - round_size;

	std::cout<<"Allocating memory...\n";
	
	unsigned char* h_hamming_pts = new unsigned char [num_bytes * num_keys * num_traces];
	
	float* h_data = new float [num_pts * num_traces];

	#pragma omp parallel for
	for (unsigned int i = 0; i < num_traces; i++)
	{
		for ( unsigned int j = 0; j < num_pts; j++)
		{
			h_data[i * num_pts + j] = data.at(i).at(j);
		}
	}

	unsigned char* h_ciphertext = new unsigned char [num_bytes * num_traces];

	#pragma omp parallel for
	for (unsigned int i = 0; i < num_traces; i++)
	{
		for (int j = 0; j < num_bytes; j++)
		{
			h_ciphertext[i * num_bytes + j] = ciphertext.at(i).at(j);
		}
	}

///////////////////////////////// CUDA Memory Allocation /////////////////////////////////////////////////
	
	cudaMalloc(&d_inv_sbox, sizeof(unsigned char) * num_keys);
	cudaMemcpy(d_inv_sbox, aes::inv_sbox, sizeof(unsigned char) * num_keys, cudaMemcpyHostToDevice);
	cudaMalloc(&d_power_pts, sizeof(float) * num_traces);
	cudaMalloc(&d_hamming_pts, sizeof(unsigned char) * num_bytes * num_keys * num_traces);
	
	cudaMalloc(&d_data, sizeof(float) * num_pts * num_traces);
	cudaMemcpy(d_data, h_data, sizeof(float) * num_pts * num_traces, cudaMemcpyHostToDevice);
	cudaMalloc(&d_ciphertext, sizeof(unsigned char) * num_pts * num_traces);
	cudaMemcpy(d_ciphertext, h_ciphertext, sizeof(unsigned char) * num_pts * num_traces, cudaMemcpyHostToDevice);


/////////////////////////////////////////////////////////////////////////////////////////
	std::vector<float> power_pts (num_traces, 0.0f);
	std::vector< std::vector<float> > r_pts ( num_bytes, std::vector<float> (num_keys, 0.0f) );
	std::vector< std::vector< std::vector<float> > > hamming_pts 
		( num_bytes, std::vector< std::vector<float> > 
		(num_keys, std::vector<float> (num_traces ,0.0f) ) );

/////////////////////////  Launching Kernels //////////////////////////////////////////////

	pcpa_reduce_wrapper(d_data, d_power_pts, trace_start, num_pts, num_traces);
	
	pcpa_wrapper(gridSize, blockSize, d_ciphertext, d_power_pts, d_hamming_pts, num_pts, num_traces, d_inv_sbox);


////////////////////////////////////////////////////////////////////////////////////////
		
	cudaMemcpy(&power_pts[0], d_power_pts, sizeof(float) * num_traces, cudaMemcpyDeviceToHost);
	cudaMemcpy(h_hamming_pts, d_hamming_pts, sizeof(unsigned char) * num_bytes * num_keys * num_traces, cudaMemcpyDeviceToHost);
	
	cudaFree(d_inv_sbox);
	cudaFree(d_power_pts);
	cudaFree(d_hamming_pts);
	cudaFree(d_data);
	cudaFree(d_ciphertext);

	#pragma omp parallel for
	for (int i= 0; i < num_bytes; i++)
		for (int j = 0; j < num_keys; j++)
			for (unsigned int k = 0; k < num_traces; k++)
				hamming_pts.at(i).at(j).at(k) = h_hamming_pts[(i * num_keys * num_traces) + (j * num_traces) + k ];
	
	//t = clock();

	#pragma omp parallel for
	for (int i = 0; i < num_bytes; i++)
	{
		for (int j = 0; j < num_keys; j++)
		{
			r_pts.at(i).at(j) = stats::pearsonr(power_pts, hamming_pts.at(i).at(j));
		}
	}


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

	//t = clock() - t;

	//std::cout<<"\nThe Pearson r correlation computations took " <<(float(t))/CLOCKS_PER_SEC<<" seconds\n";

	aes::inv_key_expand(round_key, full_key);

	std::cout<<"\nKey is ";
	for (unsigned int i = 0; i < full_key.size(); i++)
		std::cout<< static_cast<int> (full_key.at(i)) <<", ";

	std::cout<<std::endl<<std::endl;

}
		
