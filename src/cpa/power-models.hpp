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
// This file contains the power model functions needed
// for the CPA algorithms
//

#ifndef SCA_CPA_POWER_MODELS_H
#define SCA_CPA_POWER_MODELS_H

namespace pm
{

// Calculates the Hamming distance between two
// values and is truncated to a specified number
// of bits
inline int Hamming_dist(int num1, int num2, int bits = 8)
{
        int x;
	int ham_dist;

	x = (num1 ^ num2) & ((1 << bits) - 1);
        ham_dist = 0;

        while (x)
        {
                ham_dist += x & 1;
                x >>= 1;
        }

        return ham_dist;
}

inline int Hamming_weight(unsigned char byte_) {

	std::bitset<8> byte = std::bitset<8>(byte_);

	//// NOTE only the inverse HW model works, i.e., bit 0 considered as HW 1 and bit 1 marked as HW 0 -- possibly some of the AES operations introduces this inversion issue,
	//// which didn't came to light before when using the HD model...
	// return (1 * (8 - byte.count())) + (0 * (byte.count()));
	return (8 - byte.count());

	//// NOTE an example for a more accurate power model -- did not make noticable difference in correlation values for 1st and 2nd candidates --> not needed
	// return (0.02561 * (byte.count())) + (0.03217 * (8 - byte.count()));
}

inline float power(unsigned char pre_byte_, unsigned char post_byte_, unsigned int byte_id, std::unordered_multimap< unsigned int, cpa::power_table_FF > const& power_model, bool clk_high) {

	std::bitset<8> pre_byte = std::bitset<8>(pre_byte_);
	std::bitset<8> post_byte = std::bitset<8>(post_byte_);
	float ret = 0;

	// sum up power values for all 8 bits of byte of interest
	for (unsigned int i = 0; i < 8; i++) {

		auto key_range = power_model.equal_range((byte_id * 8) + i);

		for (auto iter = key_range.first; iter != key_range.second; ++iter) {

			auto const& tab = iter->second;

			// only consider the entry in the power model where data conditions match
			//
			if (tab.CP == clk_high && tab.D == pre_byte[i] && tab.Q == post_byte[i]) {

				ret += tab.value;
				break;
			}
		}
	}

	return ret;
}

}//end namespace

#endif
