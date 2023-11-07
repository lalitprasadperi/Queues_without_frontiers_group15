#!/bin/bash

function run_test() {
    for nthr in 1 2 4 8 16 32; do
        ./$1 $nthr > /dev/null
        for i in `seq 1 3`; do
            ./$1 $nthr
        done
        echo
    done
}

echo "--------------------------------------"
echo "test test-spinlock-ttas"
echo "--------------------------------------"
run_test "test-spinlock-ttas"

