#!/bin/sh

FIFO=/tmp/ttt
trap "rm $FIFO; exit" SIGTERM SIGKILL SIGINT

declare -A field # declare as assoc array
FLD_SIZE=3
INPUT_CONSTRAINT_MSG="You may enter from 0 to $((FLD_SIZE-1))"
PLACEHOLDER='_'

for ((i=0; i<$FLD_SIZE; i++)) do
    for ((j=0; j<$FLD_SIZE; j++)) do
        field[$i,$j]="$PLACEHOLDER"
    done
done
    
function draw_field() {
    for ((i=0; i<$FLD_SIZE; i++)) do
        echo -n '|'
        for ((j=0; j<$FLD_SIZE; j++)) do
            echo -n "${field[$i,$j]}"
            if [[ $j -eq $((FLD_SIZE-1)) ]]; then
                echo '|'
            else
                echo -n ' '
            fi
        done
    done
}

function set_symbol() {
    if [[ $1 -lt "$FLD_SIZE" ]] && [[ $1 -gt -1 ]] && \
       [[ $2 -lt "$FLD_SIZE" ]] && [[ $2 -gt -1 ]]; then
        if [[ ${field[$1,$2]} == "$PLACEHOLDER" ]]; then
            field[$1,$2]=$SYMB
            return 0
        else
            echo "Please choose an empty cell"
            return 1
        fi
    else
        echo $INPUT_CONSTRAINT_MSG
        return 1
    fi
}

function check_win() {
    declare col_last row_last diag1_last diag2_last
    diag1_last=${field[0,0]}
    diag2_last=${field[$((FLD_SIZE-1)),0]}
    for ((i=0; i<$FLD_SIZE; i++)) do
        col_last=${field[i,0]}
        row_last=${field[0,i]}
        for ((j=0; j<$FLD_SIZE; j++)) do
            if [[ ${field[i,j]} != $col_last ]]; then
                col_last=false
            fi
            if [[ ${field[j,i]} != $row_last ]]; then
                row_last=false
            fi
        done
        if [[ ${field[i,i]} != $diag1_last ]]; then
            diag1_last=false;
        fi
        if [[ ${field[$((FLD_SIZE-i-1)),i]} != $diag2_last ]]; then
            diag2_last=false;
        fi
    done
    winarr=($col_last $row_last $diag1_last $diag2_last)
    echo "col: $col_last"
    echo "row: $row_last"
    echo "d1: $diag1_last"
    echo "d2: $diag2_last"
    for winner in ${winarr[*]}; do
        if [[ $winner != false && $winner != $PLACEHOLDER ]]; then
            echo $winner
            return 0
        fi
    done
    return -1
}

if [[ ! -p "$FIFO" ]]; then
    SYMB='X'
    mknod $FIFO p
else
    SYMB='0'
fi

clear
echo "You're playing as $SYMB"
echo "Enter coordinates to put your $SYMB like this: x y"
echo $INPUT_CONSTRAINT_MSG

declare x y
while true; do
    draw_field
    winner=$(check_win)
    if [[ $winner != "" && $? -eq 0 ]]; then
        echo "$winner wins!"
        break;
    fi
    echo $winner
    read y x
    clear
    set_symbol $x $y
    #if [[ $? -eq 0 ]]; then
    #    echo yos
    #fi
done

rm $FIFO
