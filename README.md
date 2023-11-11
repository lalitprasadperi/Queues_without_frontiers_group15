**Assignment and Homework Group #15**
Lalit Prasad Peri (lalitprasad@vt.edu)
Will Downey (willd20@vt.edu)

**How to Compile and Run**
> >make all           (this will build TTAS and Wait-free tests) 
> >make -f Makefile2  (this will build lock-free tests)
> >./run_test.sh      (run all)

**Queues Without Frontiers**

You may work in groups of two for this assignment.  The purpose of your second programming assignment is to implement efficient lock-free and wait-free (your choice) FIFO queues. You will observe during the assignment that wait-free implementations of FIFO queues with arbitrary numbers of enqueuers and dequeuers can be challenging. You will find solutions to this problem in the literature we share with you. That said, you are also free to constrain the number of concurrent enqueuers/dequeuers in your wait-free implementation without grade penalty if this helps you complete the assignment.

Your queues should be simply linked lists with head and tail pointers. The head is always a sentinel node, while the tail, depending on your implementation, could be a pointer to the last node or a sentinel node. The queue should have two methods:

>* enqueue (Q, int data) #enqueue a node with content "data" at the tail of the queue pointed to by Q
>* dequeue(Q, int *data) #dequeue a node from the head of the queue pointed by Q and return its data

The starting point of this assignment should be the best lock you have experimented with in the previous assignment. We ask you to clone your first assignment repository, where you keep only that best lock (notionally, this should be the TTAS lock with a carefully tuned back-off). You should then create a simple FIFO queue data structure in C, using pointers, and enable multithreaded enqueued and dequeues using the best lock you implemented thus far for mutual exclusion. You should populate your currently empty second assignment repos in GitHub Classroom using this linkLinks to an external site..

**_Deliverables_**
* You should implement at least one lock-free queue (35% of your grade), and at least one wait-free queue (35%) of your grade. The choice of algorithms is entirely yours, but we do expect a justification of your choice in your report. The justification can be experimental (based on measurements on different implementations), or theoretical (based on your understanding of the behavior of different algorithms under contention).
* You should conduct a series of correctness checks (15% of your grade) of your lock-free and your wait-free queues as follows:
Verify the content queue with 10^6 concurrent enqueue() calls (in separate tests  2, 4, 8, and 16 threads), with a data value from 1 to 10^6. Each thread should perform the same number of enqueues (approximately, since the number of threads may not divide evenly the number of enqueue operations). You should verify the queue's content with a single thread (sequentially), scanning the queue from start to finish and expecting to find all the inserted values. Make sure that each thread enqueue inserts a distinct value (you may find it helpful to use thread IDs for this and partition the range of values between threads according to their ID). Obviously, you will not be able to control the order in which the data elements are inserted in the queue.
* Following verification of the inserts, verify the queue with 10^6 concurrent dequeues(), using the same parameters (varying the number of threads and having each thread perform approximately the same number of dequeues). Your test should verify that you return existing values from the queue, that all distinct values are returned, and that the queue is empty at the end of the test.
* Create a new test to verify concurrent enqueues, which need to be completed and then followed by 
concurrent dequeues and concurrent enqueues. You need to use a barrier to verify completion of the first enqueue batch. For the second stage of the test, alternate enqueue() and dequeue() calls in each thread. Your queue at the end of this test should be left with half a million valid data items from previous enqueues, and all your dequeues should return valid data items. Each thread should perform approximately the same number of enqueues and dequeues, and you should vary the number of threads as in the previous tests. 
* You should evaluate the performance of your lock-based and lock-free/wait-free queues (15% of your grade) and provide a report with your findings, along with an explanation for them. You should measure the performance of the queues in terms of throughput (enqueues/second, dequeues/second, mixed enqueues-dequeues/second), and you can use the same tests that you used for verification without the verification code for this purpose. Your report should be added as a markup file to your GitHub repository. You should also provide a comprehensive readme file that explains how to precisely run your verification and performance tests. All your verification and performance tests should be reproducible in terms of our ability to execute them, verify the correctness of your code, and approximately verify your results (system conditions may, of course, vary during executions, but over many executions, our experimental results from running your codes should be similar to yours).

**_Read-Up_**
In searching for an efficient concurrent queue algorithm to implement, you can start from references [125]Links to an external site., [126Links to an external site.], and [164]Links to an external site. in your textbook. More recent works explore (hereLinks to an external site., hereLinks to an external site., and hereLinks to an external site.) wait-free implementations with various performance or space guarantees.

