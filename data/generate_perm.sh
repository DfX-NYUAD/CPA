#!/bin/bash

if [ $# -lt 4 ]; then
       echo "Parameters required:"
       echo "1) Number of steps"
       echo "2) Number of traces"
       echo "3) Number of traces per step"
       echo "4) Number of permutations"
       exit
fi

# mandatory parameters
steps=$1
traces=$2
traces_per_step=$3
permutations=$4

# derive other parameters
#
## max trace index
trace_max_index=$((traces - 1))
#
## permutations file has to start from 1; in case sca is started with -steps_start, the earlier steps are dropped from parsing, but they are
## still expected to be in the permutations file
steps_start=1
#
## /dev/urandom should suffice, and it also doesn't block like /dev/random
## https://www.gnu.org/software/coreutils/manual/html_node/Random-sources.html
random_src="/dev/urandom"

# derive file name
perm_file="perm_"$traces"_traces_"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start.txt"

echo "Generate permutations file: $perm_file"
echo " steps=$steps"
echo " traces=$traces"
echo " traces_per_step=$traces_per_step"
echo " permutations=$permutations"
echo " trace_max_index=$trace_max_index"
echo " steps_start=$steps_start"
echo " random_src=$random_src"
echo ""

# backup and rm old file, if any
if [ -f $perm_file ]; then
	mv $perm_file $perm_file".back_`date +%s`"
fi

# outer loop, steps
for ((step = steps_start; step <= steps; step++))
do
	data_pts=$((step * traces_per_step))

	echo "Step $step; generating $data_pts data pts for each permutation ..."

	echo "STEP_START" >> $perm_file

	# inner loop, permutations
	for ((perm = 1; perm <= permutations; perm++))
	do
		echo "PERM_START" >> $perm_file

		# main command, to generate permutations
		#
		## -i: select random number from that range, including both min and max
		## -n: the count of random numbers
		## xargs is to bring all numbers into one line; shuf outputs multiple lines by default
		echo `shuf -i 0-$trace_max_index -n $data_pts --random-source=$random_src | xargs` >> $perm_file
	done
done
