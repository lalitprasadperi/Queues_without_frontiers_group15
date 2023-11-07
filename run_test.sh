#!/bin/bash

function run_test() {
    for nthr in 1 2 4 8 16; do
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

echo "--------------------------------------"
echo "test test-enque-ttas"
echo "--------------------------------------"
run_test "test-enque-ttas"
echo "--------------------------------------"
echo "test test-deque-ttas"
echo "--------------------------------------"
run_test "test-deque-ttas"
echo "--------------------------------------"
echo "test test-enq/deq-ttas"
echo "--------------------------------------"
run_test "test-enq-deq-ttas"

 echo "--------------------------------------"
 echo "test test-enqu-lock-free"
 echo "--------------------------------------"
 run_test "test-enque-lock-free"
 echo "--------------------------------------"
 echo "test test-deq-lock-free"
 echo "--------------------------------------"
 run_test "test-deque-lock-free"
 echo "--------------------------------------"
 echo "test test-enq.deq-lock-free"
 echo "--------------------------------------"
 run_test "test-enq-deq-lock-free"
 
 echo "--------------------------------------"
 echo "test test-enqu-wait-free"
 echo "--------------------------------------"
 run_test "test-enque-wait-free"
 echo "--------------------------------------"
 echo "test test-deq-wait-free"
 echo "--------------------------------------"
 run_test "test-deque-wait-free"
 echo "--------------------------------------"
 echo "test test-enq.deq-wait-free"
 echo "--------------------------------------"
 run_test "test-enq-deq-wait-free"

