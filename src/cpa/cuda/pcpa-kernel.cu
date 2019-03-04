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

#include "pcpa-kernel.hpp"

__global__
void pcpa(unsigned char* ciphertext, float* power_pts, unsigned char* hamming_pts, unsigned int num_pts, unsigned int num_traces, unsigned char* inv_sbox)
{
	
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	
	unsigned char key_byte;
	unsigned char pre_byte;
	unsigned char post_byte;
	unsigned char x;

	int post_row;
	int pre_row;
	int post_col;
	int pre_col;
	int ham_dist;
	int byte_id;

	const int num_bytes = 16;
	const int num_keys = 256;
	

	__shared__ unsigned char cipher[4][4];

	
	cipher[tid%4][tid/4] = ciphertext[(bid * num_bytes) + (tid/4) * 4 + (tid%4)];

	__syncthreads();


	post_row = tid / 4;
	post_col = tid % 4;

	post_byte = cipher[post_row][post_col];

	for (int k = 0; k < num_keys; k++)
	{

		key_byte = (unsigned char)k;

		//aes::shift_rows(post_row, post_col, pre_row, pre_col);
		pre_row = post_row;
		pre_col = post_col - post_row;
		if (pre_col < 0)
			pre_col += 4;

		pre_byte = cipher[pre_row][pre_col];

		//pre_byte = aes::add_round_key(key_byte, pre_byte);
		pre_byte = key_byte ^ pre_byte;

		//pre_byte = aes::inv_sub_bytes(pre_byte);
		pre_byte = inv_sbox[(int)pre_byte];

		byte_id = pre_col * 4 + pre_row;

		//hamming_pts[byte_id][k][i] = pm::hamming_dist(pre_byte, post_byte, 8);
		x = pre_byte ^ post_byte;

		ham_dist = 0;

		while(x)
		{
			ham_dist += x & 1;
			x >>= 1;
		}
		
		hamming_pts[(byte_id * num_keys * num_traces) + (k * num_traces) + bid] = ham_dist;
	}

}
	
void pcpa_wrapper(int gridSize, int blockSize, unsigned char* ciphertext, float* power_pts, unsigned char* hamming_pts, unsigned int num_pts, unsigned int num_traces, unsigned char* inv_sbox)
{
	pcpa<<<gridSize, blockSize>>>(	ciphertext, 
					power_pts, 
					hamming_pts, 
					num_pts, 
					num_traces, 
					inv_sbox);
}	
