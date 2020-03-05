#!/bin/bash
# Test floating bug with variable length of result file
LOG=./wrong.log
TLOG=/tmp/l
ZFILE=./z
[[ ! -f $ZFILE ]] && $(truncate -s 1M $ZFILE)
# out
ZO=./zz
ZSIZE=$(stat -c %s $ZFILE)
rm $LOG $TLOG 2>/dev/null

for i in {0..1000}; do
    cat $ZFILE | ./gz-sparse $ZO
    ZOSIZE=$(stat -c %s $ZO)
    if [[ $ZOSIZE -ne $ZSIZE ]]; then
        echo $ZOSIZE >> $TLOG
    fi
done
if [[ ! -f $TLOG ]]; then
    echo 'All ok!'
    rm $ZFILE $ZO $LOG $TLOG 2>/dev/null
    exit 0
else
    sort $TLOG | uniq > $LOG
    rm $TLOG
    wl=$(cat $LOG | wc -l)
    echo "$wl lengths was different from $ZSIZE, see $LOG"
    exit 1
fi

