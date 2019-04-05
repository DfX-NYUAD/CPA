#!/bin/bash

if [ $# -lt 7 ]; then
       echo "Parameters required:"
       echo "1) Number of steps"
       echo "2) Number of permutations"
       echo "3) Start step"
       echo "4) Run ID"
       echo "5) List of fully specified technologies, as one parameter; e.g., \"7nm 32nm 45nm 65nm_SDF NCFET baseline\""
       echo "6) Number of keys"
       echo "7) Folder containing the log files"
       exit
fi

steps=$1
permutations=$2
steps_start=$3
run=$4
tech_list=$5
keys=$6
folder=$7

for tech in $tech_list
do
	for ((key = 1; key <= keys; key++))
	do
		# only for listing files; copied from below
		file=$folder/"*"$tech"_key_"$key"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run".log"

		ls -l $file
	done
done

for tech in $tech_list
do
	for ((key = 1; key <= keys; key++))
	do
		file=$folder/"*"$tech"_key_"$key"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run".log"

		# in case the related run has failed or is still ongoing (the keyword for finished runs is "Overall runtim"), then generate
# an empty data point
		if [ ! -f $file ]; then
			echo -n "	"
		elif [ `grep 'Overall runtime' $file | wc -l` == 0 ]; then
			echo -n "	"
# in case the relate run is done, parse for the last occurrence of "Working"; actually, the line after that last match; and print the number
# of traces for that last step 
		else
			echo -n `grep "Working" -A 1 $file | tail -1 | awk '{print $1}' | cut -c 2-`"	"
		fi
	done
	echo
done
