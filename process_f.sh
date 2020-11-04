#!/bin/env sh

b=$(cat log.txt | grep -e "ta (s:ns)" | cut -d" " -f4- | awk '{split($0,a,":"); print a[2]}');

min=10000000001000
max=0
previous=0
for i in ${b}
do
    if [ "$i" -lt "$min" ]
    then
        min=$i
        echo min=$min max=$max
    fi
    if [ "$i" -gt "$max" ]
    then
        max=$i
        echo min=$min max=$max
    fi
done


