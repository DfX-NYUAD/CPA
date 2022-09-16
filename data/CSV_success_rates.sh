#!/bin/bash

if [ $# -lt 7 ]; then
       echo "Parameters required:"
       echo "1) List of technologies, e.g., \"TFE0 TFE1 TFE2 TFE3 TFE4\""
       echo "2) Run ID"
       echo "3) Main folder comprising sub-folders for each run"
       echo "4) Run prefix to match correct sub-folders for each run, e.g., \"fixed_run_\""
       echo "5) Key"
       echo "6) Number of steps"
       echo "7) Number of permutations"
       echo "8) Optional, last key; if provided, all keys from ``4) Key'' upto this one will be handled"
       echo "9) Optional, last run ID; if provided, all runs from ``2) Run ID'' upto this one will be handled"
       exit
fi

tech=$1
start_run=$2
folder=$3
folder_run_prefix=$4
start_key=$5
steps=$6
permutations=$7

# optional ones; assign with default values if not provided by user
stop_key=$8
if [ "$stop_key" = "" ]; then
	stop_key=$start_key
fi
stop_run=$9
if [ "$stop_run" = "" ]; then
	stop_run=$start_run
fi

# which files considered; as header for CSV files
for ((key = start_key; key <= stop_key; key++))
do
	for ((run = start_run; run <= stop_run; run++))
	do
		file=$folder/"$folder_run_prefix"$run/key_"$key"_success_rates.csv

		./parse_generic.sh $steps $permutations 1 $run "$tech" $key $key "Success rate" $folder/"$folder_run_prefix"$run 7 yes > $file
	done
done

# actual CSV data
for ((key = start_key; key <= stop_key; key++))
do
	for ((run = start_run; run <= stop_run; run++))
	do
		file=$folder/"$folder_run_prefix"$run/key_"$key"_success_rates.csv
		echo "Working on $folder/"$folder_run_prefix"$run/key_"$key"_success_rates.csv ..."

		./parse_generic.sh $steps $permutations 1 $run "$tech" $key $key "Success rate" $folder/"$folder_run_prefix"$run 7 >> $file
	done
done
