#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <cmath>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <sys/utsname.h>
#include <unistd.h>

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

// Prints memory information
void getMemory(int i, bool graphics, double &prevMemory, std::string &memoryString) {
    struct sysinfo memory;
    sysinfo(&memory);

    double totalRam = static_cast<double>(memory.totalram) / 1024 / 1024 / 1024; // GB
    double freeRam = static_cast<double>(memory.freeram) / 1024 / 1024 / 1024;   // GB
    double totalSwap = static_cast<double>(memory.totalswap) / 1024 / 1024 / 1024;
    double freeSwap = static_cast<double>(memory.freeswap) / 1024 / 1024 / 1024;

    double totalVirtual = totalRam + totalSwap;
    double usedVirtual = totalVirtual - (freeRam + freeSwap);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << (totalRam - freeRam) << " GB / " << totalRam << " GB -- "
        << usedVirtual << " GB / " << totalVirtual << " GB";
    memoryString = oss.str();

    if (graphics) {
        double currentMemory = usedVirtual;
        double diffMemory = currentMemory - prevMemory;
        if (i == 0) diffMemory = 0;

        oss << "   |";
        int numChars = static_cast<int>(std::abs(diffMemory) * 10); // number of characters for graphic

        if (diffMemory > 0) {
            oss << std::string(numChars, '#') << '*';
        } else if (diffMemory < 0) {
            oss << std::string(numChars, ':') << '@';
        } else if (std::abs(diffMemory) < 0.01) {
            oss << 'o';
        }

        oss << " " << std::abs(diffMemory) << " (" << currentMemory << ")";
        memoryString = oss.str();
        prevMemory = currentMemory;
    }
}

// Prints memory data for each sample
void printMemory(int i, int samples, bool sequential, const std::vector<std::string> &memoryArray) {
    std::cout << "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n";
    if (sequential) {
        for (int j = 0; j < i; j++) std::cout << "\n";
        std::cout << memoryArray[i] << "\n";
    } else {
        for (int j = 0; j <= i; j++) std::cout << memoryArray[j] << "\n";
    }
    for (int j = samples - 1; j > i; j--) std::cout << "\n";
}

// Collects active user information
void getUser(std::string &userString) {
    struct utmp *user;
    setutent();
    while ((user = getutent()) != nullptr) {
        if (user->ut_type == USER_PROCESS) {
            userString += "\n" + std::string(user->ut_user) + "      " + user->ut_line + " " + user->ut_host;
        }
    }
    endutent();
}

// Collects CPU usage information
void getCPU(int i, bool graphics, double &currCPU, double &prevTime, double &prevUtilization, std::string &cpuString) {
    std::ifstream file("/proc/stat");
    long int user, nice, system, idle, iowait, irq, softirq;
    file >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    file.close();

    double currTime = user + nice + system + idle + iowait + irq + softirq;
    double currUtilization = currTime - idle;
    double totalUtilization = 0;
    
    if (i > 0) {
        totalUtilization = 100 * ((currUtilization - prevUtilization) / (currTime - prevTime));
    }

    if (graphics) {
        cpuString = "\n        ";
        int numChars = static_cast<int>(std::abs(totalUtilization)) + 1;
        cpuString += std::string(numChars, '|') + " " + std::to_string(totalUtilization);
    }

    currCPU = totalUtilization;
    prevTime = currTime;
    prevUtilization = currUtilization;
}

// Prints CPU usage data for each sample
void printCPU(int i, int samples, bool graphics, bool sequential, double currentCPU, const std::vector<std::string> &cpuArray) {
    std::cout << "Number of cores: " << sysconf(_SC_NPROCESSORS_ONLN) << "\n";
    std::cout << "Total CPU use = " << currentCPU << "%\n";
    if (graphics) {
        if (sequential) {
            for (int j = 0; j <= i; j++) std::cout << "\n";
            std::cout << cpuArray[i];
        } else {
            for (int j = 0; j <= i; j++) std::cout << cpuArray[j];
        }
        for (int j = samples; j > i; j--) std::cout << "\n";
    }
}
