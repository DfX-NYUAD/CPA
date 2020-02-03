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
inline int Hamming_dist(int num1, int num2, int bits)
{
        int x;
	int ham_dist;

	x = (num1 ^ num2) & ((1 << bits) - 1);
        ham_dist = 0;

	//std::cout << "num1: " << num1 << "; 0b";
	//for (int i = 1 << (bits - 1); i > 0; i = i / 2) 
	//	(num1 & i)? printf("1"): printf("0");
	//std::cout << std::endl;

	//std::cout << "num2: " << num2 << "; 0b";
	//for (int i = 1 << (bits - 1); i > 0; i = i / 2) 
	//	(num2 & i)? printf("1"): printf("0");
	//std::cout << std::endl;

        while (x)
        {
                ham_dist += x & 1;
                x >>= 1;
        }

	//std::cout << "HD: " << ham_dist << std::endl;

        return ham_dist;
}

}//end namespace

#endif
