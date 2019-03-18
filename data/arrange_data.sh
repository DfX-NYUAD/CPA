#!/bin/bash

# copy the data from dfx
for tech in 7 32 55 65 90
do
	scp dfx:/home/projects/power_sim/aes_regression/gls/vcs_"$tech"nm_5000_10/"$tech"nm_5000_*.txt .
	scp dfx:/home/projects/power_sim/aes_regression/ptpx/aes128_table_ecb_"$tech"nm/"$tech"nm_5000_power_*.txt .
done

# check whether all ciphertext files are the same
for i in {1..10}
do
	diff -q --from-file `ls *nm_5000_"$i".txt`
done

# derive the uniform ciphertext files
for i in {1..10}
do
	cp 7nm_5000_"$i".txt cipher_text_aes_5k_key_"$i".txt
done

# extract the individual keys from one keyfile
for i in {1..10}
do
	echo `head -n $i 7nm_5000_10_key.txt | tail -n 1` > correct_key_aes_5k_key_"$i".txt
done

# cp the power files
for i in {1..10}
do
	for tech in 7 32 55 65 90
	do
		cp "$tech"nm_5000_power_"$i".txt power_traces_aes_5k_"$tech"nm_key_"$i".txt
	done
done

# zip the original files
for tech in 7 32 55 65 90
do
	zip orig.zip "$tech"nm_*
done

# rm the original files
for tech in 7 32 55 65 90
do
	rm "$tech"nm_*
done
