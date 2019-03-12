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
#include <map>
#include <functional>
#include <omp.h>
#include <chrono>
#include <random>
#include <algorithm>

#include "../common/aes-op.hpp"
#include "../common/csv_read.hpp"
#include "cpa.hpp"
#include "power-models.hpp"
#include "stats.hpp"


void cpa::cpa(std::string data_path, std::string ct_path, int candidates, int permutations)
{
	const int num_bytes = 16;
	const int num_keys = 256;	
	
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

	int candidate;

	// Prepare vectors
	std::vector< std::vector<float> > data;
	std::vector< std::vector<unsigned char> > ciphertext;
	std::vector<unsigned char> round_key (16);
	std::vector<unsigned char> full_key (16);
	std::vector< std::vector<unsigned char> > cipher (4, std::vector<unsigned char> (4));
	std::vector<float> max_correlation (16);

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
	std::cout<<"Allocating memory...\n\n";
	//std::vector< std::multimap<float, unsigned char, std::greater<float>> > r_pts
	//	( 16, std::multimap<float, unsigned char, std::greater<float>> () );
	std::vector< std::multimap<float, unsigned char> > r_pts
		( 16, std::multimap<float, unsigned char> () );
	std::vector<float> power_pts (num_traces, 0.0f);
	std::vector< std::vector< std::vector<float> > > Hamming_pts 
		( 16, std::vector< std::vector<float> > 
		(256, std::vector<float> (num_traces, 0.0f) ) );

	// Prepare the permutation of trace data; define a vector with all trace indices which is later on randomly shuffled
	std::vector<unsigned> trace_indices (num_traces);
	for (unsigned i = 0; i < num_traces; i++) {
		trace_indices.at(i) = i;
	}

	// Obtain a time-based seed
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::cout<<"Determine peak power values...\n\n";

	// Consider the maximum power point as the leakage point
	//
	// search the whole trace
	for (unsigned int i = 0; i < num_traces; i++)
	{
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

	std::cout<<"Calculate Hamming distances...\n\n";
			
	// Calculate Hamming distances
	for (unsigned int i = 0; i < num_traces; i++)
	{
		// Get cipher for this particular trace
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				cipher.at(k).at(j) = ciphertext.at(i).at(j * 4 + k);

		// Find ciphertext bytes at different stages for the Hamming 
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
			
				// Find the Hamming distance between the bytes
				Hamming_pts.at(byte_id).at(k).at(i) = pm::Hamming_dist(pre_byte, post_byte, 8);
			}
		}
	}

	// Consider multiple runs, as requested by permutations parameter
	for (int perm = 0; perm < permutations; perm++) {

		// reset Pearson correlation multimaps
		for (int i = 0; i < num_bytes; i++) {
			r_pts.at(i).clear();
		}

		std::cout<<"Generate permutation...\n\n";

		shuffle(trace_indices.begin(), trace_indices.end(), std::default_random_engine(seed));

		// Perform Pearson r correlation for this permutation Hamming distance sets and power data

		std::cout<<"Calculate Pearson correlation...\n\n";

		#pragma omp parallel for
		for (int i = 0; i < num_bytes; i++)
		{
			for (int j = 0; j < num_keys; j++)
			{
				// Pearson r correlation with power data
				//
				r_pts.at(i).emplace( std::make_pair(

							// Note that power_pts and Hamming_pts.at(i).at(j) contain the data for all traces;
							// only the data points from trace_indices[0] ... trace_indices[data_pts - 1] will
							// be considered
							stats::pearsonr(power_pts, Hamming_pts.at(i).at(j), trace_indices, num_traces),

							// keep track of the related key byte in the multimap; this allows for easy
							// extraction of the key later on
							j
						));
			}
		}

		std::cout<<"Derive the key candidates...\n\n";

		// The keys will be derived by considering all 256 options for each key byte, whereas the key bytes are ordered by the correlation
		// values -- note that this is different from deriving all possible keys (there, one would combine all 256 options for key byte 0,
		// with all 256 options for key byte 1, etc).
		//
		// depending on the runtime parameter, consider all keys by starting from 0, or provide only the most probable one, which is the
		// last
		if (candidates) {
			candidate = 0;
		}
		else {
			candidate = num_keys - 1;
		}
		for (; candidate < num_keys; candidate++) {
			for (int i = 0; i < num_bytes; i++) {

				auto iter = r_pts.at(i).begin();
				std::advance(iter, candidate);

				round_key.at(i) = iter->second;
				max_correlation.at(i) = iter->first;
			}

			// Reverse the AES key scheduling to retrieve the original key
			aes::inv_key_expand(round_key, full_key);

			// Report the key
			std::cout<<"Key candidate " << std::dec << candidate << " is (in hex): ";
			for (unsigned int i = 0; i < full_key.size(); i++)
				std::cout << std::hex << static_cast<int>(full_key.at(i)) << " ";
			std::cout<<"\n";

			std::cout<<"Related round key is (in hex): ";
			for (unsigned int i = 0; i < round_key.size(); i++)
				std::cout << std::hex << static_cast<int>(round_key.at(i)) << " ";
			std::cout<<"\n";

			// Report the related correlation values
			std::cout<<"Related Pearson correlation values are: ";
			for (unsigned int i = 0; i < full_key.size(); i++)
				std::cout << max_correlation.at(i) << " ";
			std::cout<<"\n";

			std::cout<<"\n";
		}
	}
}
		
