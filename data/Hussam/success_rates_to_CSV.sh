#!/bin/bash

## which files considered; as header for CSV files
#for tech in {0..4}
#do
#	for run in {1..3}
#	do
#		../parse_generic.sh 1000 1000 1 $run ncfet_TFE"$tech" 1 10 "Success rate" 10_keys/fixed_run_"$run"/ 7 yes > Hussam_TFE"$tech"_fixed_run_"$run"_keys_1--10.csv
#	done
#done
#
## actual CSV data
#for tech in {0..4}
#do
#	for run in {1..3}
#	do
#		../parse_generic.sh 1000 1000 1 $run ncfet_TFE"$tech" 1 10 "Success rate" 10_keys/fixed_run_"$run"/ 7 >> Hussam_TFE"$tech"_fixed_run_"$run"_keys_1--10.csv
#	done
#done

# which files considered; as header for CSV files
for key in {1..10}
do
	for run in {1..3}
	do
		../parse_generic.sh 1000 1000 1 $run "ncfet_TFE0 ncfet_TFE1 ncfet_TFE2 ncfet_TFE3 ncfet_TFE4" $key $key "Success rate" 10_keys/fixed_run_"$run"/ 7 yes > Hussam_fixed_run_"$run"_key_"$key".csv
	done
done

# actual CSV data
for key in {1..10}
do
	for run in {1..3}
	do
		../parse_generic.sh 1000 1000 1 $run "ncfet_TFE0 ncfet_TFE1 ncfet_TFE2 ncfet_TFE3 ncfet_TFE4" $key $key "Success rate" 10_keys/fixed_run_"$run"/ 7 >> Hussam_fixed_run_"$run"_key_"$key".csv
	done
done
