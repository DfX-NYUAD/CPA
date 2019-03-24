#!/bin/bash

if [ $# -lt 7 ]; then
       echo "Parameters required:"
       echo "1) Number of steps"
       echo "2) Number of permutations"
       echo "3) Start step"
       echo "4) Run ID"
       echo "5) List of fully specified technologies, as one parameter; e.g., \"7nm 32nm 45nm 65nm_SDF NCFET baseline\""
       echo "6) Number of keys"
       echo "7) Tail lines of log files to consider"
       exit
fi

steps=$1
permutations=$2
steps_start=$3
run=$4
tech_list=$5
keys=$6
lines=$7

for tech in $tech_list
do
	for ((key = 1; key <= keys; key++))
	do
		file="*"$tech"_key_"$key"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run".log"

		ls -l $file
	done
done

for tech in $tech_list
do
	for ((key = 1; key <= keys; key++))
	do
		file="*"$tech"_key_"$key"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run".log"

		echo -n `tail -n $lines $file | head -n 1 | awk '{print $1}' | cut -c 2-`"	"
	done
	echo
done
