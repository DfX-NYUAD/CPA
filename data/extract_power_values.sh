#!/bin/bash

files=/home/projects/jb7410/aes_static/synth/aes128_table_ecb_22nm/reports_-2_*_0.5/collect.rpt
lib_root=/data/old_server/home/projects/vlsi/libraries/28nm/ref_libs/std_cells/
lib_root_=/timing_libs/ccs/libs/
lib_file_ext=.lib
out_file=power.table

##
## global init
##

rm $out_file 2> /dev/null

unique_FFs=$(cat $files | awk '{print $NF}' | sort -u)

for FF in $unique_FFs; do

	##
	## init per FF
	##

	echo "Unique FF: $FF"

	lib=${FF%/*}
	cell=${FF#*/}

	if [[ "$lib" == *"lvt"* ]]; then
		vt=lvt
	elif [[ "$lib" == *"rvt"* ]]; then
		vt=rvt
	elif [[ "$lib" == *"hvt"* ]]; then
		vt=hvt
	else
		echo "ERROR: lib has unsupported VT string!"
		exit 1
	fi

	lib_file=$lib_root$vt$lib_root_$lib$lib_file_ext

	if ! [[ -e $lib_file ]]; then

		echo "ERROR: lib file $lib_file does not exist!"
		exit 1
	fi

	echo " Related lib file: $lib_file"

	##
	## extract line numbers / range for related cell definition in lib file
	##

	range_string=$(grep -nE "^cell \(.*\)" $lib_file | grep -A1 -E "cell \($cell\)")
## dbg
#	echo $range_string

	range_start=-1
	range_stop=-1

	for word in $range_string; do

		if [[ "$word" != *":cell" ]]; then
			continue
		fi

		if [[ $range_start == -1 ]]; then
			range_start=${word%:*}
		else
			range_stop=${word%:*}
		fi
	done

## dbg
#	echo "range_start=$range_start"
#	echo "range_stop=$range_stop"


	##
	## extract power values for all relevant cases, i.e., for CDN high and all other cases of CP, D, Q
	## TODO revise for different FFs as needed
	##

	for CP in {0..1}; do
		for D in {0..1}; do
			for Q in {0..1}; do

				## build up string to search for

				if [[ $CP == 0 ]]; then
					CP_string="!CP"
				else
					CP_string="CP"
				fi

				if [[ $D == 0 ]]; then
					D_string="!D"
				else
					D_string="D"
				fi

				if [[ $Q == 0 ]]; then
					Q_string="!Q"
				else
					Q_string="Q"
				fi

				search_string="CDN $CP_string $D_string $Q_string"

				## build up string to be put into power model file
				model_string="CDN=1,CP=$CP,D=$D,Q=$Q"

				## actual search/extraction

				value_=$(sed -n $range_start","$range_stop"p" $lib_file | grep -B1 "when : \"$search_string\"" | head -n1 | awk '{print $NF}')
				value=${value_%;*}

				echo " Case: $model_string"
				echo " Leakage power: $value"

				echo "$cell -- $model_string -- $value" >> $out_file
			done
		done
	done

## dbg
#	exit

	echo
done
