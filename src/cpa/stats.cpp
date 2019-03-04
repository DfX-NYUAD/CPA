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

#include <cmath>
#include <vector>
#include <iostream>

#include "stats.hpp"

float stats::max_in_range(std::vector<float>& vec, unsigned int start, unsigned int range)
{
	float max_pt;

	max_pt = vec.at(start);

	for (unsigned int i = start; i < (start + range); i++)
	{
		if (max_pt < vec.at(i))
			max_pt = vec.at(i);
	} 

	return max_pt;
}

float stats::mean(std::vector<float>& vec)
{
	float sum;
	float mean;
	int n;

	n = vec.size();
	sum = 0;

	for (int i = 0; i < n; i++)
		sum += vec.at(i);

	mean = sum / vec.size();

	return mean;
}

float stats::sum_of_squares(std::vector<float>& vec)
{
	
	float sum;
	int n;

	n = vec.size();
	sum = 0;

	for (int i = 0; i < n; i++)
		sum += (vec.at(i) * vec.at(i));

	return sum;
}

float stats::pearsonr(std::vector<float>& a, std::vector<float>& b)
{
	float a_mean;
	float b_mean;
	float num;
	float den;	
	float result;
	int n;

	if (a.size() != b.size())
	{
		std::cerr<<"\n\nstats::pearsonr: input vectors are not same size\n";
		return -2;
	}

	std::vector<float> am(a.size());
	std::vector<float> bm(b.size());

	a_mean = mean(a);
	b_mean = mean(b);

	n = a.size();
	num = 0;

	for (int i = 0; i < n; i++)
	{
		am.at(i) = a.at(i) - a_mean;
		bm.at(i) = b.at(i) - b_mean;
	
		num += am.at(i) * bm.at(i);
	}

	num *= n;
	den = n * (sqrt ( sum_of_squares(am) * sum_of_squares(bm) ) );

	result = num / den;
	result = fmax(fmin(result, 1.0f), -1.0f);
	
	return result;
	
}

float stats::var(std::vector<float>& vec)
{
	float sum = 0;
	float vec_mean = mean(vec);
	int n = vec.size();
	
	for (int i = 0; i < n; i++)
	{
		sum += (vec.at(i) - vec_mean)*(vec.at(i) - vec_mean);
	}

	float variance = sum / (vec.size() - 1);
	
	return variance;
}

