#!/bin/bash
make

INPUTS=$(ls inp*)
OUT=./out.txt
./exceptions $INPUTS $OUT
RIGHT_RESULT=$(cat $INPUTS | grep -aoP '\d*')
RESULT=$(cat $OUT)
if [[ $RIGHT_RESULT == $RESULT ]]; then
    echo 'All ok!'
else
    echo 'Wrong result'
    diff <(echo $RIGHT_RESULT) <(echo $RESULT)
fi