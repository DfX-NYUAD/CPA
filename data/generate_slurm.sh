#!/bin/bash

if [ $# -lt 8 ]; then
       echo "Parameters required:"
       echo "1) Number of steps for exploring subsets of traces?"
       echo "2) Number of permutations to consider for each subset of traces?"
       echo "3) Start step?"
       echo "4) Run ID?"
       echo "5) List of fully specified technologies, as one parameter; e.g., \"7nm 32nm 45nm 65nm_SDF NCFET baseline\""
       echo "6) Number of keys?"
       echo "7) Name of cipher, e.g,. aes"
       echo "8) Number of traces"
       echo "9) Optional; fully specified commands, as one parameter; \"-steps_stop 100 -rate_stop 99.9\""
       exit
fi

# mandatory parameters
steps=$1
permutations=$2
steps_start=$3
run=$4
tech_list=$5
keys=$6
cipher=$7
traces=$8

# optional parameters
commands=$9

rm sbatch.sh

for ((key = 1; key <= keys; key++))
do
	for tech in $tech_list
	do
		run_generic=$cipher"_"$traces"_key_"$key
		run_tech=$cipher"_"$traces"_"$tech"_key_"$key
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

		sed_string="s,steps_start=TODO,steps_start="$steps_start",g"
		sed -i "$sed_string" $script

		sed_string="s,permutations=TODO,permutations="$permutations",g"
		sed -i "$sed_string" $script

		sed_string="s,commands=TODO,commands=\""$commands\"",g"
		sed -i "$sed_string" $script

		sed_string="s,SBATCH -o TODO,SBATCH -o "$run_".log,g"
		sed -i "$sed_string" $script

		sed_string="s,SBATCH -e TODO,SBATCH -e "$run_".err,g"
		sed -i "$sed_string" $script

		echo "sbatch $script" >> sbatch.sh
	done
done
