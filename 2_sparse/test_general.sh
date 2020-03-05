#!/bin/bash
GZ_SRC=./mixed # random data
GZ_RES=./mixed_result # unpacked data
Z1=./z # zeroes (sparse)

[[ ! -f $Z1 ]] && $(truncate -s 512K $Z1)
rm $GZ_SRC.gz $GZ_RES $GZ_SRC 2>/dev/null

# create file from random data and zeroes 
for i in {1..3}; do
    dd if=/dev/urandom of=$GZ_SRC bs=1M count=$i >&/dev/null
    cat $Z1 >> $GZ_SRC
done

gzip -k $GZ_SRC
gzip -cd $GZ_SRC.gz | ./gz-sparse $GZ_RES
diff $GZ_SRC $GZ_RES
if [[ $? -eq 0 ]]; then
    if [[ $(stat -c %b $GZ_RES) -lt $(stat -c %b $GZ_SRC) ]]; then
        echo 'All ok!'
        rm $GZ_SRC.gz $GZ_RES $GZ_SRC 2>/dev/null
        exit 0
    else
        echo "Block sizes of source and result file are equal (result file is not sparse)"
        exit 1
    fi
else
    echo 'Failed!'
    echo "Source size: $(stat -c %s $GZ_SRC)"
    echo "Result size: $(stat -c %s $GZ_RES)"
    exit 1
fi
