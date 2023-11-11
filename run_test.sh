#!/bin/bash

function run_test() {
    for nthr in 1 2 4 8 16; do
        ./$1 $nthr > /dev/null
        #for i in `seq 1 3`; do
            ./$1 $nthr
        #done
        echo
    done
}

make clean
make all

echo "--------------------------------------"
echo "test test-spinlock-ttas"
echo "--------------------------------------"
run_test "test-spinlock-ttas"

echo "--------------------------------------"
echo "TEST#1 ENQ-TTAS // 1 Million Enqueues "
echo "--------------------------------------"
run_test "test-enque-ttas"
echo "--------------------------------------"
echo "TEST#2 DEQ-TTAS // 1 Million Dequeues "
echo "--------------------------------------"
run_test "test-deque-ttas"
echo "--------------------------------------"
echo "TEST#3 ENQ.ENQ.DEQ-TTAS // 1 Million Enqueues + 1 Million Enques-> 1 Million Dequeues"
echo "--------------------------------------"
run_test "test-enq-deq-ttas"

echo "--------------------------------------"
echo "TEST#1 ENQ-WAIT-FREE // 1 Million Enqueues "
echo "--------------------------------------"
run_test "test-enque-wait-free"
echo "--------------------------------------"
echo "TEST#2 DEQ-WAIT-FREE // 1 Million Dequeues "
echo "--------------------------------------"
run_test "test-deque-wait-free"
echo "--------------------------------------"
echo "TEST#3 ENQ.ENQ.DEQ-WAIT-FREE // 1 Million Enqueues + 1 Million Enques-> 1 Million Dequeues"
echo "--------------------------------------"
run_test "test-enq-deq-wait-free"

# echo "--------------------------------------"
# echo "TEST ENQ-LOCK-FREE // 1 Million Enqueues "
# echo "--------------------------------------"
# run_test "test-enque-lock-free"
# echo "--------------------------------------"
# echo "TEST DEQ-LOCK-FREE // 1 Million Dequeues "
# echo "--------------------------------------"
# run_test "test-deque-lock-free"
echo "--------------------------------------"
echo "--------------------------------------"
echo "--------------------------------------"
echo "TEST ENQ.DEQ-LOCK-FREE // 1 Million Enqueues + 1 Million Dequeues"
echo "--------------------------------------"
run_test "msqueue"

