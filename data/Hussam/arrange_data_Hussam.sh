#!/bin/bash

if [ $# -lt 3 ]; then
       echo "Parameters required:"
       echo "1) Number of keys"
       echo "2) Name of cipher, e.g,. aes"
       echo "3) Number of traces"
       exit
fi

tech_list="NCFET baseline"
keys=$1

# copy the data from dfx
for tech in $tech_list
do
	mkdir "$tech"
	scp dfx:/home/projects/power_sim/"$cipher"_regression/hussam_results/"$tech"/TFE*_txt/7nm_"$traces"_*.txt "$tech"/
	scp dfx:/home/projects/power_sim/"$cipher"_regression/hussam_results/"$tech"/7nm_"$traces"_power_*.txt "$tech"/
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
	cp baseline/7nm_"$traces"_"$key".txt cipher_text_"$cipher"_"$traces"_key_"$key".txt
done

# extract the individual keys from one keyfile
for ((key = 1; key <= keys; key++))
do
	echo `head -n $key "baseline/7nm_"$traces"_"$keys"_key.txt" | tail -n 1` > correct_key_"$cipher"_"$traces"_key_"$key".txt
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
	rm "$tech"/ -ri
done
