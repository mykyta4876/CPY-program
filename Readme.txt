The goal of this simulation is to give you some experiences using POSIX Pthreads library functions for thread creations and synchronizations. You will learn how to create threads using pthread_create() and to solve the critical section problems using pthread_mutex_lock(), pthread_mutex_unlock(), pthread_cond_wait(), and pthread_cond_signal() or pthread_cond_broadcast(). 
Program Description 
Write a program in C language under Linux environment to simulate the operations of a Simple Multiprocessor Scheduling Simulation (SMSS). The program, named smss.c, for this simulator should include the following features: 
1.	1)  There is a First-In-First-Out (FIFO) queue, called ReadyQ, containing a list of tasks that are waiting for n CPUs / processors, CPU-1, CPU-2, ..., CPU-n, to execute. Assume that ReadyQ is initially empty, and all CPUs are waiting for the tasks to arrive. Assume ReadyQ has a capacity for m tasks. 
2.	2)  Each CPU is waiting for at least one task to arrive in ReadyQ. The arrival of a task (to ReadyQ) should interrupt any waiting CPUs, each of which will take one task from the queue, and execute it. 
3.	3)  For the input of the simulator, assume you have a file, named TaskFile, that contains a list of tasks. Each task in the file is represented as following: 
TaskNo BurstTime 
where TaskNo is a positive integer, and BurstTime is another positive integer (in second). Note that, each task may have different BurstTime. Create your own TaskFile that contains up to 50 tasks, numbered from 1 to 50, each of which has randomly generated BurstTime with value between 1 and 5. For example, TaskFile may contain the following ten tasks: 
•	1 4 
•	2 3
•	3 5
•	4 1
•	5 2
•	6 4
•	7 3
•	8 1
•	9 3
•	10 2 
4.	4)  Write a function task() that takes no more than three tasks at a time from the TaskFile and puts them into ReadyQ. For each task placed in ReadyQ, e.g., TaskNo = 41 with BurstTime of 2, the task() function would write this activity into a file, named LogFile. 
•	TaskNo 41: 2
Arrival time: 13:42: 
where the arrival time is the time the TaskNo 41 is placed into the queue (actual time). 
5.	5)  Write a function cpu() that simulates the operations of each CPU. When there is task in the queue, one of the processors takes the task from the queue and executes it for its entire BurstTime. Simulate this event using a sleep(BurstTime) call. 
6.	6)  When CPU-1, for example, takes one task from ReadyQ, e.g., TaskNo 41, the CPU would write the following information in LogFile: 
•	Statistics for CPU-1: 
•	TaskNo 41
Arrival time: 13:42:51 
•	Service time: 13:42:57 
The service time is the time the CPU picked up the TaskNo 41 from ReadyQ. Notice that the task’s waiting time is its service time minus arrival time. 
7.	7)  When CPU-1, for example finishes with TaskNo 41, the CPU would write the following information in LogFile: 
•	Statistics for CPU-1: 
•	TaskNo 41
Arrival time: 13:42:51
•	Completion time: 13:42:57 
where the completion time is the time when the CPU finished servicing the TaskNo 41. Notice that the task’s turnaround time is its completion time minus its arrival time. 
8.	8)  The simulator terminates when there is no more task (NOT when the ReadyQ is empty). The simulator should write the following information to LogFile: 
•	Number of tasks:
•	#of tasks Average waiting time: w seconds
•	Average turnaround time: t seconds  
where #of tasks is the total tasks serviced by the n CPUs. The average waiting time and average turnaround time are computed by dividing the total waiting time and turnaround time with the total number of tasks over all the n CPUs, respectively. Use variables TotalTasks, TotalWaitTime and TotalTurnTime shared by the n CPUs. 
9) Note, the program does not require high degree of precision for the time. 


Program Implementation 
1.	1)  Create a thread task that runs the task() routine, and n threads CPU-1, CPU-2, ..., CPU- n, each of which runs the cpu() function. Each CPU thread blocks when the ReadyQ is empty, and the task thread blocks when ReadyQ is full. 
Thus, access to the ReadyQ follows the producer/consumer problem with bounded buffer, i.e., task thread is the producer and CPU threads are the consumers. 
2.	2)  Use pthread mutual exclusion functions, e.g., pthread_mutex_lock(), pthread_mutex_unlock(), pthread_cond_wait(), pthread_cond_signal(), pthread_cond_broadcast(), to solve the critical section and synchronization problems in the simulator. Make sure you consider all possible race conditions in the simulator. 
3.	3)  You must describe/discuss in detail each of the variables, including its data structure, the threads that access them, and how mutual exclusion is achieved. 
4.	4)  Remember to clean up all resources created in your program. 
5.	5)  To test for the correctness of your program, you should run the program as follows: 
smss TaskFile m n 
for m ≤ 3 and m < n ≤ 5. 

===============================
To generate a random TaskFile:
python generate_tasks.py <task_count>

To build the program:
gcc -o smss smss.c -pthread

To run the program:
./smss TaskFile m n

