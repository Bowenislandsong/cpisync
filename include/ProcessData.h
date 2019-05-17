//
// Created by Bowen on 11/14/18.
//

#ifndef CPISYNCLIB_PROCESSDATA_H
#define CPISYNCLIB_PROCESSDATA_H

#if __APPLE__
#include<mach/mach.h>
#include <iomanip>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>

#include <sys/param.h>
#include <sys/mount.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#elif __linux__ // TODO: Implement Libraries for linux
    #include "sys/types.h"
    #include "sys/sysinfo.h"
    #include "stdlib.h"
    #include "stdio.h"

#endif

#include <cstring>
#include <string>
#include <gperftools/heap-profiler.h> // heap profiler
static size_t NOT_SET = 0; // not set parameters are not used

struct Resources{
    struct timespec start_time, finish_time;
    double TimeElapsed;
    size_t VmemUsed;
};


// use PERFTOOLS_VERBOSE=-4 at commandline to get rid of gperf tool printing

using namespace std;
#if __APPLE__
inline size_t getHeapSize() {
    size_t mem_size = 0;
    char *tmp = GetHeapProfile();
    if (IsHeapProfilerRunning() and tmp) {
        string str(tmp);
        if (!str.empty()) {
            str = str.substr(0, str.find("["));
            str = str.substr(str.find(":") + 1);
            str = str.substr(str.find(":") + 1);
            mem_size = stoul(str) - 1048576; // a hard thershold from google
        }
//        FLAGS_verbose=-1;

//        HeapProfilerStop(); // clear cache
//        HeapProfilerStart("Resource Monitor"); // restart counting
    }
    delete tmp;
    return mem_size;
}

inline double getFinishTime(Resources& res){
    double str_time;
    clock_gettime(CLOCK_MONOTONIC, &res.finish_time);
    str_time = (res.finish_time.tv_sec - res.start_time.tv_sec);
    str_time += (res.finish_time.tv_nsec - res.start_time.tv_nsec) / 1000000000.0;
    return str_time;
}

inline void initResources(Resources& res){
    clock_gettime(CLOCK_MONOTONIC, &res.start_time);
    if(!IsHeapProfilerRunning())
        HeapProfilerStop();
    HeapProfilerStart("Resource Monitor"); // comment out to disable heap profiling
    res.VmemUsed = 0;
}

inline bool resourceMonitor(Resources& res, double cap_secs, size_t cap_bytes){
    if(!IsHeapProfilerRunning()){
        return (cap_secs > getFinishTime(res));
    }else {
        return (cap_secs > getFinishTime(res)) and (cap_bytes > (res.VmemUsed=getHeapSize()));
    }
}

inline void resourceReport(Resources& res){
    res.TimeElapsed = getFinishTime(res);
    if(!IsHeapProfilerRunning()) return;
    res.VmemUsed+=getHeapSize();
    HeapProfilerStop();
}

 // VM currently Used by my process


inline void printMemUsage() {
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS == task_info(mach_task_self(),
                                  TASK_BASIC_INFO, (task_info_t) &t_info,
                                  &t_info_count)) {

        //cout<< std::setprecision(std::numeric_limits<long double>::digits10 + 1)<< t_info.virtual_size*(long double)1.25e-10<<endl;
        cout << "Process Resident size:" << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
             << t_info.resident_size * (long double) 1.25e-10
             << " virtual size:" << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
             << t_info.virtual_size * (long double) 1.25e-10 << endl;


        struct statfs stats;
        if (0 == statfs("/", &stats)) {
            cout << "Total MEM left: " << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
                 << (long double) stats.f_bsize * stats.f_bavail * 1.25e-10 << endl;
        }
        //process resident diff =  879087616 == 0.109 GB
        //virtual size diff = 59762421760 == 7.470 GB
        //start V - size = 0.98
        //end V - size = 8.45GB
    }
}
#elif __linux

inline int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

inline int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
inline void printMemUsage() {
    cout<<getValue()<<endl;
}
#endif
/**
 * Monitor Mem based on a hard threshold of 2GB
 * @param virtualMemUsed if not set, set initial value, else monitor mem growth
 * @param MaxMem Maximum amount of mem allowed before returning false
 * @return True if it is still within threshold, false if exceeds
 */
inline bool virtualMemMonitor(size_t & virtualMem=NOT_SET, const int MaxMem = 2e9/* 1 GB */ ) {
#if __APPLE__

    struct task_basic_info t_info;
    struct statfs stats;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS == task_info(mach_task_self(),
                                  TASK_BASIC_INFO, (task_info_t) &t_info,
                                  &t_info_count) && 0 == statfs("/", &stats)) {

        //cout<< std::setprecision(std::numeric_limits<long double>::digits10 + 1)<< t_info.virtual_size*(long double)1.25e-10<<endl; // mem print
//        if (t_info.virtual_size - virtualMemUsed + t_info.virtual_size> stats.f_bsize * stats.f_bavail) return false; // return if no Virtual mem available
        if (virtualMem == NOT_SET) {//set initial mem
            virtualMem = (size_t) t_info.virtual_size;
        } else {
            if (t_info.virtual_size - virtualMem > MaxMem) return false;
        }
        return true;
    }
#elif __linux
    struct sysinfo memInfo;

    sysinfo (&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    //Add other values in next statement to avoid int overflow on right hand side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

//    if (memInfo.totalram - memInfo.freeram - virtualMem > memInfo.freeram) return false;  //prevent out of mem
//    virtualMem = memInfo.totalram - memInfo.freeram;
//    //Add other values in next statement to avoid int overflow on right hand side...
//    virtualMem += memInfo.totalswap - memInfo.freeswap;
//    virtualMem *= memInfo.mem_unit;

        if (virtualMem == NOT_SET) {//set initial mem
            // virtualMem = memInfo.totalram - memInfo.freeram;
            // virtualMem += memInfo.totalswap - memInfo.freeswap;
            // virtualMem *= memInfo.mem_unit;
            virtualMem = getValue();
        } else {
            // size_t usedmem = memInfo.totalram - memInfo.freeram;
            // usedmem += memInfo.totalswap - memInfo.freeswap;
            // usedmem *= memInfo.mem_unit;
            if (getValue() - virtualMem > MaxMem) return false;
        }

#endif
    return true;
}

//inline void printRAMUsage() { //RAM Currently Used
//    vm_size_t page_size;
//    mach_port_t mach_port;
//    mach_msg_type_number_t count;
//    vm_statistics64_data_t vm_stats;
//
//    mach_port = mach_host_self();
//    count = sizeof(vm_stats) / sizeof(natural_t);
//    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
//        KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
//                                          (host_info64_t) &vm_stats, &count)) {
//        long long free_memory = (int64_t) vm_stats.free_count * (int64_t) page_size;
//
//        long long used_memory = ((int64_t) vm_stats.active_count +
//                                 (int64_t) vm_stats.inactive_count +
//                                 (int64_t) vm_stats.wire_count) * (int64_t) page_size;
//        //printf("Total free memory: %f used memory: %f\n", free_memory*1.25e-10, used_memory*1.25e-10);
//        cout<< "Total free memory: " << std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<free_memory*(long double)1.25e-10
//        <<" Used Mem: "<< std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<used_memory*(long double)1.25e-10<<endl;
//        // Total free memory diff = -126529536 == -0.0158
//        // used mem diff = 443351040 == 0.055 GB
//    }
//
//}

//inline void printRAMavl(){ // Total RAM Available
//    int mib[2];
//    int64_t physical_memory;
//    mib[0] = CTL_HW;
//    mib[1] = HW_MEMSIZE;
//    auto length = sizeof(int64_t);
//    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
//    cout << "Total RAM Available: "+to_string(physical_memory)+" length:"+to_string(length) <<endl;
//}

#endif //CPISYNCLIB_PROCESSDATA_H

//Process Resident size:0.293176 virtual size:0.981849
//Total free memory: 0.002555 used memory: 0.907750
//
//Process Resident size:0.462133 virtual size:8.472152
//Total free memory: 0.002353 used memory: 0.961219


//Process Resident size:0.0004761600000000000297 virtual size:0.5505469440000000343
//Total free memory: 0.01874380800000000117 Used Mem: 0.7587348480000000472
//
//Process Resident size:0.006219776000000000387 virtual size:0.5526440960000000344
//Total free memory: 0.01874380800000000117 Used Mem: 0.7587348480000000472

//Process Resident size:0.128721920000000008 virtual size:0.7821824000000000487
//Total free memory: 0.002358784000000000147 Used Mem: 0.7626142720000000475
//
//Process Resident size:0.3350896640000000209 virtual size:8.442508800000000526
//Total free memory: 0.002508288000000000156 Used Mem: 0.8493696000000000529

