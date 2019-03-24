#!/bin/bash

if [ $# -lt 6 ]; then
       echo "Parameters required:"
       echo "1) Number of steps for exploring subsets of traces?"
       echo "2) Number of permutations to consider for each subset of traces?"
       echo "3) Start step?"
       echo "4) Run ID?"
       echo "5) List of technologies, as one parameter; e.g., \"7nm 32nm 45nm SDF NCFET baseline\""
       echo "5) Number of keys?"
       echo "7) Optional; stop step, e.g., 50"
       echo "8) Optional; perm file, e.g., aes_5k__100_steps_10000_perm_1_steps_start.perm"
       echo "9) Optional; rate stop, e.g., 99.99"
       exit
fi

# mandatory parameters
steps=$1
permutations=$2
steps_start=$3
run=$4
tech_list=$5
keys=$6

# optional parameters
steps_stop=$7
perm_file=$8
rate_stop=$9

# prepare optional parameters, if provided
if [ "$steps_stop" != "" ]; then
	steps_stop="-steps_stop $steps_stop"
fi
if [ "$perm_file" != "" ]; then
	perm_file="-perm_file $perm_file"
fi
if [ "$rate_stop" != "" ]; then
	rate_stop="-rate_stop $rate_stop"
fi

rm sbatch.sh

for ((key = 1; key <= keys; key++))
do
	for tech in $tech_list
	do
		run_generic="aes_5k_key_"$key
		if [ "$tech" == "SDF" ]; then
			tech="65nm"
			run_tech="aes_5k_"$tech"_SDF_key_"$key
		else
			run_tech="aes_5k_"$tech"_key_"$key
		fi

		run_=$run_tech"__"$steps"_steps_"$permutations"_perm_"$steps_start"_steps_start__run_"$run
		script=$run_".slurm"

		echo "Generate $script ..."

		cp slurm.template $script

		sed_string="s,power_traces=TODO,power_traces=power_traces_"$run_tech".txt,g"
		sed -i "$sed_string" $script

		sed_string="s,cipher_text=TODO,cipher_text=cipher_text_"$run_generic".txt,g"
		sed -i "$sed_string" $script

		sed_string="s,correct_key=TODO,correct_key=correct_key_"$run_generic".txt,g"
		sed -i "$sed_string" $script

		sed_string="s,steps=TODO,steps="$steps",g"
		sed -i "$sed_string" $script

		sed_string="s,permutations=TODO,permutations="$permutations",g"
		sed -i "$sed_string" $script

		sed_string="s,steps_start=TODO,steps_start="$steps_start",g"
		sed -i "$sed_string" $script

		sed_string="s,steps_stop=TODO,steps_stop=\""$steps_stop\"",g"
		sed -i "$sed_string" $script

		sed_string="s,rate_stop=TODO,rate_stop=\""$rate_stop\"",g"
		sed -i "$sed_string" $script

		sed_string="s,perm_file=TODO,perm_file=\""$perm_file\"",g"
		sed -i "$sed_string" $script

		sed_string="s,SBATCH -o TODO,SBATCH -o "$run_".log,g"
		sed -i "$sed_string" $script

		sed_string="s,SBATCH -e TODO,SBATCH -e "$run_".err,g"
		sed -i "$sed_string" $script

		echo "sbatch $script" >> sbatch.sh
	done
done
