#!/bin/bash
make
# if you don't want to include an input file, simply add an underscore to its name
INPUTS=$(ls inp*)
OUT=./out.txt
./exceptions $INPUTS $OUT
#grep all numbers | remove leading zeros | sort
RIGHT_RESULT=$(cat $INPUTS | grep -aoP '\d*' | sed -E "s/^0+([0-9]+)/\1/g" | sort -n)
RESULT=$(cat $OUT)
if [[ $RIGHT_RESULT == $RESULT ]]; then
    echo 'All ok!'
else
    echo 'Wrong result'
    diff <(echo $RIGHT_RESULT | sed "s/ /\n/g") <(echo $RESULT | sed "s/ /\n/g")
fi