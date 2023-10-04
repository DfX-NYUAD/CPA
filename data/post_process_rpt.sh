#!/bin/bash

for file in /home/projects/jb7410/aes_static/synth/aes128_table_ecb_22nm/reports_-2_*_0.5/collect.rpt; do

	echo $file

	for i in {126..18}; do
	
		ii=$((i+1))

		sudo sed -i "s/state\[$i\]/state\[$ii\]/g" $file
	done

	sudo sed -i "s/n2746/state\[18\]/g" $file
done
