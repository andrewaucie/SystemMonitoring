#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <utmp.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#ifndef __main
#define __main

void getMemory(int i, bool graphics, double *prevMemory, char memoryArray[1024]);
void getUser(char userArray[1024]);
void getCPU(int i, bool graphics, double *currCPU, double *prevTime, double *prevUtilization, char cpuArray[1024]);
void printMemory(int i, int samples, bool sequential, char memoryArray[][1024]);
void printCPU(int i, int samples, bool graphics, bool sequential, double currentCPU, char cpuArray[][1024]);
double getMemoryUsage();

#endif