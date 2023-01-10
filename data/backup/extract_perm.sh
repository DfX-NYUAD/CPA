#!/bin/bash

if [ $# -lt 4 ]; then
       echo "Parameters required:"
       echo "1) Permutation file to extract permutation subsets from"
       echo "2) Number of permutations present in the permutations file"
       echo "3) Number of permutations to consider for each step, starting from the top"
       echo "4) Number of steps to consider, starting from the top"
       exit
fi

perm_file=$1
perm_file_extracted=$perm_file".extracted"
permutations_max=$2
permutations=$3
steps=$4

echo "Extracting selected permutations from $perm_file to $perm_file_extracted ..."

sed_string=""

for ((step = 1; step <= steps; step++))
do
#	echo " Preparing to extract the first $permutations permutations for step $step ..."

	start_line=$(((step - 1) * 2 * permutations_max + step))
	stop_line=$((start_line + 2 * permutations))

#	echo "  (Considering lines $start_line--$stop_line)"

	sed_string=$sed_string$start_line","$stop_line"p;"
done
last_line=$((stop_line + 1))
sed_string=$sed_string$last_line"q"

sed -n "$sed_string;" $perm_file > $perm_file_extracted

echo "Done!"
