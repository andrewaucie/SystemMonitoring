#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/resource.h>
#include <math.h>
#include <getopt.h>

#include "main.h"

void getMemory(int i, bool graphics, double *prevMemory, char memoryArray[1024]) { // PRINTS MEMORY INFORMATION
    struct sysinfo memory;
    sysinfo(&memory);
    double totalram = (double) memory.totalram/1024/1024/1024; // get total ram
    double freeram = (double) memory.freeram/1024/1024/1024; // get free ram
    double totalswap = (double) memory.totalswap/1024/1024/1024; // get total swap
    double freeswap = (double) memory.freeswap/1024/1024/1024; // get free swap
    double totvirtual = (totalram + totalswap); // calculate total virtual memory
    double curvirtual = totvirtual - (freeram + freeswap); // calculate virtual memory used
    sprintf(memoryArray, "%.2lf GB / %.2lf GB -- %.2lf GB / %.2lf GB", totalram - freeram, totalram, curvirtual, totvirtual); // copy the string into array of strings
    if (graphics) { // if graphics command is run
        double currentMemory = curvirtual;
        double diffMemory = currentMemory - *prevMemory;
        if (i == 0) diffMemory = 0;
        strcat(memoryArray, "   |");
        char buffer[1024];
        double absDiffMemory = fabs(diffMemory); // gets absolute value of the memory difference
        int numBuffer = absDiffMemory*10; // sets the number of characters displayed to be the number of 0.1 decimals in number
        if (diffMemory > 0) { // if the differnece is positive, use '#' graphic character
            memset(buffer, '#', numBuffer);
            buffer[numBuffer] = '*';
        } 
        else if (diffMemory < 0) { // if the difference is negative, use ':' graphic character
            memset(buffer, ':', numBuffer);
            buffer[numBuffer] = '@';
        }
        buffer[numBuffer+1] = '\0';
        if ((diffMemory > 0 && diffMemory < 0.01) || diffMemory == 0) { // if the difference is 0 < x < 0.01, count it as zero+
            strcat(memoryArray, "o");
        }
        else {
            strcat(memoryArray, buffer);
        }
        sprintf(memoryArray+strlen(memoryArray), " %.2lf (%.2lf)", absDiffMemory, currentMemory); // copies string into the indexed memory array
        *prevMemory = currentMemory;
    }
}

void printMemory(int i, int samples, bool sequential, char memoryArray[][1024]) { // PRINTS MEMORY INFORMATION
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    if (sequential) { // if sequential flag is called, only print current iteration
        for (int j=0; j<i; j++) printf("\n");
        printf("%s\n", memoryArray[i]);
    }
    else { // otherwise, print previous + current iterations
        for (int j=0; j<=i; j++) printf("%s\n", memoryArray[j]);
    }
    for (int j=samples-1; j>i; j--) printf("\n");
}

void getUser(char userArray[1024]) { // PRINTS USERS
    struct utmp *user1;
    setutent();
    while (user1 = getutent()) {
        if (user1->ut_type == USER_PROCESS) {
            sprintf(userArray + strlen(userArray), "\n%s      %s %s", user1->ut_user, user1->ut_line, user1->ut_host); // print user information
        }
    }
    endutent();
}

void getCPU(int i, bool graphics, double *currCPU, double *prevTime, double *prevUtilization, char cpuArray[1024]) { // PRINTS CPU INFORMATION
    FILE *fp = fopen("/proc/stat", "r");
    long int user, nice, system, idle, iowait, irq, softirq;
    fscanf(fp, "cpu %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq); // access cpu information from file
    fclose(fp);
    double currTime = user + nice + system + idle + iowait + irq + softirq;
    double currUtilization = currTime - idle;
    double totalUtilization = 0;
    if (i > 0) {
        totalUtilization = 100*((currUtilization - * prevUtilization) / (currTime - *prevTime)); // as per instructions
    }
    if (graphics) {
        strcpy(cpuArray, "\n        ");
        char buffer[1024];
        int numBuffer = (int) fabs(totalUtilization) + 1; // sets the number of characters displayed to be the rounded number of cpu use
        memset(buffer, '|', numBuffer);
        buffer[numBuffer] = '\0';
        sprintf(cpuArray+strlen(cpuArray), "%s %.2lf", buffer, totalUtilization); // copies current string to cpuArray
    }
    *currCPU = totalUtilization; 
    *prevTime = currTime;
    *prevUtilization = currUtilization;
}



void printCPU(int i, int samples, bool graphics, bool sequential, double currentCPU, char cpuArray[][1024]) { // PRINTS CPU INFORMATION
    printf("Number of cores: %ld\n", sysconf(_SC_NPROCESSORS_ONLN)); // get number of cores
    printf("total cpu use = %.2lf%%\n", currentCPU);
    if (graphics) {
        if (sequential) { // if sequential flag is raised, only print the information of current iteration
            for (int j=0; j<=i; j++) printf("\n");
            printf("%s", cpuArray[i]);
        }
        else { // otherwise, print information from previous + current iteration
            for (int j=0; j<=i; j++) printf("%s", cpuArray[j]);
        }
        for (int j=samples; j>i; j--) printf("\n");
    }
}
