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


void cpa::cpa(std::string data_path, std::string ct_path, std::string key_path, std::string perm_path, bool candidates, int permutations, int steps, int steps_start, int steps_stop, float rate_stop, bool verbose, bool key_expansion)
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

	unsigned round_key_index = 10;

	// Obtain a time-based seed
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	// Prepare vectors
	std::vector< std::vector<float> > data;
	std::vector< std::vector<unsigned char> > ciphertext;
	std::vector< std::vector<unsigned char> > correct_key;
	std::vector< std::vector<unsigned char> > correct_round_key (11, std::vector<unsigned char> (num_bytes));
	std::vector< std::vector<unsigned char> > cipher (4, std::vector<unsigned char> (4));
	std::vector< std::vector<unsigned> > perm_from_file;

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

		for (unsigned int i = 0; i < num_bytes; i++)
			std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(correct_key[0][i]) << " ";
		std::cout<<std::endl;

		if (key_expansion) {
			std::cout<<" That key is considered to be a full key; key expansion is applied" << std::endl;
		}
		else {
			std::cout<<" That key is considered to be a round-10 key; key expansion is NOT applied" << std::endl;
		}

		// also derive round keys
		//
		// note that correct_round_key[0] contains the initial key
		aes::key_expand(correct_key[0], correct_round_key);

		// now, in case key expansion shall not be considered, the relevant key to look at is correct_round_key[0], the initial key
		if (!key_expansion) {
			round_key_index = 0;
		}

		// log round keys only for key expansion
		if (key_expansion) {
			for (unsigned round = 1; round < 11; round++) {
				std::cout << "  Related round-" << std::dec << round << " key: ";
				for (unsigned int i = 0; i < num_bytes; i++)
					std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(correct_round_key[round][i]) << " ";
				std::cout<<std::endl;
			}
		}
	}

	// Handle the permutations file, if provided
	if (perm_path != "") {

		std::cout<<"Handle permutations file: " << perm_path << std::endl;

		// try to read in permutations from file
		perm_file.open(perm_path.c_str(), std::ifstream::in);

		if (perm_file.is_open()) {
			perm_file_read = true;

			// dummy reading of "step 0", to initialize file parsing
			csv::read_perm_file(perm_file, 0, 0, 0, 0, perm_from_file);

			// dummy reading/dropping of all earlier steps which are not required for current call
			for (int drop = 1; drop < steps_start; drop++) {
				csv::read_perm_file(perm_file, drop, 0, 0, 0, perm_from_file);
			}
		}
		// if reading failed, consider to write out to the perm_file
		else {
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
	std::vector< std::vector< std::vector<unsigned int> > > Hamming_pts__0_1_flips
		( num_bytes, std::vector< std::vector<unsigned int> > 
		(256, std::vector<unsigned int> (num_traces, 0) ) );
	std::vector< std::vector< std::vector<unsigned int> > > Hamming_pts__1_0_flips
		( num_bytes, std::vector< std::vector<unsigned int> > 
		(256, std::vector<unsigned int> (num_traces, 0) ) );
	std::vector< std::vector< std::vector<int> > > Hamming_pts__flips_HD
		( num_bytes, std::vector< std::vector<int> > 
		(256, std::vector<int> (num_traces, 0) ) );

	std::vector<unsigned> trace_indices (num_traces);
	// Prepare with all trace indices; required for shuffling/generating permutations in case they are not read in
	for (unsigned i = 0; i < num_traces; i++) {
		trace_indices[i] = i;
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
	
		power_pts[i] = max_pt;
	}

	std::cout<<"Calculate Hamming distances...\n";
	std::cout<<std::endl;
			
	// Calculate Hamming distances
	for (unsigned int i = 0; i < num_traces; i++)
	{
		// Get cipher for this particular trace
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				cipher[k][j] = ciphertext.at(i).at(j * 4 + k);

		// Find ciphertext bytes at different stages for the Hamming 
		// distance calculation
		for (int j = 0; j < num_bytes; j++)
		{
			// Select byte
			post_row = j / 4;
			post_col = j % 4;
			post_byte = cipher[post_row][post_col];

			// Create all possible bytes that could have resulted
			// selected byte
			for (int k = 0; k < num_keys; k++)
			{
				// Undo AES-128 operations
				key_byte = static_cast<unsigned char> (k);
				aes::shift_rows(post_row, post_col, pre_row, pre_col);
				pre_byte = cipher[pre_row][pre_col];
				pre_byte = aes::add_round_key(key_byte, pre_byte);
				pre_byte = aes::inv_sub_bytes(pre_byte);
				byte_id = pre_col * 4 + pre_row;
			
				// Find the Hamming distance between the bytes
				Hamming_pts[byte_id][k][i] = pm::Hamming_dist(pre_byte, post_byte, 8);

				// also track the 0->1 and 1->0 flips
				Hamming_pts__0_1_flips[byte_id][k][i] = 0;
				Hamming_pts__1_0_flips[byte_id][k][i] = 0;
				for (unsigned char b = 1 << 7; b > 0; b = b / 2)  {
					// 0->1
					if (!(pre_byte & b) && (post_byte & b))
						Hamming_pts__0_1_flips[byte_id][k][i]++;
					// 1->0
					if ((pre_byte & b) && !(post_byte & b))
						Hamming_pts__1_0_flips[byte_id][k][i]++;
				}

				// also track the HD between the 0->1 and 1-> 0 flips; has direct impact on accuracy of simple HD power model !
				// 
				// note that absolute HD is calculated here, not using abs -- this allows to differentiate between bias towards
				// 0->1 or 1->0 flips!
				Hamming_pts__flips_HD[byte_id][k][i] = static_cast<int>(Hamming_pts__0_1_flips[byte_id][k][i] - Hamming_pts__1_0_flips[byte_id][k][i]);

				//// dbg logging
				//std::cout << "pre_byte:  0b";
				//for (int i = 1 << (8 - 1); i > 0; i = i / 2) 
				//	(pre_byte & i)? printf("1"): printf("0");
				//std::cout << std::endl;
				//std::cout << "post_byte: 0b";
				//for (int i = 1 << (8 - 1); i > 0; i = i / 2) 
				//	(post_byte & i)? printf("1"): printf("0");
				//std::cout << std::endl;
				//std::cout << " HD(pre_byte, post_byte, 8): " << Hamming_pts[byte_id][k][i] << std::endl;
				//std::cout << " 0->1 flips: " << Hamming_pts__0_1_flips[byte_id][k][i] << std::endl;
				//std::cout << " 1->0 flips: " << Hamming_pts__1_0_flips[byte_id][k][i] << std::endl;
				//std::cout << " HD between flips: " << std::dec << Hamming_pts__flips_HD[byte_id][k][i] << std::endl;
			}
		}
	}

	// Consider multiple runs, as requested by step_size parameter
	//
	// s has to be float, to properly calculate data_pts
	for (float s = steps_start; s <= steps_stop; s++) {

		int data_pts = num_traces * (s / steps);

		std::cout << "Working on step " << std::dec << s << "...\n";

		float data_pts_perc = data_pts;
		data_pts_perc /= num_traces;
		data_pts_perc *= 100;
		std::cout << "(" << data_pts << " / " << num_traces << ") traces = " << data_pts_perc << " % of all traces\n";
		std::cout<<std::endl;

		float success_rate = 0.0f;
		float key_HD = 0.0f;
		std::vector<float> key_byte_correlation_HD (num_bytes, 0.0f);
		std::vector<unsigned char> round_key (num_bytes);
		std::vector<float> max_correlation (num_bytes, 0.0f);

		float avg_HD = 0.0f;
		float avg_0_1_flips = 0.0f;
		float avg_1_0_flips = 0.0f;
		float avg_flips_HD = 0.0f;

		// track correlation for all 256 key candidates and the correct key separately; correlation for correct key is in
		// avg_correlation[256]
		std::vector<float> avg_correlation (num_keys + 1, 0.0f);

		// init permutations vector from file
		if (perm_file_read) {

			csv::read_perm_file(perm_file, s, data_pts, permutations, num_traces, perm_from_file);
		}

		// Consider multiple runs, as requested by permutations parameter
		for (int perm = 1; perm <= permutations; perm++) {

			// reset Pearson correlation multimaps
			for (int i = 0; i < num_bytes; i++) {
				r_pts[i].clear();
			}

			if (verbose) {
				std::cout<<" Start permutation #" << std::dec << perm << "...\n";
				std::cout<<std::endl;
			}

			// in case pre-defined permutations have been read in, use those
			if (perm_file_read) {

				trace_indices = perm_from_file[perm - 1];
			}
			// otherwise, consider a random one
			else {
				shuffle(trace_indices.begin(), trace_indices.end(), std::default_random_engine(seed));

				// also write out the newly generated, random permutation
				if (perm_file_write) {

					// write out markers for simpler parsing
					if (perm == 1) {
						perm_file << "STEP_START\n";
					}
					perm_file << "PERM_START\n";

					// only write out the first #(data_pts) as needed
					for (int d = 0; d < data_pts; d++) {
						perm_file << trace_indices[d] << " ";
					}
					// one permutation per line (only for readability, not required for parsing)
					perm_file << "\n";
				}
			}

			// Perform Pearson r correlation for this permutation Hamming distance sets and power data

			if (verbose) {
				std::cout<<"  Calculate Pearson correlation...\n";
				std::cout<<std::endl;
			}

			#pragma omp parallel for
			for (int i = 0; i < num_bytes; i++)
			{
				for (int j = 0; j < num_keys; j++)
				{
					// Pearson r correlation with power data
					//
					r_pts[i].emplace( std::make_pair(

								// Note that power_pts and Hamming_pts[i][j] may contain the data for
								// all traces; only the #data_pts data points from trace_indices[0] ...
								// trace_indices[data_pts - 1] will be considered
								stats::pearsonr(power_pts, Hamming_pts[i][j], trace_indices, data_pts),

								// keep track of the related key byte in the multimap; this allows for easy
								// extraction of the key later on
								j
							));
				}
			}

			if (verbose) {
				std::cout<<"  Derive the key candidates...\n";
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

			for (; candidate < num_keys; candidate++) {

				// track max and average correlation
				float avg_cor = 0.0;
				for (int i = 0; i < num_bytes; i++) {

					auto iter = r_pts[i].begin();
					std::advance(iter, candidate);

					round_key[i] = iter->second;
					max_correlation[i] = iter->first;

					avg_cor += max_correlation[i];
				}
				avg_cor /= num_bytes;

				// track avg correlation for all candidates across all permutations
				avg_correlation[candidate] += avg_cor;

				if (verbose) {

					// Report the key
					if (candidates) {
						std::cout<<"  Round-10 key candidate " << std::dec << candidate - num_keys + 1 << " (in hex): ";
					}
					else {
						std::cout<<"  Round-10 key prediction (in hex):  ";
					}
					for (unsigned int i = 0; i < num_bytes; i++)
						std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(round_key[i]) << " ";
					std::cout<<"\n";

					// Report the related correlation values
					std::cout<<"   Related Pearson correlation values are: ";

					for (unsigned int i = 0; i < num_bytes; i++) {
						std::cout << std::dec << max_correlation[i] << " ";
					}
					std::cout<<"\n";

					std::cout<<"    Avg Pearson correlation across all round-10 key bytes: " << avg_cor;
					std::cout<<"\n";

					if (key_expansion) {

						// Reverse the AES key scheduling to retrieve the original key
						std::vector<unsigned char> full_key (num_bytes);
						aes::inv_key_expand(round_key, full_key);

						std::cout<<"   Full key (in hex): ";
						for (unsigned int i = 0; i < num_bytes; i++)
							std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(full_key[i]) << " ";
						std::cout<<"\n";
					}
					std::cout<<std::endl;

				} // verbose
			} // candidate

			// if correct key was provided, evaluate success rate, HD values, and avg correlation for correct-key bytes; also track
			// flips
			//
			if (!correct_key.empty()) {
				bool success = true;

				// track success rate and actual HD for that last candidate; always concerning round-10 key (round_key)
				for (unsigned int i = 0; i < num_bytes; i++) {

					// check whether any byte is off
					if  (round_key[i] != correct_round_key[round_key_index][i]) {
						success = false;
					}

					// calculate the actual HD for the key candidate and the correct key
					key_HD += pm::Hamming_dist(round_key[i], correct_round_key[round_key_index][i], 8);
				}
				if (success)
					success_rate += 1;

				// now, also track the correlation-related HD, i.e., for each round-key byte, track how far away was the
				// correct byte from the picked, most probable one
				//
				float avg_cor = 0.0;
				for (unsigned int i = 0; i < num_bytes; i++) {

					// check for the correct byte among all the candidates, not only the last one
					//
					// reverse order as correct byte is probably more toward the end
					for (int candidate = num_keys - 1; candidate >= 0; candidate--) {

						auto iter = r_pts[i].begin();
						std::advance(iter, candidate);
						round_key[i] = iter->second;

						if  (round_key[i] == correct_round_key[round_key_index][i]) {

							key_byte_correlation_HD[i] += ((num_keys - 1) - candidate);

							//std::cout << "byte: " << i;
							//std::cout << "; ";
							//std::cout << "candidate: " << candidate;
							//std::cout << "; ";
							//std::cout << "HD_candidate: " << ((num_keys - 1) - candidate);
							//std::cout << std::endl;
							
							// also track avg correlation for the correct key, by summing up the byte-level
							// correlation here
							max_correlation[i] = iter->first;
							avg_cor += max_correlation[i];

							// no need to check other candidates once the correct byte is found
							break;
						}
					}
				}

				// derive avg correlation for correct key bytes
				avg_correlation[num_keys] += (avg_cor / num_bytes);

				// track average HD and flips
				//
				float avg_HD_ = 0.0f;
				float avg_0_1_flips_ = 0.0f;
				float avg_1_0_flips_ = 0.0f;
				float avg_flips_HD_ = 0.0f;

				for (int trace = 0; trace < data_pts; trace++) {

					// it's essential to evaluate the HD and flips individually for each trace, and not for all traces at
					// once; so they must be tracked separately
					//
					float avg_HD__ = 0.0f;
					float avg_0_1_flips__ = 0.0f;
					float avg_1_0_flips__ = 0.0f;
					float avg_flips_HD__ = 0.0f;

					for (unsigned int i = 0; i < num_bytes; i++) {

						avg_HD__ += Hamming_pts
							[i]
							[ correct_round_key[round_key_index][i] ]
							[ trace_indices[trace] ];

						avg_0_1_flips__ += Hamming_pts__0_1_flips
							[i]
							[ correct_round_key[round_key_index][i] ]
							[ trace_indices[trace] ];

						avg_1_0_flips__ += Hamming_pts__1_0_flips
							[i]
							[ correct_round_key[round_key_index][i] ]
							[ trace_indices[trace] ];

						avg_flips_HD__ += Hamming_pts__flips_HD
							[i]
							[ correct_round_key[round_key_index][i] ]
							[ trace_indices[trace] ];
					}

					// flips, artificial normalization (should help to limit rounding errors for large number of
					// data_pts/traces)
					avg_0_1_flips_ += (avg_0_1_flips__ / 128);
					avg_1_0_flips_ += (avg_1_0_flips__ / 128);
					// absolute HD, normally not to be normalized
					avg_flips_HD_ += (avg_flips_HD__ / 128);

					// regular HD, normalize over all 128 bits in general
					avg_HD_ += (avg_HD__ / 128);
				}

				// sum up averages normalized over data_pts/traces; will be normalized over permutations later on
				avg_HD += (avg_HD_ / data_pts);
				// also restore artificial normalization
				avg_flips_HD += 128 * (avg_flips_HD_ / data_pts);
				avg_0_1_flips += 128 * (avg_0_1_flips_ / data_pts);
				avg_1_0_flips += 128 * (avg_1_0_flips_ / data_pts);

				if (verbose) {
					std::cout << "  Statistics for text before last round and after last round (the latter is the ciphertext), considering the correct round-key:" << std::endl;

					std::cout << "   Avg Hamming distance for texts for this permutation: " << (avg_HD_ / data_pts) * 100 << " %" << std::endl;

					std::cout << "   Avg 0->1 flips (for all 128 bits of text) for this permutation: " << 128 * (avg_0_1_flips_ / data_pts) << std::endl;
					std::cout << "   Avg 1->0 flips (for all 128 bits of text) for this permutation: " << 128 * (avg_1_0_flips_ / data_pts) << std::endl;

					float std_dev_flips = std::abs(128 * (avg_0_1_flips_ / data_pts) - 128 * (avg_1_0_flips_ / data_pts)) / std::sqrt(2.0);
					std::cout << "    Std dev for the two averages (of 0->1 and 1->0 flips): " << std_dev_flips << std::endl;

					std::cout << "   Avg absolute Hamming distance for flips for this permutation: " << 128 * (avg_flips_HD_ / data_pts) << std::endl;
					std::cout << "    Note that a negative distances means more 1->0 transitions, whereas a positive number means more 0->1 transitions" << std::endl;
				}

			} // correct key

			if (verbose) {
				std::cout<<" Stop permutation #" << std::dec << perm << "...\n";
				std::cout<<std::endl;
			}
		} // perm

		// correlation stats for all candidates
		if (candidates) {
			candidate = 0;
		}
		else {
			candidate = num_keys - 1;
		}

		std::cout<<" Avg Pearson correlations, across all key bytes and permutations\n";

		for (; candidate < num_keys; candidate++) {

			if (candidates) {
				std::cout<<"  For round-10 key candidate " << std::dec << candidate - num_keys + 1;
			}
			else {
				std::cout<<"  For round-10 key prediction";
			}
			std::cout << ": " << avg_correlation[candidate] / permutations;
			std::cout<<"\n";
		}

		// more stats in case correct key was provided
		//
		if (!correct_key.empty()) {

			// correlation stats for correct key
			std::cout << "  For correct round-10 key: ";
			std::cout << avg_correlation[num_keys] / permutations;
			std::cout << "\n";
			std::cout << "\n";

			std::cout << " Success rate: ";
			std::cout << "(" << success_rate << " / " << permutations << ") = ";

			success_rate /= permutations;
			success_rate *= 100.0;

			std::cout << success_rate << " %";
			std::cout << "\n";

			std::cout << " Avg Hamming distance between correct round-key and round-key guess: ";
			std::cout << (key_HD / 128 / permutations) * 100.0 << " %";
			std::cout << "\n";

			std::cout << " Avg correlation-centric Hamming distances between correct round-key bytes and round-key byte guesses:\n";

			float key_byte_correlation_HD_overall = 0;
			for (unsigned int i = 0; i < num_bytes; i++) {

				std::cout << "  Byte " << std::dec << i << ": ";

				// each byte could be off by 255 at max, namely when the least probable candidate was the correct one
				std::cout << (key_byte_correlation_HD[i] / 255 / permutations) * 100.0 << " %";
				if (key_byte_correlation_HD[i] > 0) {
					std::cout << " (translates to being off by " << (key_byte_correlation_HD[i] / permutations) << " candidates)";
				}
				std::cout << "\n";

				// also track overall HD
				key_byte_correlation_HD_overall += key_byte_correlation_HD[i];
			}
			std::cout << "   Avg across all bytes: ";
			std::cout << (key_byte_correlation_HD_overall / (num_bytes * 255) / permutations) * 100.0 << " %";
			if (key_byte_correlation_HD_overall > 0) {
				std::cout << " (translates to being off by " << (key_byte_correlation_HD_overall / num_bytes / permutations) << " candidates)\n";
			}
			else {
				std::cout << "\n";
			}

			std::cout << "\n Statistics for text before last round and after last round (the latter is the ciphertext), considering the correct round-key:" << std::endl;

			// HD; normalized over permutations; in percent
			avg_HD = (avg_HD / permutations) * 100;
			std::cout << "  Avg Hamming distance for texts: ";
			std::cout << avg_HD << " %" << std::endl;

			// average flips (over all 128 bits); normalized over permutations
			avg_0_1_flips /= permutations;
			avg_1_0_flips /= permutations;
			std::cout << "  Avg 0->1 flips (for all 128 bits of text): " << avg_0_1_flips << std::endl;
			std::cout << "  Avg 1->0 flips (for all 128 bits of text): " << avg_1_0_flips << std::endl;

			// std dev for the averages of 0->1 and 1->0 flips
			float std_dev_flips = std::abs(avg_0_1_flips - avg_1_0_flips) / std::sqrt(2.0);
			std::cout << "   Std dev for the two averages (of 0->1 and 1->0 flips): " << std_dev_flips << std::endl;

			// HD; normalized over permutations
			// note that the absolute HD for flips is _not_ in percent
			avg_flips_HD /= permutations;
			std::cout << "  Avg absolute Hamming distance for flips: " << avg_flips_HD << std::endl;
			std::cout << "   Note that a negative distances means more 1->0 transitions, whereas a positive number means more 0->1 transitions" << std::endl;
		}
		// correct key was not provided; report on key prediction
		else {
			std::cout<<" Round-10 key prediction (in hex); just the one from the last permutation considered for that step:  ";
			for (unsigned int i = 0; i < num_bytes; i++)
				std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(round_key[i]) << " ";
			std::cout<<"\n";

			if (key_expansion) {

				// Reverse the AES key scheduling to retrieve the original key
				std::vector<unsigned char> full_key (num_bytes);
				aes::inv_key_expand(round_key, full_key);

				std::cout<<"  Full key (in hex): ";
				for (unsigned int i = 0; i < num_bytes; i++)
					std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(full_key[i]) << " ";
				std::cout<<"\n";
			}
		}

		std::cout << std::endl;

		// in case the correct key and a stop rate is provided, abort steps once that success rate is reached
		//
		if (!correct_key.empty()) {

			if ((rate_stop != -1) && (success_rate >= rate_stop)) {
				break;
			}
		}
	}

	if (perm_file_write) {
		perm_file.close();
	}
}
		
