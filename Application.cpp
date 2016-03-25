/* 
 * File:   CSkript.cpp
 * Author: johannes
 *
 * Created on 20 December 2015, 13:54
 */

#include <cstdlib>
#include <sys/mman.h>
#include <assert.h>
#include <iostream>

#include <dlfcn.h>

using namespace std;

#include "include/Application.h"
#include "include/JobSystem.h"
#include "include/ConfigSettingInt.h"
#include "include/ConfigSettingString.h"
#include <x86intrin.h>



/*
 * 
 */
int main(int argc, char** argv) {

    ////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////
    ConfigSettingString ProgramAPI_Location("PlatformAPI", "blah", "../build/libPlatformAPI.so");
    ConfigSettingInt StorageSize("PermanentStorage", "Size of this kind of Storage", Megabytes((uint64) 512));
    ConfigSettingInt StorageSizeTrasient("TransientStorage", "Size of this kind of Storage", Gigabytes((uint64) 1));

    ConfigSettingInt verbosity("Verbosity", "Defines the verbosity level of logging operations", 2, 0, 2);
    ConfigSettingInt threadcount("ThreadNumber", "Number of available work threads", 0, 0, 20);


    ////////////////////////////////////////////////////////////////////////////
    //Setting up the Application
    //Reserve Memory
    ////////////////////////////////////////////////////////////////////////////
    void* BaseAddress = (void*) (Terabytes(1));
    App* Program = nullptr;

    AppMemory   Memory;
    MemoryArena TotalArena;
    //------------------------------------------------------------------------//

    Memory.PlatformAPI = nullptr;
    Memory.PermanentStorage = nullptr;
    Memory.PermanentStorageSize = *head->FindSetting("PermanentStorage");

    //-----------Load shared object on linux or dll on windoof------------------
    platform_API API;
#if defined(__win32__)
#elif defined(__apple__)
#elif defined(__linux__)
    std::cerr << "Load API\n";
    void* CodeHandle = nullptr;

    if ((CodeHandle = dlopen(*head_s->FindSetting("PlatformAPI"), RTLD_LAZY)) != nullptr) {

        API.AllocateMemory = (platform_allocate_memory*) dlsym(CodeHandle, "platform_allocate_memory");
        API.DeallocateMemory = (platform_deallocate_memory*) dlsym(CodeHandle, "platform_deallocate_memory");

        API.AllocVirtMem = (platform_allocate_memory*) dlsym(CodeHandle, "platform_allocate_virtual_memory");
        API.DeallocVirtMem = (platform_deallocate_memory*) dlsym(CodeHandle, "platform_deallocate_virtual_memory");

        API.InitializeArena = (platform_create_arena*) dlsym(CodeHandle, "platform_create_arena");

        API.Allocate_Job = (platform_allocate_Job*) dlsym(CodeHandle, "platform_allocate_Job");
        API.CreateJob = (platform_create_Job*) dlsym(CodeHandle, "platform_create_Job");
        API.CreateJobAsChild = (platform_create_Job_as_Child*) dlsym(CodeHandle, "platform_create_Job_as_Child");
        API.ExecuteJob = (platform_execute_Job*) dlsym(CodeHandle, "platform_execute_Job");
        API.FinishJob = (platform_finish_Job*) dlsym(CodeHandle, "platform_finish_Job");
        API.GetJob = (platform_get_Job*) dlsym(CodeHandle, "platform_get_Job");
        API.WaitJob = (platform_wait_Job*) dlsym(CodeHandle, "platform_wait_Job");
        API.RunJob = (platform_run_Job*) dlsym(CodeHandle, "platform_run_Job");
        API.QueuePop = (platform_work_queue_pop*) dlsym(CodeHandle, "platform_work_queue_pop");
        API.QueuePush = (platform_work_queue_push*) dlsym(CodeHandle, "platform_work_queue_push");
        API.QueueSteal = (platform_work_queue_steal*) dlsym(CodeHandle, "platform_work_queue_steal");

        Memory.PlatformAPI = &API;
       std::cerr << "API is loaded\n";

    } else {
        std::cerr << dlerror() << "\n";
    }
#else
#error "Not supported platform"
#endif

    try {
        //Ask OS for certain amount of Memory of TotalStorage Size
        uint64 TotalStorageSize = (uint64) (Memory.PermanentStorageSize);
        if ((Memory.PlatformAPI != nullptr)) {
            Memory.PermanentStorage = Memory.PlatformAPI->AllocVirtMem(TotalStorageSize, BaseAddress);
            if(Memory.PermanentStorage == nullptr)
                throw std::bad_alloc();
        } else {
            throw std::runtime_error("Platform API is apparently not loaded!");
        }


        std::cerr<<"- "<<Memory.PermanentStorage<<"\n";
        //Check for validity of new memory (a second time is always better)
        Assert(Memory.PermanentStorage);

        //The first few bits are exclusive for the app structure
        Program = (App *) Memory.PermanentStorage;
        Program->Platform = &API;
        Program->IsInitialized = false;

        if (!Program->IsInitialized) {

            Assert(Program->Platform->InitializeArena);
            Program->Platform->InitializeArena(
                    Memory.PermanentStorageSize - sizeof (App),
                    &Program->ApplicationMemoryArena);


                Program->IsInitialized = true;

            }

        }
     catch (std::bad_alloc& e) {
        std::cerr << e.what()<<std::endl;
    }catch(std::runtime_error& e)
    {
        std::cerr<<e.what()<<std::endl;
    }

    ////////////////////////////////////////////////////////////////////////////
    //Initialize the application
    ////////////////////////////////////////////////////////////////////////////

    int a[Megabytes(1)];
    void* p = Program->Platform->AllocateMemory({Megabytes(1)},&Program->ApplicationMemoryArena);
    void* p2 = Program->Platform->AllocateMemory({Megabytes(1)},&Program->ApplicationMemoryArena);
    std::cerr<<"p                                   : "<<p<<"\n";
    std::cerr<<"p2                                  : "<<p2<<"\n";


    ////////////////////////////////////////////////////////////////////////////
    //Run the application
    ////////////////////////////////////////////////////////////////////////////
    std::cerr << "Memorybase             Address    : " << BaseAddress << "\n";
    std::cerr << "PermanentStorage       Address    : " << Memory.PermanentStorage << "\n";
    std::cerr << "ApplicationMemoryArena Address    : " << (void*) Program->ApplicationMemoryArena.StartAdress << "\n";
    std::cerr << "ApplicationMemoryArena Size       : " << Program->ApplicationMemoryArena.Size / 1024 / 1024 << " MB\n";
    std::cerr << "ApplicationMemoryArena Used Memory: " << Program->ApplicationMemoryArena.Used / 1024 / 1024 << "MB\n";
    std::cerr << "Sizeof App             Address    : " << (void*) sizeof (App) << "\n\n";


    //std::cerr << ((platform_CycleCount*) dlsym(CodeHandle, "platform_CycleCount"))()<<"\n";
    //std::cerr << __rdtsc()<<"\n";


    //Memory.PlatformAPI->RunJob(nullptr);
    //Memory.PlatformAPI->WaitJob(nullptr);
    ////////////////////////////////////////////////////////////////////////////
    //Clear up the application
    ////////////////////////////////////////////////////////////////////////////
    Memory.PlatformAPI->DeallocVirtMem(Memory.PermanentStorage, Memory.PermanentStorageSize);
    Memory.PlatformAPI = nullptr;
    dlclose(CodeHandle);





    return 0;
}
