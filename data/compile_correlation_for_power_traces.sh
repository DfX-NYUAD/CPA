#!/bin/bash

if [ $# -lt 3 ]; then
       echo "Parameters required:"
       echo "1) List of technologies, e.g., \"TFE0 TFE1 TFE2 TFE3 TFE4\""
       echo "2) Key"
       echo "3) Folder comprising power trace files"
       exit
fi

tech_list=$1
key=$2
folder=$3

# build up header, CSV file name, and list of relevant power trace files
header=""
csv_file="correlation_for_power_traces_across_techs_"
power_files=""
for tech in $tech_list
do
	header=$header""$tech","
	csv_file=$csv_file""$tech"_"
	power_files=$power_files""$folder"/power_traces_*"$tech"*_key_"$key".txt"" "
done
# finally, drop last delimiter
header=${header::-1}
csv_file=${csv_file::-1}
power_files=${power_files::-1}

# add key and ".csv" for file name
csv_file=$csv_file"__key_$key.csv"

# init CSV file with header
echo $header > $folder/$csv_file

# now, paste all the data into the CSV file
paste -d "," $power_files >> $folder/$csv_file

# finally, also generate Pearson correlation equations for spreadsheet handling

## first, generate spacer and header lines
echo "" >> $folder/$csv_file
echo "Pearson correlation,"$header >> $folder/$csv_file

## next, generate the actual correlation calls as matrix
### derive range of column, from `wc -l` -- we must assume all power trace files have same length
col_length=`wc -l $power_files | awk '{print $1}' | head -n1`
col_start=2
col_end=$((col_length + 1))

## walk over matrix
### also init running variable for letters, representing columns within spreadsheet
tech1_letter="A"
for tech1 in $tech_list
do
	line=$tech1","

	tech2_letter="A"
	for tech2 in $tech_list
	do
		line=$line"\"=PEARSON($tech1_letter$col_start:$tech1_letter$col_end,$tech2_letter$col_start:$tech2_letter$col_end)\","

		# increment letter via tr command
		tech2_letter=`echo $tech2_letter | tr '[A-Y]Z' '[B-Z]A'`
	done

	tech1_letter=`echo $tech1_letter | tr '[A-Y]Z' '[B-Z]A'`

	# finally, drop last delimiter and generate line
	echo ${line::-1} >> $folder/$csv_file
done
