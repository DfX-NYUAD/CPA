#!/bin/bash

if [ $# -lt 2 ]; then
       echo "Parameters required:"
       echo "1) Number of steps for exploring subsets of traces?"
       echo "2) Number of permutations to consider for each subset of traces?"
       exit
fi

for run in `cat dalma_runs`
do
	script=$run".sh"

	echo "Generate $script ..."

	cp slurm.sh.template $script

	sed_string="s,power_traces=TODO,power_traces=power_traces_"$run".txt,g"
	sed -i "$sed_string" $script

	sed_string="s,cipher_text=TODO,cipher_text=cipher_text_"$run".txt,g"
	sed -i "$sed_string" $script

	sed_string="s,correct_key=TODO,correct_key=correct_key_"$run".txt,g"
	sed -i "$sed_string" $script

	sed_string="s,steps=TODO,steps="$1",g"
	sed -i "$sed_string" $script

	sed_string="s,permutations=TODO,permutations="$2",g"
	sed -i "$sed_string" $script

	sed_string="s,SBATCH -o TODO,SBATCH -o "$run".log,g"
	sed -i "$sed_string" $script

	sed_string="s,SBATCH -e TODO,SBATCH -e "$run".log.error,g"
	sed -i "$sed_string" $script
done
