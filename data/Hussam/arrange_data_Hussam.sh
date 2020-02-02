#!/bin/bash

#if [ $# -lt 3 ]; then
#       echo "Parameters required:"
#       echo "1) Number of keys"
#       echo "2) Name of cipher, e.g,. aes"
#       echo "3) Number of traces"
#       exit
#fi

baseline="aes_ncfet_TFE4"
tech_list="aes_ncfet_TFE4"
keys=10
cipher="aes_ncfet_12Sept2019"
traces=2000

#baseline="aes_ncfet_TFE0"
#tech_list="aes_ncfet_TFE0 aes_ncfet_TFE1 aes_ncfet_TFE2 aes_ncfet_TFE3 aes_ncfet_TFE4"
#keys=10
#cipher="aes_ncfet_05Sept2019"
#traces=2000

#keys=$1
#cipher=$2
#traces=$3

# copy the data from dfx
for tech in $tech_list
do
	mkdir "$tech"
	scp dfx:/home/projects/power_sim/"$cipher"/"$tech"/ptpx/*/7nm_"$traces"_power_*.txt "$tech"/
	scp dfx:/home/projects/power_sim/"$cipher"/"$tech"/gls/7nm_"$traces"_*.txt "$tech"/
done

# check whether all ciphertext files are the same
for ((key = 1; key <= keys; key++))
do
	diff -q --from-file `ls */7nm_"$traces"_"$key".txt 2>/dev/null`

	# some files differ
	if [ $? -eq 1 ]
	then
		# cleanup
		for tech in $tech_list
		do
			rm "$tech"/ -ri
		done

		# exit
		exit
	fi
done

# derive the uniform ciphertext files
for ((key = 1; key <= keys; key++))
do
	cp $baseline/7nm_"$traces"_"$key".txt cipher_text_"$cipher"_"$traces"_key_"$key".txt
done

# extract the individual keys from one keyfile
for ((key = 1; key <= keys; key++))
do
	echo `head -n $key $baseline"/7nm_"$traces"_"$keys"_key.txt" | tail -n 1` > correct_key_"$cipher"_"$traces"_key_"$key".txt
done

# cp the power files
for ((key = 1; key <= keys; key++))
do
	for tech in $tech_list
	do
		cp "$tech"/7nm_"$traces"_power_"$key".txt power_traces_"$cipher"_"$traces"_"$tech"_key_"$key".txt
	done
done

## zip the original files
#for tech in $tech_list
#do
#	zip orig.zip "$tech"/ -r
#done

# rm the original files
for tech in $tech_list
do
	rm "$tech"/ -r
done
