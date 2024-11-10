#include <iostream>
#include <vector>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

struct MemoryData {
    std::string memoryString;
    double previousMemory = 0.0;
};

struct CPUData {
    std::string cpuString;
    double currentCPU = 0.0;
    double previousTime = 0.0;
    double previousUtilization = 0.0;
};

// Signal handler for CTRL+C
void INThandler(int sig) {
    std::signal(sig, SIG_IGN);
    std::cout << "Do you really want to quit? [y/n]: ";
    char c = getchar();
    if (c == 'y' || c == 'Y') {
        exit(0);
    } else {
        std::signal(SIGINT, INThandler);
    }
}

// Signal handler for CTRL+Z
void interceptZ(int signal) {
    // Implement handling of CTRL+Z here, if needed.
}

// Utility function to check if a string is a digit
bool isDigit(const std::string &num) {
    return std::all_of(num.begin(), num.end(), ::isdigit) && !num.empty();
}

// Print system information
void printSystemInfo() {
    struct utsname sysinfo;
    uname(&sysinfo);
    std::cout << "### System Information ###\n"
              << "System Name: " << sysinfo.sysname << "\n"
              << "Machine Name: " << sysinfo.nodename << "\n"
              << "Version: " << sysinfo.version << "\n"
              << "Release: " << sysinfo.release << "\n"
              << "Architecture: " << sysinfo.machine << "\n"
              << "---------------------------------------\n";
}

void getMemory(int iteration, bool graphics, double *previousMemory, char *memoryString);
void getUser(char *userString);
void getCPU(int iteration, bool graphics, double *currentCPU, double *previousTime, double *previousUtilization, char *cpuString);

int main(int argc, char **argv) {
    bool system_f = false, user_f = false, graphics = false, sequential = false;
    int samples = 10, tdelay = 1;
    bool firstArg = false, secondArg = false;

    std::signal(SIGTSTP, interceptZ);
    std::signal(SIGINT, INThandler);

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--system" || arg == "-s") {
            system_f = true;
        } else if (arg == "--user" || arg == "-u") {
            user_f = true;
        } else if (arg == "--graphics" || arg == "-g") {
            graphics = true;
        } else if (arg == "--sequential" || arg == "-se") {
            sequential = true;
        } else if (arg.rfind("--samples=", 0) == 0 || arg.rfind("-s=", 0) == 0) {
            std::string num = arg.substr(arg.find('=') + 1);
            if (isDigit(num)) {
                samples = std::stoi(num);
                firstArg = true;
            }
        } else if (arg.rfind("--tdelay=", 0) == 0 || arg.rfind("-t=", 0) == 0) {
            std::string num = arg.substr(arg.find('=') + 1);
            if (isDigit(num)) {
                tdelay = std::stoi(num);
                secondArg = true;
            }
        } else if (isDigit(arg) && !firstArg && !secondArg) {
            samples = std::stoi(arg);
            firstArg = true;
        } else if (isDigit(arg) && firstArg && !secondArg) {
            tdelay = std::stoi(arg);
            secondArg = true;
        }
    }

    std::vector<std::string> memoryArray(samples), cpuArray(samples);
    MemoryData memoryData;
    CPUData cpuData;

    for (int i = 0; i < samples; i++) {
        std::string memoryString, userString, cpuString;
        int memoryPipe[2], userPipe[2], cpuPipe[2];

        if (pipe(memoryPipe) == -1 || pipe(userPipe) == -1 || pipe(cpuPipe) == -1) {
            std::cerr << "Failed to create pipe\n";
            exit(EXIT_FAILURE);
        }

        std::vector<pid_t> child_pids(3);

        for (int j = 0; j < 3; j++) {
            if ((child_pids[j] = fork()) == -1) {
                std::cerr << "Failed to fork\n";
            } else if (child_pids[j] == 0) { // Child process
                if (j == 0) { // Memory process
                    close(memoryPipe[0]);
                    getMemory(i, graphics, &memoryData.previousMemory, memoryData.memoryString.data());
                    write(memoryPipe[1], &memoryData, sizeof(memoryData));
                    close(memoryPipe[1]);
                } else if (j == 1) { // User process
                    close(userPipe[0]);
                    getUser(userString.data());
                    write(userPipe[1], userString.data(), userString.length() + 1);
                    close(userPipe[1]);
                } else if (j == 2) { // CPU process
                    close(cpuPipe[0]);
                    getCPU(i, graphics, &cpuData.currentCPU, &cpuData.previousTime, &cpuData.previousUtilization, cpuData.cpuString.data());
                    write(cpuPipe[1], &cpuData, sizeof(cpuData));
                    close(cpuPipe[1]);
                }
                exit(EXIT_SUCCESS);
            }
        }

        for (auto &pid : child_pids) {
            waitpid(pid, nullptr, 0); // Wait for each child to finish
        }

        if (sequential) {
            std::cout << "\n>>> iteration " << (i + 1) << "\n";
        } else {
            std::cout << "\033[H\033[2J\n"; // Clears screen and places cursor at top
            std::cout << "Nbr of samples: " << samples << " -- every " << tdelay << " secs\n";
        }

        struct rusage usage_info;
        getrusage(RUSAGE_SELF, &usage_info);
        std::cout << "Memory usage: " << usage_info.ru_maxrss << " kilobytes\n"
                  << "---------------------------------------\n";

        if (user_f && !system_f) {
            std::cout << "### Sessions/users ###\n" << userString << "\n";
        } else if (!user_f && system_f) {
            // printMemory(i, samples, sequential, memoryArray);
            std::cout << "---------------------------------------\n";
            // printCPU(i, samples, graphics, sequential, cpuData.currentCPU, cpuArray);
        } else {
            // printMemory(i, samples, sequential, memoryArray);
            std::cout << "---------------------------------------\n";
            std::cout << "### Sessions/users ###\n" << userString << "\n"
                      << "---------------------------------------\n";
            // printCPU(i, samples, graphics, sequential, cpuData.currentCPU, cpuArray);
        }

        sleep(tdelay); // Wait for tdelay seconds
    }

    std::cout << "---------------------------------------\n";
    printSystemInfo();

    return 0;
}
