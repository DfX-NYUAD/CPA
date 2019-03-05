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

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <omp.h>

#include "../common/aes-op.hpp"
#include "../common/csv_read.hpp"
#include "cpa.hpp"
#include "power-models.hpp"
#include "stats.hpp"


void cpa::cpa(std::string data_path, std::string ct_path)
{
	const int num_bytes = 16;
	const int num_keys = 256;	
	
	int max_index;	
	int pre_row;
	int pre_col;
	int post_row;
	int post_col;
	int byte_id;

	size_t num_traces;
	size_t num_pts;
	size_t trace_start;

	float max_pt;
	float data_pt;

	unsigned char pre_byte;
	unsigned char post_byte;
	unsigned char key_byte;
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
	std::cout<<"\nUsing GPU: NO";
	std::cout<<"\nReading data from: "<<data_path;
	std::cout<<"\nReading ciphertext from: "<<ct_path<<"\n\n";

	// Read in ciphertext and power data
	csv::read_data(data_path, data);
	csv::read_cipher(ct_path, ciphertext);

	// Record the number of traces and the
	// number of points per trace
	num_traces = data.size();
	num_pts = data.at(0).size();

	// Prepare main vectors
	std::cout<<"Allocating memory...\n";
	std::vector< std::vector<float> > r_pts ( 16, std::vector<float> (256, 0.0f) );
	std::vector<float> power_pts (num_traces, 0.0f);
	std::vector< std::vector< std::vector<float> > > hamming_pts 
		( 16, std::vector< std::vector<float> > 
		(256, std::vector<float> (num_traces, 0.0f) ) );

	// Start clock
	t = clock();

	// Find the power leakage points
	for (unsigned int i = 0; i < num_traces; i++)
	{
		// Print progress to terminal
		if ( (i + 1) != num_traces )
		{
			std::cout<<"Finding Hamming distances and power points on trace "
				<<i + 1<<" of "<<num_traces<<"\r";
			std::cout.flush();	
		}
		else
		{
			std::cout<<"Finding Hamming distances and power points on trace "
				<<i + 1<<" of "<<num_traces<<"\n";
		}

		// Use the maximum power point for the leakage point
		//
		// search the whole trace
		trace_start = 0;
		max_pt = 0.0f;
		for (unsigned int j = trace_start; j < num_pts; j++)
		{
			data_pt = data.at(i).at(j);
			if (max_pt < data_pt)
				max_pt = data_pt;
		}
	
		power_pts.at(i) = max_pt;
	}
			
	// Report time
	t = clock() - t;
	std::cout<<"\nThe power points selection took "<<(float(t)) / CLOCKS_PER_SEC << " seconds\n";

	// Start clock
	t = clock();

	// Calcuate hamming distances
	for (unsigned int i = 0; i < num_traces; i++)
	{
		// Get cipher for this particular trace
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				cipher.at(k).at(j) = ciphertext.at(i).at(j * 4 + k);

		// Find ciphertext bytes at different stages for the hamming 
		// distance calculation
		for (int j = 0; j < num_bytes; j++)
		{
			// Select byte
			post_row = j / 4;
			post_col = j % 4;
			post_byte = cipher.at(post_row).at(post_col);

			// Create all possible bytes that could have resulted
			// selected byte
			for (int k = 0; k < num_keys; k++)
			{
				// Undo AES-128 operations
				key_byte = static_cast<unsigned char> (k);
				aes::shift_rows(post_row, post_col, pre_row, pre_col);
				pre_byte = cipher.at(pre_row).at(pre_col);
				pre_byte = aes::add_round_key(key_byte, pre_byte);
				pre_byte = aes::inv_sub_bytes(pre_byte);
				byte_id = pre_col * 4 + pre_row;
			
				// Find the hamming distance between the bytes
				hamming_pts.at(byte_id).at(k).at(i) = pm::hamming_dist(pre_byte, post_byte, 8);
			}
		}
	}

	// Report time
	t = clock() - t;
	std::cout<<"\nThe hamming distance calculation took "<<(float(t)) / CLOCKS_PER_SEC << " seconds\n";
	
	// Start clock	
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
	std::cout<<"\nThe Pearson r correlation "<<(float(t)) / CLOCKS_PER_SEC << " seconds\n";
	
	// Reverse the AES key scheduling to retrieve the orginal key
	aes::inv_key_expand(round_key, full_key);

	// Report the key
	std::cout<<"\nKey in hex is ";
	for (unsigned int i = 0; i < full_key.size(); i++)
		std::cout << std::hex << static_cast<int>(full_key.at(i)) << " ";

	std::cout<<"\n\n";
}
		
