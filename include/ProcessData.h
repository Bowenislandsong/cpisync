//
// Created by Bowen on 11/14/18.
//

#ifndef CPISYNCLIB_PROCESSDATA_H
#define CPISYNCLIB_PROCESSDATA_H

#if __APPLE__
#include<mach/mach.h>

#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#elif __linux // TODO: Implement Libraries for linux
#endif

using namespace std;
inline void printMemUsage() { // VM currently Used by my process
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS == task_info(mach_task_self(),
                                  TASK_BASIC_INFO, (task_info_t) &t_info,
                                  &t_info_count)) {

        //cout<< std::setprecision(std::numeric_limits<long double>::digits10 + 1)<< t_info.virtual_size*(long double)1.25e-10<<endl;
        cout<<"Process Resident size:" << std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<t_info.resident_size*(long double)1.25e-10
        << " virtual size:" << std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<t_info.virtual_size*(long double)1.25e-10<<endl;

        //process resident diff =  879087616 == 0.109 GB
        //virtual size diff = 59762421760 == 7.470 GB
        //start V - size = 0.98
        //end V - size = 8.45GB
    }
}

inline void printRAMUsage() { //RAM Currently Used
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                          (host_info64_t) &vm_stats, &count)) {
        long long free_memory = (int64_t) vm_stats.free_count * (int64_t) page_size;

        long long used_memory = ((int64_t) vm_stats.active_count +
                                 (int64_t) vm_stats.inactive_count +
                                 (int64_t) vm_stats.wire_count) * (int64_t) page_size;
        //printf("Total free memory: %f used memory: %f\n", free_memory*1.25e-10, used_memory*1.25e-10);
        cout<< "Total free memory: " << std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<free_memory*(long double)1.25e-10
        <<" Used Mem: "<< std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<used_memory*(long double)1.25e-10<<endl;
        // Total free memory diff = -126529536 == -0.0158
        // used mem diff = 443351040 == 0.055 GB
    }

}

inline void printRAMavl(){ // Total RAM Available
    int mib[2];
    int64_t physical_memory;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    auto length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
    cout << "Total RAM Available: "+to_string(physical_memory)+" length:"+to_string(length) <<endl;
}
//TODO: 
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