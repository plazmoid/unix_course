#!/bin/bash
# Test floating bug with variable length of result file
LOG=./wrong.log
TLOG=/tmp/l
# truncate -s 1M z
ZFILE=./z
ZO=./zz
ZSIZE=$(stat -c %s $ZFILE)
rm $LOG $TLOG 2>/dev/null

for i in {0..10000}; do
    cat $ZFILE | ./gz-sparse $ZO
    ZOSIZE=$(stat -c %s $ZO)
    if [[ $ZOSIZE -ne $ZSIZE ]]; then
        echo $ZOSIZE >> $TLOG
    fi
done
if [[ ! -f $TLOG ]]; then
    echo 'All ok!'
else
    sort $TLOG | uniq > $LOG
    rm $TLOG
    wl=$(cat $LOG | wc -l)
    echo "Was $wl wrong lens, see $LOG"
fi
