#!/bin/bash

if [ $# -lt 1 ]; then
       echo "Parameters required:"
       echo "1) List of technologies, as one parameter; e.g., \"7 32 45 SDF\""
       exit
fi

# copy the data from dfx
for tech in $1
do
	# SDF results, only for 65nm for now; different folders
	if [ "$tech" == "SDF" ]; then

		mkdir SDF

		tech=65

		scp dfx:/home/projects/power_sim/aes_regression/gls_sdf/vcs_"$tech"nm_5000_10/"$tech"nm_5000_*.txt SDF/
		scp dfx:/home/projects/power_sim/aes_regression/gls_sdf/aes128_table_ecb_"$tech"nm/"$tech"nm_5000_power_*.txt SDF/

	# other pre-layout results
	else
		scp dfx:/home/projects/power_sim/aes_regression/gls/vcs_"$tech"nm_5000_10/"$tech"nm_5000_*.txt .
		scp dfx:/home/projects/power_sim/aes_regression/ptpx/aes128_table_ecb_"$tech"nm/"$tech"nm_5000_power_*.txt .
	fi
done

# check whether all ciphertext files are the same
for key in {1..10}
do
	diff -q --from-file `ls *nm_5000_"$key".txt`
#diff -q --from-file `ls *nm_5000_"$key".txt SDF/*nm_5000_"$key".txt`

	# some files differ
	if [ $? -eq 1 ]
	then
		# cleanup
		for tech in $1
		do
			if [ "$tech" == "SDF" ]; then
				rm SDF/ -r
			else
				rm "$tech"nm_* -r
			fi
		done

		# exit
		exit
	fi
done

# derive the uniform ciphertext files
for key in {1..10}
do
	cp 7nm_5000_"$key".txt cipher_text_aes_5k_key_"$key".txt
done

# extract the individual keys from one keyfile
for key in {1..10}
do
	echo `head -n $key 7nm_5000_10_key.txt | tail -n 1` > correct_key_aes_5k_key_"$key".txt
done

# cp the power files
for key in {1..10}
do
	for tech in $1
	do
		if [ "$tech" == "SDF" ]; then

			tech=65

			cp SDF/"$tech"nm_5000_power_"$key".txt power_traces_aes_5k_"$tech"nm_SDF_key_"$key".txt
		else
			cp "$tech"nm_5000_power_"$key".txt power_traces_aes_5k_"$tech"nm_key_"$key".txt
		fi
	done
done

# zip the original files
for tech in $1
do
	if [ "$tech" == "SDF" ]; then
		zip orig.zip SDF/ -r
	else
		zip orig.zip "$tech"nm_* SDF/ -r
	fi
done

# rm the original files
for tech in $1
do
	if [ "$tech" == "SDF" ]; then
		rm SDF/ -r
	else
		rm "$tech"nm_* -r
	fi
done
