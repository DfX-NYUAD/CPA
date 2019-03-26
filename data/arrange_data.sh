#!/bin/bash

if [ $# -lt 4 ]; then
       echo "Parameters required:"
       echo "1) List of technologies, as one parameter; e.g., \"7nm 32nm 45nm SDF\""
       echo "2) Number of keys"
       echo "3) Name of cipher, e.g,. aes_regression"
       echo "4) Number of traces"
       echo "5) Optional; name of cipher implementation, e.g., aes128_table_ecb"
       exit
fi

tech_list=$1
keys=$2
cipher=$3
traces=$4

# default values for optional parameters
cipher_impl="aes128_table_ecb"

# reading in values for optional parameters, if given
if [ "$5" != "" ]; then
	cipher_impl=$5
fi

# copy the data from dfx
for tech in $tech_list
do
	# SDF results, only for 65nm for now; different folders
	if [ "$tech" == "SDF" ]; then

		mkdir SDF

		tech="65nm"

		echo "scp dfx:/home/projects/power_sim/"$cipher"/gls_sdf/vcs_"$tech"_"$traces"_"$keys"/"$tech"_"$traces"_*.txt SDF/"
		scp dfx:/home/projects/power_sim/"$cipher"/gls_sdf/vcs_"$tech"_"$traces"_"$keys"/"$tech"_"$traces"_*.txt SDF/
		echo "scp dfx:/home/projects/power_sim/"$cipher"/gls_sdf/"$cipher_impl"_"$tech"/"$tech"_"$traces"_power_*.txt SDF/"
		scp dfx:/home/projects/power_sim/"$cipher"/gls_sdf/"$cipher_impl"_"$tech"/"$tech"_"$traces"_power_*.txt SDF/

	# other pre-layout results
	else
		echo "scp dfx:/home/projects/power_sim/"$cipher"/gls/vcs_"$tech"_"$traces"_"$keys"/"$tech"_"$traces"_*.txt ."
		scp dfx:/home/projects/power_sim/"$cipher"/gls/vcs_"$tech"_"$traces"_"$keys"/"$tech"_"$traces"_*.txt .
		echo "scp dfx:/home/projects/power_sim/"$cipher"/ptpx/"$cipher_impl"_"$tech"/"$tech"_"$traces"_power_*.txt ."
		scp dfx:/home/projects/power_sim/"$cipher"/ptpx/"$cipher_impl"_"$tech"/"$tech"_"$traces"_power_*.txt .
	fi
done

# check whether all ciphertext files are the same
for ((key = 1; key <= keys; key++))
do
	diff -q --from-file `ls *_"$traces"_"$key".txt 2>/dev/null`
	#diff -q --from-file `ls *_"$traces"_"$key".txt SDF/*_"$traces"_"$key".txt 2>/dev/null`

	# some files differ
	if [ $? -eq 1 ]
	then
		# cleanup
		for tech in $tech_list
		do
			if [ "$tech" == "SDF" ]; then
				rm SDF/ -r
			else
				rm "$tech"* -r
			fi
		done

		# exit
		exit
	fi
done

# derive the uniform ciphertext files
for ((key = 1; key <= keys; key++))
do
	first=`echo $tech_list | awk '{print $1}'`
	cp $first"_"$traces"_"$key".txt" "cipher_text_"$cipher"_"$traces"_key_"$key".txt"
done

# extract the individual keys from one keyfile
for ((key = 1; key <= keys; key++))
do
	first=`echo $tech_list | awk '{print $1}'`
	echo `head -n $key $first"_"$traces"_"$keys"_key.txt" | tail -n 1` > correct_key_"$cipher"_"$traces"_key_"$key".txt
done

# cp the power files
for ((key = 1; key <= keys; key++))
do
	for tech in $tech_list
	do
		if [ "$tech" == "SDF" ]; then

			tech="65nm"

			cp SDF/"$tech"_"$traces"_power_"$key".txt power_traces_"$cipher"_"$traces"_"$tech"_SDF_key_"$key".txt
		else
			cp "$tech"_"$traces"_power_"$key".txt power_traces_"$cipher"_"$traces"_"$tech"_key_"$key".txt
		fi
	done
done

## zip the original files
#for tech in $tech_list
#do
#	if [ "$tech" == "SDF" ]; then
#		zip orig.zip SDF/ -r
#	else
#		zip orig.zip "$tech"*
#	fi
#done

# rm the original files
for tech in $tech_list
do
	if [ "$tech" == "SDF" ]; then
		rm SDF/ -r
	else
		rm "$tech"* -r
	fi
done
