#!/bin/bash

function run_test() {
    for nthr in 1 2 4 8 16; do
        ./$1 $nthr > /dev/null
        for i in `seq 1 5`; do
            ./$1 $nthr
        done
        echo
    done
}

make clean
make all

#echo "--------------------------------------"
#echo "test test-spinlock-ttas"
#echo "--------------------------------------"
#run_test "test-spinlock-ttas"

echo "--------------------------------------"
echo "TEST ENQ-TTAS // 0.5 Million Enqueues "
echo "--------------------------------------"
run_test "test-enque-ttas"
echo "--------------------------------------"
echo "TEST DEQ-TTAS // 0.5 Million Dequeues "
echo "--------------------------------------"
run_test "test-deque-ttas"
echo "--------------------------------------"
echo "TEST ENQ.DEQ-TTAS // 0.5 Million Enqueues + 0.5 Million Dequeues"
echo "--------------------------------------"
run_test "test-enq-deq-ttas"

# echo "--------------------------------------"
# echo "TEST ENQ-LOCK-FREE // 0.5 Million Enqueues "
# echo "--------------------------------------"
# run_test "test-enque-lock-free"
# echo "--------------------------------------"
# echo "TEST DEQ-LOCK-FREE // 0.5 Million Dequeues "
# echo "--------------------------------------"
# run_test "test-deque-lock-free"
# echo "--------------------------------------"
# echo "TEST ENQ.DEQ-LOCK-FREE // 0.5 Million Enqueues + 0.5 Million Dequeues"
# echo "--------------------------------------"
# run_test "test-enq-deq-lock-free"
# 
# echo "--------------------------------------"
# echo "TEST ENQ WAIT-FREE // 0.5 Million Enqueues "
# echo "--------------------------------------"
# run_test "test-enque-wait-free"
# echo "--------------------------------------"
# echo "TEST DEQ WAIT-FREE // 0.5 Million Dequeues "
# echo "--------------------------------------"
# run_test "test-deque-wait-free"
# echo "--------------------------------------"
# echo "TEST ENQ.DEQ WAIT-FREE // 0.5 Million Enqueues + 0.5 Million Dequeues"
# echo "--------------------------------------"
# run_test "test-enq-deq-wait-free"
# 
