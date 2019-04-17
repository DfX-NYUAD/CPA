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
// This file contains the statistics functions required
// for the CPA algorithms
//

#ifndef SCA_CPA_STATS_H
#define SCA_CPA_STATS_H

#include <vector>

namespace stats
{

// Calculates the mean of a given vector
inline float mean(std::vector<float>& vec, std::vector<unsigned>& indices, int elements)
{
	float sum;
	float mean;

	sum = 0;

	for (int i = 0; i < elements; i++)
		sum += vec[indices[i]];

	mean = sum / elements;

	return mean;
};

// Calculates the pearson product-moment correlation coefficient
// of two equally sized vectors
//
// Only work on the first "elements" indices for both a, b
// For example, a[indices[0]], a[indices[1]], ..., a[indices[elements - 1]]
inline float pearsonr(std::vector<float>& a, std::vector<float>& b, std::vector<unsigned>& indices, int elements)
{
	float a_mean;
	float b_mean;
	float cov;
	float a_dev;
	float b_dev;
	float std_dev_a;
	float std_dev_b;
	float correlation;

	//if (a.size() != b.size())
	//{
	//	std::cerr<<"\n\nstats::pearsonr: input vectors are not same size\n";
	//	return -2;
	//}

	a_mean = mean(a, indices, elements);
	b_mean = mean(b, indices, elements);

	cov = std_dev_a = std_dev_b = 0;

	for (int i = 0; i < elements; i++)
	{
		a_dev = a[indices[i]] - a_mean;
		b_dev = b[indices[i]] - b_mean;
	
		cov += a_dev * b_dev;

		std_dev_a += std::pow(a_dev, 2.0f);
		std_dev_b += std::pow(b_dev, 2.0f);
	}
	cov /= elements;
	std_dev_a /= elements;
	std_dev_b /= elements;

	std_dev_a = std::sqrt(std_dev_a);
	std_dev_b = std::sqrt(std_dev_b);

	correlation = cov / (std_dev_a * std_dev_b);
	
	return correlation;
	
};

} //end of namespace

#endif
