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
       echo "8) Optional, number of successful runs; the result will be w.r.t.\ the very 1st match for that number, not for the very last iteration
       			you can use placeholder here, e.g., 9[0-9][0-9] will consider all numbers from 900--999"
       exit
fi

steps=$1
permutations=$2
steps_start=$3
run=$4
tech_list=$5
keys=$6
folder=$7
successful_runs=$8

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

		# in case the related run has failed or is still ongoing (the keyword for finished runs is "Overall runtime"), then generate
		# an empty data point
		if [ ! -f $file ]; then
			echo -n "	"
		elif [ `grep 'Overall runtime' $file | wc -l` == 0 ]; then
			echo -n "	"
		# otherwise, the related run is done
		#
		# in case the optional parameter successful_runs is given, consider only that related data set, running from "Success rate:
		# ($successful_runs" in reverse order, over at most 264 lines(?), up to "Working on step"; then consider only the last occurrence,
		# representing the stable state for that particular success rate, and print the numbers for that particular step, retrieved from the line
		# right after
		elif [ "$successful_runs" != "" ]; then
			# first, get the line of the 1st match for the success rate
			line=`grep -n "Success rate: ($successful_runs /" $file | cut -d ":" -f 1 | head -n 1`
			# in case no match is found, use reset value of ``0''
			if [ "$line" = "" ]; then
				line=0
			fi
			# second, working in reverser order on the range from $line--1, grep for the first occurrence of "Working on step";
			# print the number of traces for that step, retrieved from the line right before
			echo -n `sed -n 1,"$line"p $file | tac | grep "Working on step" -B 1 | head -n 1 | awk '{print $1}' | cut -c 2-`"	"
		# in case the optional parameter successful_runs is not given, parse for the last occurrence of "Working on step"; print the
		# number of traces for that last step, retrieved from the line right after
		else
			echo -n `grep "Working on step" -A 1 $file | tail -1 | awk '{print $1}' | cut -c 2-`"	"
		fi
	done
	echo
done
