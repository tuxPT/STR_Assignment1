#!/bin/env sh

b=$(cat log.txt | grep -e "ta (s:ns)" | cut -d" " -f4- | awk '{split($0,a,":"); print a[2]}');

min=10000000001000
max=0
previous=0
for i in ${b}
do
    a=$(echo $i - $previous | bc)
    if [ "$a" -lt "$min" ]
    then
        min=$a
        previous=$i
        echo min=$min max=$max
    fi
    if [ "$a" -gt "$max" ]
    then
        max=$a
        previous=$i
        echo min=$min max=$max
    fi
done


