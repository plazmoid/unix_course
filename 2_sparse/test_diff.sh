#!/bin/bash
ARG=$1
[[ -f $ARG ]] || echo 'please enter datafile to diff'; exit 1
cat $ARG | ./gz-sparse n_$ARG
diff $ARG n_$ARG
