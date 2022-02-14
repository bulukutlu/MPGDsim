#!/bin/bash

count=3
for i in {1..13}
do
	count=$((count + 1))
	head -9 field_MMG*_Edrift0-50-600_MMG440.txt > outfile_$i.txt
	sed -i '9s/.*/% x             y              z              V (V)/' outfile_$i.txt 
	awk -v l="$count" 'NR>9 {print $1,"\t\t",$2,"\t\t",$3,"\t\t",$l}' field_MMG*_Edrift0-50-600_MMG440.txt >> outfile_$i.txt
done