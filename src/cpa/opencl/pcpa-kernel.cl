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

__kernel
void pcpa(__global unsigned char* ciphertext, __global float* power_pts,
		__global unsigned char* hamming_pts, unsigned int num_pts, 
		unsigned int num_traces, __global unsigned char* inv_sbox, __local unsigned char* cipher)
{
	
	int tid = get_local_id(0);
	int bid = get_group_id(0);
	
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

	
	cipher[(tid%4) * 4 + (tid/4)] = ciphertext[(bid * num_bytes) + (tid/4) * 4 + (tid%4)];

	barrier(CLK_LOCAL_MEM_FENCE);


	post_row = tid / 4;
	post_col = tid % 4;

	post_byte = cipher[post_row * 4 + post_col];

	for (int k = 0; k < num_keys; k++)
	{

		key_byte = (unsigned char)k;

		//aes::shift_rows(post_row, post_col, pre_row, pre_col);
		pre_row = post_row;
		pre_col = post_col - post_row;
		if (pre_col < 0)
			pre_col += 4;

		pre_byte = cipher[pre_row * 4 + pre_col];

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



__kernel
void pcpa_reduce(__global float* data, __global float* power_pts,
                unsigned int trace_start, unsigned int num_pts,
                unsigned int num_traces, __local float* sdata)
{

        int tid = get_local_id(0);
        int bid = get_group_id(0);
        int size = get_local_size(0);


        sdata[tid] = -1.0f * data[bid * num_pts + tid + trace_start];
        barrier(CLK_LOCAL_MEM_FENCE);

        for (unsigned int s = size / 2; s > 0; s >>=1)
        {
                if (tid < s)
                {
                        if (sdata[tid] < sdata[tid + s]) sdata[tid] = sdata[tid + s];
                }
                barrier(CLK_LOCAL_MEM_FENCE);
        }


        if(tid == 0)
        {
                if (!(num_pts%2))
                        if (sdata[0] < sdata[num_pts - 1])
                                sdata[0] = sdata[num_pts - 1];

                power_pts[bid] = sdata[0];
        }
}


