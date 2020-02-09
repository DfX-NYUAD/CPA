#!/bin/bash

if [ $# -lt 9 ]; then
       echo "Parameters required:"
       echo "1) Number of steps"
       echo "2) Number of permutations"
       echo "3) Start step"
       echo "4) Run ID"
       echo "5) List of fully specified technologies, as one parameter; e.g., \"7nm 32nm 45nm 65nm_SDF NCFET baseline\""
       echo "6) Number of starting key"
       echo "7) Number of stop key"
       echo "8) Phrase to parse for, as one parameter; e.g., \"Avg across\""
       echo "9) Folder containing the log files"
       echo "10) Optional; Nth word to select from matching lines; will generate CSV output"
       echo "11) Optional; list only matching files in order"
       exit
fi

steps=$1
permutations=$2
steps_start=$3
run=$4
tech_list=$5
key_start=$6
key_stop=$7
keyword=$8
folder=${9}
word_index=${10}
list_files=${11}

if [ "$list_files" != "" ]; then

	for tech in $tech_list
	do
		for ((key = key_start; key <= key_stop; key++))
		do
			# only for listing files; copied from below
			file=$folder/"*"$tech"_key_"$key"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run".log"

			ls -l $file
		done
	done

	exit
fi

for tech in $tech_list
do
	for ((key = key_start; key <= key_stop; key++))
	do
		file=$folder/"*"$tech"_key_"$key"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run".log"

		if [ "$word_index" != "" ]; then

			# reset
			line_string=""

			# build up CSV line
			for word in `grep "$keyword" $file | awk '{print $'$word_index'}'`
			do
				line_string=$line_string""$word", "
			done

			# finally, print out while dropping last ", "
			echo ${line_string::-2}
		else
			cat $file | grep "$keyword"
		fi
	done
done
