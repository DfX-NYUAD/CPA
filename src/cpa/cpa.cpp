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
#include <iomanip>
#include <fstream>

#include "../common/aes-op.hpp"
#include "../common/csv_read.hpp"
#include "cpa.hpp"
#include "power-models.hpp"
#include "stats.hpp"


void cpa::cpa(std::string data_path, std::string ct_path, std::string key_path, std::string perm_path, bool candidates, int permutations, int steps, int steps_start, bool steps_stop, bool verbose)
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

	bool perm_file_write = false;
	bool perm_file_read = false;
	std::fstream perm_file;

	// Obtain a time-based seed
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	// Prepare vectors
	std::vector< std::vector<float> > data;
	std::vector< std::vector<unsigned char> > ciphertext;
	std::vector< std::vector<unsigned char> > correct_key;
	std::vector<unsigned char> round_key (num_bytes);
	std::vector<unsigned char> full_key (num_bytes);
	std::vector< std::vector<unsigned char> > cipher (4, std::vector<unsigned char> (4));
	std::vector<float> max_correlation (num_bytes);
	std::vector< std::vector< std::vector<unsigned> > > perm_from_file;

	// Print information to terminal
	//std::cout<<"\n\nMethod of Analysis: CPA";
	//std::cout<<"\nUsing GPU: NO";
	std::cout<<"\n";
	std::cout<<"Reading data from: "<<data_path;
	std::cout<<"\n";
	std::cout<<"Reading ciphertext from: "<<ct_path;
	std::cout<<std::endl;

	// Read in ciphertext and power data
	csv::read_data(data_path, data);
	csv::read_hex(ct_path, ciphertext);

	// Record the number of traces and the
	// number of points per trace
	num_traces = data.size();
	num_pts = data.at(0).size();

	// Read in the correct key, if provided 
	if (key_path != "") {

		std::cout<<"Reading correct key from: " << key_path << ": ";

		csv::read_hex(key_path, correct_key);

		for (unsigned int i = 0; i < full_key.size(); i++)
			std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(correct_key[0].at(i)) << " ";
		std::cout<<std::endl;
	}

	// Handle the permutations file, if provided
	if (perm_path != "") {

		std::cout<<"Handle permutations file: " << perm_path << std::endl;

		// try to read in permutations from file
		perm_file_read = csv::read_perm_file(
				perm_path,
				steps,
				steps_start,
				permutations,
				num_traces,
				perm_from_file
			);

		// if reading failed, consider to write out to the perm_file
		if (!perm_file_read) {
			perm_file_write = true;
			perm_file.open(perm_path.c_str(), std::fstream::out);

			std::cout << "Reading of permutations file failed; permutations are generated randomly and written out to this file" << std::endl;
		}
	}

	// Prepare main vectors
	std::cout<<"\n";
	std::cout<<"Allocating memory...\n";
	std::cout<<std::endl;
	//std::vector< std::multimap<float, unsigned char, std::greater<float>> > r_pts
	//	( num_bytes, std::multimap<float, unsigned char, std::greater<float>> () );
	std::vector< std::multimap<float, unsigned char> > r_pts
		( num_bytes, std::multimap<float, unsigned char> () );
	std::vector<float> power_pts (num_traces, 0.0f);
	std::vector< std::vector< std::vector<float> > > Hamming_pts 
		( num_bytes, std::vector< std::vector<float> > 
		(256, std::vector<float> (num_traces, 0.0f) ) );

	std::vector<unsigned> trace_indices (num_traces);
	// Prepare with all trace indices; required for shuffling/generating permutations in case they are not read in
	for (unsigned i = 0; i < num_traces; i++) {
		trace_indices.at(i) = i;
	}

	std::cout<<"Determine peak power values...\n";
	std::cout<<std::endl;

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

	std::cout<<"Calculate Hamming distances...\n";
	std::cout<<std::endl;
			
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

	// Consider multiple runs, as requested by step_size parameter
	//
	// s has to be float, to properly calculate data_pts
	for (float s = steps_start; s <= steps; s++) {

		int data_pts = num_traces * (s / steps);

		std::cout << "Working on step " << std::dec << s << "...\n";

		float data_pts_perc = data_pts;
		data_pts_perc /= num_traces;
		data_pts_perc *= 100;
		std::cout << "(" << data_pts << " / " << num_traces << ") traces = " << data_pts_perc << " % of all traces\n";
		std::cout<<std::endl;

		float overall_avg_cor = 0.0;
		float success_rate = 0.0;
		float HD = 0;

		// Consider multiple runs, as requested by permutations parameter
		for (int perm = 1; perm <= permutations; perm++) {

			// reset Pearson correlation multimaps
			for (int i = 0; i < num_bytes; i++) {
				r_pts.at(i).clear();
			}

			if (verbose) {
				std::cout<<"Consider permutation #" << std::dec << perm << "...\n";
				std::cout<<std::endl;
			}

			// in case pre-defined permutations have been read in, use those
			if (perm_file_read) {

				trace_indices = perm_from_file[s - steps_start][perm - 1];
			}
			// otherwise, consider a random one
			else {
				shuffle(trace_indices.begin(), trace_indices.end(), std::default_random_engine(seed));

				// also write out the newly generated, random permutation
				if (perm_file_write) {

					// write out markers for simpler parsing
					if (perm == 1) {
						perm_file << "STEP_START" << std::endl;
					}
					perm_file << "PERM_START" << std::endl;

					// only write out the first #(data_pts) as needed
					for (int d = 0; d < data_pts; d++) {
						perm_file << trace_indices[d] << " ";
					}
					// one permutation per line, for readability
					perm_file << std::endl;
				}
			}

			// Perform Pearson r correlation for this permutation Hamming distance sets and power data

			if (verbose) {
				std::cout<<"Calculate Pearson correlation...\n";
				std::cout<<std::endl;
			}

			#pragma omp parallel for
			for (int i = 0; i < num_bytes; i++)
			{
				for (int j = 0; j < num_keys; j++)
				{
					// Pearson r correlation with power data
					//
					r_pts.at(i).emplace( std::make_pair(

								// Note that power_pts and Hamming_pts.at(i).at(j) contain the data for all
								// traces; only the #data_pts data points from trace_indices[0] ...
								// trace_indices[data_pts - 1] will be considered
								stats::pearsonr(power_pts, Hamming_pts.at(i).at(j), trace_indices, data_pts),

								// keep track of the related key byte in the multimap; this allows for easy
								// extraction of the key later on
								j
							));
				}
			}

			if (verbose) {
				std::cout<<"Derive the key candidates...\n";
				std::cout<<std::endl;
			}

			// The keys will be derived by considering all 256 options for each key byte, whereas the key bytes are ordered by
			// the correlation values -- note that this is different from deriving all possible keys (there, one would combine
			// all 256 options for key byte 0, with all 256 options for key byte 1, etc).

			// depending on the runtime parameter, consider all keys by starting from 0, or provide only the most probable one,
			// which is the last
			if (candidates) {
				candidate = 0;
			}
			else {
				candidate = num_keys - 1;
			}

			float avg_cor;
			for (; candidate < num_keys; candidate++) {

				// track max and average correlation
				avg_cor = 0.0;
				for (int i = 0; i < num_bytes; i++) {

					auto iter = r_pts.at(i).begin();
					std::advance(iter, candidate);

					round_key.at(i) = iter->second;
					max_correlation.at(i) = iter->first;

					avg_cor += max_correlation.at(i);
				}
				avg_cor /= num_bytes;

				// Reverse the AES key scheduling to retrieve the original key
				aes::inv_key_expand(round_key, full_key);

				if (verbose) {

					// Report the key
					if (candidates) {
						std::cout<<"Key candidate " << std::dec << candidate << " (in hex): ";
					}
					else {
						std::cout<<"Full key (in hex):  ";
					}
					for (unsigned int i = 0; i < full_key.size(); i++)
						std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(full_key.at(i)) << " ";
					std::cout<<"\n";

					std::cout<<"Round key (in hex): ";
					for (unsigned int i = 0; i < round_key.size(); i++)
						std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(round_key.at(i)) << " ";
					std::cout<<"\n";

					// Report the related correlation values
					std::cout<<"Related Pearson correlation values are: ";

					for (unsigned int i = 0; i < full_key.size(); i++) {
						std::cout << std::dec << max_correlation.at(i) << " ";
					}
					std::cout<<"\n";

					std::cout<<"Avg Pearson correlation across all key bytes: " << avg_cor;
					std::cout<<"\n";

					std::cout<<std::endl;
				}
			} // candidate

			// track correlation across all permutations, but only for last candidate
			overall_avg_cor += avg_cor;

			// also track success rate and HD for that last candidate, if correct key was provided
			if (!correct_key.empty()) {
				bool success = true;
				for (unsigned int i = 0; i < full_key.size(); i++) {

					if  (full_key.at(i) != correct_key[0].at(i)) {
						success = false;
					}

					HD += pm::Hamming_dist(full_key.at(i), correct_key[0].at(i), 8);
					//std::cout << pm::Hamming_dist(full_key.at(i), correct_key[0].at(i), 8) << "\n";
				}
				//std::cout << HD << "\n";
				if (success)
					success_rate += 1;
			}

		} // perm

		// overall stats, for current step size/set of traces considered
		//
		std::cout<<"The following stats are concerning the most probable key candidate across all permutations for this subset of trace";
		std::cout<<"\n";

		std::cout<<" Avg Pearson correlation (across all key bytes): ";
		std::cout << overall_avg_cor / permutations;
		std::cout<<"\n";

		if (!correct_key.empty()) {

			std::cout<<" Success rate: ";
			std::cout<<"(" << success_rate << " / " << permutations << ") = ";
			std::cout << (success_rate / permutations) * 100.0 << " %";
			std::cout<<"\n";

			std::cout<<" Avg Hamming distance: ";
			std::cout << (HD / 128 / permutations) * 100.0 << " %";
			std::cout<<"\n";

			// abort steps if requested by parameter
			if (steps_stop) {

				// but only once success_rate is 100%
				if ((success_rate / permutations) == 1) {

					// finally, steps cannot be aborted when permutations are written out; all possible permutations
					// have to be recorded, since they might be required for other data sets where fewer sets would not
					// suffice to resolve the key
					//
					if (!perm_file_write) {

						std::cout<<std::endl;
						break;
					}
				}
			}
		}
		std::cout<<std::endl;
	}

	if (perm_file_write) {
		perm_file.close();
	}
}
		
