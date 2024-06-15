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
#include <sys/types.h>
#include <sys/wait.h>

#include "main.h"
#include  <stdio.h>
#include  <signal.h>
#include  <stdlib.h>

void INThandler(int sig)
{
    char c;
    signal(sig, SIG_IGN);
    printf("Do you really want to quit? [y/n]");
    c = getchar();
    if (c == 'y' || c == 'Y')
        exit(0);
    else
        signal(SIGINT, INThandler);
    getchar(); // Get new line character
}
void interceptZ(int signal) {
    // intercept ctrl-z
}

typedef struct {
    char memoryString[1024];
    double previousMemory;
} MemoryData;

// Define the struct to hold the data for the CPU pipe
typedef struct {
    char cpuString[1024];
    double currentCPU;
    double previousTime;
    double previousUtilization;
} CPUData;

bool isDigit(char num[]) { // checks if every character of a string is a digit
    for (int j=0; j<strlen(num); j++) {
        if (!(num[j] >= '0' && num[j] <= '9')) {
            return false;
        }
    }
    return atoi(num) != 0;
}

void printSystemInfo() { // PRINTS SYSTEM INFORMATION
    struct utsname sysinfo;
    uname(&sysinfo);
    printf("### System Information ###\n");
    printf(" System Name: %s\n", sysinfo.sysname);
    printf(" Machine Name: %s\n", sysinfo.nodename);
    printf(" Version: %s\n", sysinfo.version);
    printf(" Release: %s\n", sysinfo.release);
    printf(" Architecture: %s\n", sysinfo.machine);
    printf("---------------------------------------\n");
}

int main(int argc, char **argv) {
    bool system_f = false, user_f = false, graphics = false, sequential = false;
    int samples = 10, tdelay = 1;
    bool firstArg = false, secondArg = false; // used for positional arguments
    signal(SIGTSTP, interceptZ);
    signal(SIGINT, INThandler);
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "--system") == 0 || strcmp(argv[i], "-s") == 0) { // check for system flag
            system_f = true;
        }
        else if (strcmp(argv[i], "--user") == 0 || strcmp(argv[i], "-u") == 0) { // check for user flag
            user_f = true;
        }
        else if (strcmp(argv[i], "--graphics") == 0 || strcmp(argv[i], "-g") == 0) { // check for graphics flag
            graphics = true;
        }
        else if (strcmp(argv[i], "--sequential") == 0 || strcmp(argv[i], "-se") == 0) { // check for sequential flag
            sequential = true;
        }
        else if (strncmp(argv[i], "--samples=", 10) == 0 || strncmp(argv[i], "-s=", 3) == 0) { // check for sample flag
            int j=0;
            for (j=0; j<strlen(argv[i]); j++) { // finds index of =
                if (argv[i][j] == '=') {
                    break;
                }
            }
            char num[1024];
            strcpy(num, &argv[i][j+1]); // copies the string after the '=' into num
            if (isDigit(num)) { // assigns value to samples if 'num' is a digit
                samples = atoi(num);
                firstArg = true; // tells the program that the samples flag has now been called
            }
        }
        else if (strncmp(argv[i], "--tdelay=", 9) == 0 || strncmp(argv[i], "-t=", 3) == 0) {
            int j=0;
            for (j=0; j<strlen(argv[i]); j++) { // finds index of =
                if (argv[i][j] == '=') {
                    break;
                }
            }
            char num[1024];
            strcpy(num, &argv[i][j+1]); // copies the string after the '=' into num
            if (isDigit(num)) { // assigns value to samples if 'num' is a digit
                tdelay = atoi(num); 
                secondArg = true; // tells the program that the tdelay flag has now been called
            }
        }
        else if (isDigit(argv[i]) && !firstArg && !secondArg) { // used for positional arguments, if sample and tdelay haven't been called
            samples = atoi(argv[i]);
            firstArg = true;
        }
        else if (isDigit(argv[i]) && firstArg && !secondArg) { // used for positional arguments, if sample has been called, but not tdelay
            tdelay = atoi(argv[i]);
            secondArg = true;
        }
    }
    char memoryArray[samples][1024], cpuArray[samples][1024];

    char memoryString[1024], userString[1024], cpuString[1024];
    MemoryData memoryData;
    CPUData cpuData;
    for (int i=0; i<samples; i++) {
        strcpy(memoryString, "");
        strcpy(userString, "");
        strcpy(cpuString, "");

        double memoryUsage = 0;
        int retval; // variable to hold return values of system calls
        int memoryPipe[2], userPipe[2], cpuPipe[2];
        
        if (pipe(memoryPipe) != 0 || pipe(userPipe) != 0 || pipe(cpuPipe) != 0) {
            fprintf(stderr, "Failed to create pipe");
            exit(EXIT_FAILURE);
        }

        pid_t child_pids[3]; // global array to store child PIDs

        for (int j=0; j<3; j++) {
            if (j == 0) {
                if ((child_pids[j] = fork()) == -1) { // if fork failed
                    fprintf(stderr, "Failed to fork");
                } 
                else if (child_pids[j] == 0) { // child to calculate memory and write to pipe
                    close(memoryPipe[0]);
                    getMemory(i, graphics, &memoryData.previousMemory, memoryData.memoryString);
                    if ((retval = write(memoryPipe[1], &memoryData, sizeof(memoryData))) < 0) { // error checking pipe
                        fprintf(stderr, "Error writing to pipe");
                        exit(EXIT_FAILURE);
                    }
                    close(memoryPipe[1]);
                }
                else {
                    while(wait(NULL) > 0); // wait for chil to finish before reading from pipe and storing string
                    close(memoryPipe[1]);
                    if ((retval = read(memoryPipe[0], &memoryData, sizeof(memoryData))) < 0) {
                        fprintf(stderr, "Error writing to pipe");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(memoryArray[i], memoryData.memoryString);
                    close(memoryPipe[0]);
                }
            }
            else if (j == 1) {
                if ((child_pids[j] = fork()) == -1) {
                    fprintf(stderr, "Failed to fork");
                } 
                else if (child_pids[j] == 0) {
                    close(userPipe[0]);
                    getUser(userString);
                    if ((retval = write(userPipe[1], userString, strlen(userString) + 1)) < 0) {
                        fprintf(stderr, "Error writing to pipe");
                        exit(EXIT_FAILURE);
                    }
                    close(userPipe[1]);
                }
                else {
                    while(wait(NULL) > 0);
                    close(userPipe[1]);
                    if ((retval = read(userPipe[0], userString, sizeof(userString))) < 0) {
                        fprintf(stderr, "Error reading from pipe");
                        exit(EXIT_FAILURE);
                    }
                    close(userPipe[0]);
                }
            }
            else if (j == 2) {
                if ((child_pids[j] = fork()) == -1) {
                    fprintf(stderr, "Failed to fork");
                } 
                else if (child_pids[j] == 0) {
                    close(cpuPipe[0]);
                    getCPU(i, graphics, &cpuData.currentCPU, &cpuData.previousTime, &cpuData.previousUtilization, cpuData.cpuString);
                    if ((retval = write(cpuPipe[1], &cpuData, sizeof(cpuData))) < 0) {
                        fprintf(stderr, "Error writing to pipe");
                        exit(EXIT_FAILURE);
                    }
                    close(cpuPipe[0]);
                }
                else {
                    while(wait(NULL) > 0);
                    close(cpuPipe[1]);
                    if ((retval = read(cpuPipe[0], &cpuData, sizeof(cpuData))) < 0) {
                        fprintf(stderr, "Error reading from pipe");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(cpuArray[i], cpuData.cpuString);
                    close(cpuPipe[0]);
                }
            }
        }
        for (int i=0; i<3; i++) {
            if (child_pids[i] == 0) {
                exit(EXIT_SUCCESS);
            }
        }
        if (sequential) {
            printf("\n>>> iteration %d\n", i+1); // prints current iteration for the sequential flag
        }
        else {
            printf("\033[H \033[2J \n"); // clears screen and places cursor at top
            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay); // prints number of samples and tdelay
        }
        struct rusage usage_info;
        getrusage(RUSAGE_SELF, &usage_info);
        printf(" Memory usage: %ld kilobytes", memoryUsage); //access rusage to get memory usage
        printf("\n---------------------------------------\n");
        
        if (user_f && !system_f) { // if only user flag is called, print users only
            printf("### Sessions/users ###");
            printf("%s\n", userString);
        }
        else if (!user_f && system_f) { // if only system flag is called, print system usage only
            printMemory(i, samples, sequential, memoryArray);
            printf("---------------------------------------\n");
            printCPU(i, samples, graphics, sequential, cpuData.currentCPU, cpuArray);
        }
        else { // if no flags are called or both user and system flags are called, print system usage and users
            printMemory(i, samples, sequential, memoryArray); // prints memory
            printf("---------------------------------------\n"); 
            printf("### Sessions/users ###");
            printf("%s", userString);
            printf("\n---------------------------------------\n");
            printCPU(i, samples, graphics, sequential, cpuData.currentCPU, cpuArray); // prints cpu info
        }
        sleep(tdelay); //wait tdelay seconds
    }
    printf("---------------------------------------\n");
    printSystemInfo();
    return 0;
}

