#!/bin/bash

if [ $# -lt 3 ]; then
       echo "Parameters required:"
       echo "1) Run ID"
       echo "2) Tech ID"
       echo "3) Key"
       exit
fi

run=$1
tech=$2
key=$3

file=$run"_"$tech"_"$key"_success.csv"

#rm $file

## all key candidates
#for ((key = -255; key <= 0; key++))
#do
#	echo "Parsing correlation values for key candidate $key ..."
### in search string, termination with : is important, otherwise e.g. 25 will match 255, 254, etc
#	../parse_generic.sh 1000 1000 1 $run "aes_ncfet_TFE$tech" $key $key "key candidate $key:" 10_keys/fixed_run_$run/ 6 >> $file
#done
#
## correct key
#echo "Parsing correlation values for correct key ..."
#../parse_generic.sh 1000 1000 1 $run "aes_ncfet_TFE$tech" $key $key "correct round-10 key" 10_keys/fixed_run_$run/ 5 >> $file

#echo "Parsing success rates ..."
#../parse_generic.sh 1000 1000 1 $run "aes_ncfet_TFE$tech" $key $key "Success rate" 10_keys/fixed_run_$run/ 7 >> $file

for ((tech = 0; tech <= 4; tech++))
do
	echo "Parsing success rates ..."
	../parse_generic.sh 1000 1000 1 $run "aes_ncfet_TFE$tech" $key $key "Success rate" 10_keys/fixed_run_$run/ 7 >> $file
done
