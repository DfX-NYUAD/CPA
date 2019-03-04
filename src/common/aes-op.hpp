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
// This file contains all the AES-128 based operations that are used 
// throughout the Side Channel Analysis algorithms
//

#ifndef SCA_CPA_AES_OP_H
#define SCA_CPA_AES_OP_H

#include <vector>

#define SBOX_SIZE 256

namespace aes
{

/* Lookup Tables for AES 128 */
extern unsigned char sbox[SBOX_SIZE];
extern unsigned char inv_sbox[SBOX_SIZE];
extern unsigned char rcon[SBOX_SIZE];

/* AES Encryption Operations */
unsigned char sub_bytes(unsigned char value);
void shift_rows(int old_row, int old_col, int& new_row, int& new_col);
unsigned char add_round_key(unsigned char key, unsigned char value);

/* For key expansion, key_in must be a 16 element array and key_out must be a 11x16 element array */
void key_expand(std::vector<unsigned char>& key_in, std::vector< std::vector<unsigned char> >& key_out);


/* AES Decryption Operations */
unsigned char inv_sub_bytes(unsigned char value);
void inv_shift_rows(int old_row, int old_col, int& new_row, int& new_col);

/* For inverse key expansion, key_out and key_in must be a 16 element array */
void inv_key_expand(std::vector<unsigned char>& key_out, std::vector<unsigned char>& key_in);

} //end namespace

#endif
