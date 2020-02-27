#!/bin/sh

FIFO=/tmp/ttt
trap "cleanup" SIGTERM SIGKILL SIGINT

declare -A field # declare as assoc array
FLD_SIZE=3
INPUT_CONSTRAINT_MSG="You may enter from 0 to $((FLD_SIZE-1))"
PLACEHOLDER='_'

for ((i=0; i<$FLD_SIZE; i++)) do
    for ((j=0; j<$FLD_SIZE; j++)) do
        field[$i,$j]="$PLACEHOLDER"
    done
done

function cleanup() {
    rm $FIFO 2>/dev/null
    exit
}

function show_header() {
    echo "Your symbol is: $SYMB"
    echo "Enter coordinates in this way: x y"
    echo "$INPUT_CONSTRAINT_MSG"
    echo
    echo "Status: $STATUS"
    [[ $ERR_MSG ]] && echo $ERR_MSG
    echo
}
    
function draw_field() {
    echo -n '*'
    for ((i=0; i<$FLD_SIZE; i++)) do
        echo -n " $i"
    done
    echo
    for ((i=0; i<$FLD_SIZE; i++)) do
        echo -n "$i|"
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
            field[$1,$2]=$3
            return 0
        else
            ERR_MSG="Please choose an empty cell"
            return 1
        fi
    else
        ERR_MSG="Please enter valid coords" 
        return 1
    fi
}

function check_win() {
    declare col_last row_last diag1_last diag2_last
    diag1_last=${field[0,0]}
    diag2_last=${field[$((FLD_SIZE-1)),0]}
    for ((i=0; i<$FLD_SIZE; i++)) do
        row_last=${field[$i,0]}
        col_last=${field[0,$i]}
        for ((j=0; j<$FLD_SIZE; j++)) do
            if [[ ${field[$i,$j]} != $row_last ]]; then
                row_last=false
            fi
            if [[ ${field[$j,$i]} != $col_last ]]; then
                col_last=false
            fi
        done
        if [[ ${field[$i,$i]} != $diag1_last ]]; then
            diag1_last=false;
        fi
        if [[ ${field[$((FLD_SIZE-i-1)),$i]} != $diag2_last ]]; then
            diag2_last=false;
        fi
        [[ $col_last != false && $col_last != "$PLACEHOLDER" ]] || \
        [[ $row_last != false && $row_last != "$PLACEHOLDER" ]] && break
    done
    winarr=($col_last $row_last $diag1_last $diag2_last)
    for winner in ${winarr[*]}; do
        if [[ $winner != false && $winner != $PLACEHOLDER ]]; then
            set_status "$winner won!"
            cleanup
        fi
    done
    if [[ ! "${field[@]}" =~ "_" ]]; then
        set_status "Draw!"
        cleanup
    fi
    return -1
}

function redraw_all() {
    clear
    show_header
    draw_field
    unset ERR_MSG
}

function set_status() {
    STATUS=$1
    redraw_all
}

if [[ ! -p "$FIFO" ]]; then
    SYMB='X'
    OPP_SYMB='0'
    mknod $FIFO p
    SWITCH=true
else
    SYMB='0'
    OPP_SYMB='X'
    SWITCH=false
fi


declare x y
while true; do
    check_win
    if [[ $SWITCH == false ]]; then
        set_status "Waiting for opponent's turn"
        read y x < $FIFO
        set_symbol $x $y $OPP_SYMB
        [[ $SWITCH ]] && SWITCH=false || SWITCH=true
    fi
    set_status 'Your turn'
    check_win
    while true; do
        echo -n '> '
        read y x
        set_symbol $x $y $SYMB
        [[ $? -eq 0 ]] && break || redraw_all
    done
    if [[ $SWITCH ]]; then
        set_status "Waiting for connection"
        echo $y $x > $FIFO
        [[ $SWITCH ]] && SWITCH=false || SWITCH=true
    fi
done

cleanup
