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

#include "pcpa-reduction.hpp"

__global__
void pcpa_reduce(float* data, float* power_pts, unsigned int trace_start, unsigned int num_pts, unsigned int num_traces)
{
	
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	int size = blockDim.x;
	
	extern __shared__ float sdata[];
	
	
	sdata[tid] = -1.0f * data[bid * num_pts + tid + trace_start];
	__syncthreads();


	for (unsigned int s = size / 2; s > 0; s >>=1)
	{
		if (tid < s)
		{
			if (sdata[tid] < sdata[tid + s]) sdata[tid] = sdata[tid + s];
		}
		__syncthreads();
	}


	if(tid == 0) 
	{
		if (!(num_pts%2))
			if (sdata[0] < sdata[num_pts - 1])
				sdata[0] = sdata[num_pts - 1];		
 	
		power_pts[bid] = sdata[0];
	}
}


void pcpa_reduce_wrapper(float *data, float* power_pts, unsigned int trace_start, unsigned int num_pts, unsigned int num_traces)
{
	unsigned int sdata_size = num_pts - trace_start;

	pcpa_reduce<<<num_traces, sdata_size, sdata_size * sizeof(float)>>>(  data,
									power_pts,
									trace_start,
									num_pts,
									num_traces);
}

