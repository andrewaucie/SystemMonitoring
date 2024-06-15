## ﻿Signals:
I intercepted the Ctrl-C and Ctrl-Z signal to make Ctrl-C ask the user to exit and Ctrl-Z to do nothing. I used SIGTSTP to intercept Ctrl-Z and SIGINT to intercept Ctrl-C
## Concurrency:
I forked in a for loop 3 times, depending on the index, I went into different processes of the for loop to get memory, users, and cpu usage.
In each fork, I wait for the child to finish before corresponding to its parent. I only exit from the children at the end of the for loop to ensure the processes are concurrent, by keeping track of the child pids and exiting from all of them to ensure no zombies.
# Functions: 
### bool isDigit (char num[])
    • Checks if every character in num is a digit (between 0 and 9 inclusive) • 	Returns true if it is, false otherwise 
`void getMemory(int i, bool graphics, double *prevMemory, char memoryArray[1024])`
    • Get memory 
`void getUser(char userArray[1024])`
    • Gets user information
`void getCPU(int i, bool graphics, double *currCPU, double *prevTime, double *prevUtilization, char cpuArray[1024])`
    • Get CPU information depending 
`void printMemory(int i, int samples, bool sequential, char memoryArray[][1024])`
`void printCPU(int i, int samples, bool graphics, bool sequential, double currentCPU, char cpuArray[][1024])`
To calculate memory usage, I accessed rusage based on the instructors answers and research online.  
To calculate physical and virtual memory, I accessed sysinfo and calculated physical memory used with:  
“total ram – free ram”, total physical memory with “total ram”, virtual memory used (as per instructions) with “(total ram + total swap)  - (free ram + free swap)”, and virtual memory with “total ram + total swap”  
I used an array of strings to store the information from previous iterations, and printed/updated the array on every iteration.  
When doing graphics, my scale used was displaying one ‘#’ or ‘:’ for every difference of 0.1. For example, a difference in 0.2GB would result in 2 characters being displayed.  
When calculating the zero+, zero-, I considered when the number is 0 < x < 0.01 and -0.01 < x < 0 CPU 
–  
When doing graphics, my scaled used was displaying one ‘|’ for every number (i.e. 25.5% is 25 ‘|’)  
The flags used were:  
System Usage: --system or -s  
Users: --users or -u  
Sequential: --sequential or -se  
Graphics: --graphics or -g  
Samples: --samples=N or -s=N or positional argument #1  
Tdelay: --tdelay=N or -t=N or positional argument #2  

# Instructions
Run the following commands:
`make`
`./b09a3 [FLAGS]`

  
