#!/bin/bash

# copy the data from dfx
for tech in NCFET baseline
do
	mkdir "$tech"
	scp dfx:/home/projects/power_sim/aes_regression/hussam_results/"$tech"/TFE*_txt/7nm_5000_*.txt "$tech"/
	scp dfx:/home/projects/power_sim/aes_regression/hussam_results/"$tech"/7nm_5000_power_*.txt "$tech"/
done

# check whether all ciphertext files are the same
for key in {1..10}
do
	diff -q --from-file `ls */7nm_5000_"$key".txt 2>/dev/null`

	# some files differ
	if [ $? -eq 1 ]
	then
		# cleanup
		for tech in NCFET baseline
		do
			rm "$tech"/ -ri
		done

		# exit
		exit
	fi
done

# derive the uniform ciphertext files
for key in {1..10}
do
	cp NCFET/7nm_5000_"$key".txt cipher_text_aes_5k_key_"$key".txt
done

# extract the individual keys from one keyfile
for key in {1..10}
do
	echo `head -n $key NCFET/7nm_5000_10_key.txt | tail -n 1` > correct_key_aes_5k_key_"$key".txt
done

# cp the power files
for key in {1..10}
do
	for tech in NCFET baseline
	do
		cp "$tech"/7nm_5000_power_"$key".txt power_traces_aes_5k_"$tech"_key_"$key".txt
	done
done

## zip the original files
#for tech in NCFET baseline
#do
#	zip orig.zip "$tech"/ -r
#done

# rm the original files
for tech in NCFET baseline
do
	rm "$tech"/ -ri
done
