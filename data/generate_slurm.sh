#!/bin/bash

if [ $# -lt 2 ]; then
       echo "Parameters required:"
       echo "1) Number of steps for exploring subsets of traces?"
       echo "2) Number of permutations to consider for each subset of traces?"
       echo "1) Start step?"
       exit
fi

rm sbatch.sh

#perm_file="-perm_file aes_5k__100_steps_10000_perm_1_steps_start.perm"
perm_file=""

for key in {1..10}
do
	for tech in 7 32 45 55 65 90
	do
		run_generic="aes_5k_key_"$key
		run_tech="aes_5k_"$tech"nm_key_"$key

		run_=$run_tech"__"$1"_steps_"$2"_perm_"$3"_steps_start"
		script=$run_".slurm"

		echo "Generate $script ..."

		cp slurm.template $script

		sed_string="s,power_traces=TODO,power_traces=power_traces_"$run_tech".txt,g"
		sed -i "$sed_string" $script

		sed_string="s,cipher_text=TODO,cipher_text=cipher_text_"$run_generic".txt,g"
		sed -i "$sed_string" $script

		sed_string="s,correct_key=TODO,correct_key=correct_key_"$run_generic".txt,g"
		sed -i "$sed_string" $script

		sed_string="s,steps=TODO,steps="$1",g"
		sed -i "$sed_string" $script

		sed_string="s,permutations=TODO,permutations="$2",g"
		sed -i "$sed_string" $script

		sed_string="s,steps_start=TODO,steps_start="$3",g"
		sed -i "$sed_string" $script

#sed_string="s,perm_file=TODO,perm_file="$run_".perm,g"
		sed_string="s,perm_file=TODO,perm_file=\""$perm_file\"",g"
		sed -i "$sed_string" $script

		sed_string="s,SBATCH -o TODO,SBATCH -o "$run_".log,g"
		sed -i "$sed_string" $script

		sed_string="s,SBATCH -e TODO,SBATCH -e "$run_".err,g"
		sed -i "$sed_string" $script

		echo "sbatch $script" >> sbatch.sh
	done
done
