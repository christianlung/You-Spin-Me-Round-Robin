## UID: 005731615
(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

## You Spin Me Round Robin
This is a simulation for round robin scheduling given a workload and quantum length from a txt file. It returns the average wait time and response time for processes to assess the effectiveness of the scheduling algorithm. Wait time is defined by ... while response time is.... Round Robin scheduler optimizes these wait and response times by giving each process the CPU for a quanta of time before switching off with the next process. This is implemented using a queue and new processes are added to the back of the queue. 

## Building

Explain briefly how to build your program

Run the command ```"make"```

## Running

Show an example run of your program, using at least two additional arguments, and what to expect
```
./rr processes.txt 3
Average waiting time: 7.00
Average response time: 2.75
```
You should expect two lines: 1 line for average wait time and 1 line for average response time

## Cleaning up

Explain briefly how to clean up all binary files
Run the command "make clean"
