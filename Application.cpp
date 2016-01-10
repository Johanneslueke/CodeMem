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



/*
 * 
 */
int main(int argc, char** argv) {

    ////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////
    ConfigSettingString ProgramAPI_Location("PlatformAPI", "blah", "/media/johannes/SecretSoftware/Programms/Handmade/PlatformAPI/dist/Debug/GNU-Linux-x86/libPlatformAPI.so");
    ConfigSettingInt StorageSize("PermanentStorage", "Size of this kind of Storage", Megabytes((uint64) 512));
    ConfigSettingInt StorageSizeTrasient("TransientStorage", "Size of this kind of Storage", Gigabytes((uint64) 1));

    ConfigSettingInt verbosity("Verbosity", "Defines the verbosity level of logging operations", 2, 0, 2);
    ConfigSettingInt threadcount("ThreadNumber", "Number of available work threads", 0, 0, 20);


    ////////////////////////////////////////////////////////////////////////////
    //Setting up the Application
    //Reserve Memory
    ////////////////////////////////////////////////////////////////////////////
    void* BaseAddress = (void*) (Terabytes(1));
    app* Program = nullptr;

    memory_arena TotalArena;
    memory_arena WorkQueueHigh;
    memory_arena WorkQueueLow;
    app_memory Memory;
    //------------------------------------------------------------------------//

    Memory.PlatformAPI = nullptr;
    Memory.PermanentStorage = nullptr;
    Memory.TransientStorage = nullptr;
    Memory.PermanentStorageSize = *head->FindSetting("PermanentStorage");
    Memory.TransientStorageSize = *head->FindSetting("TransientStorage");

    //-----------Load shared object on linux or dll on windoof------------------
    platform_API API;
#if defined(__win32__)
#elif defined(__apple__)
#elif defined(__linux__)
    std::cerr << "Load API\n";
    void* CodeHandle = nullptr;

    if ((CodeHandle = dlopen(*head_s->FindSetting("PlatformAPI"), RTLD_LAZY)) != nullptr) {
        API.AllocateMemory = (platform_allocate_memory*) dlsym(CodeHandle, "platform_allocate_memory");
        API.Allocate_Job = (platform_allocate_Job*) dlsym(CodeHandle, "platform_allocate_Job");
        API.CreateJob = (platform_create_Job*) dlsym(CodeHandle, "platform_create_Job");
        API.CreateJobAsChild = (platform_create_Job_as_Child*) dlsym(CodeHandle, "platform_create_Job_as_Child");
        API.DeallocateMemory = (platform_deallocate_memory*) dlsym(CodeHandle, "platform_deallocate_memory");
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
        uint64 TotalStorageSize = (uint64) (Memory.PermanentStorageSize + Memory.TransientStorageSize);
        if ((Memory.PlatformAPI != nullptr)) {
            Memory.PermanentStorage = Memory.PlatformAPI->AllocateMemory(TotalStorageSize, BaseAddress);
            if(Memory.PermanentStorage == nullptr)
                throw std::bad_alloc();
        } else {
            throw std::runtime_error("Platform API is not loaded!");
        }


        //Check for validity of new memory
        Assert(Memory.PermanentStorage);

        //The first few bits are exclusive for the app structure
        Program = (app *) Memory.PermanentStorage;

        if (!Program->IsInitialized) {

            InitializeArena(&Program->ApplicationMemoryArena,
                    Memory.PermanentStorageSize + Memory.TransientStorageSize - sizeof (app),
                    (uint8*) Memory.PermanentStorage + sizeof (app));

            

            //            std::cerr << "WorkqueueHigh " << (((Job*) (WorkQueueHigh.Base + sizeof (platform_work_queue)))->padding) << "\n";
            //            std::cerr << "In Memory     " << Memory.HighPriorityQueue + sizeof (platform_work_queue) << "\n\n";

            Program->IsInitialized = true;

        }

        //Calculate Base address relative to the PermanentStorage segment
        Memory.TransientStorage = (uint8*) (Memory.PermanentStorage) + Memory.PermanentStorageSize;

        Assert(sizeof (transient_state) <= Memory.TransientStorageSize);
        transient_state *TranState = (transient_state *) Memory.TransientStorage;

        if (!TranState->IsInitialized) {
            InitializeArena(&TranState->TranArena, Memory.TransientStorageSize - sizeof (transient_state),
                    (uint8 *) Memory.TransientStorage + sizeof (transient_state));

            //SubArena(&Program->ApplicationMemoryArena, &TotalArena, Megabytes(1));
            SubArena(&WorkQueueHigh, &Program->ApplicationMemoryArena, Megabytes(64));
            SubArena(&WorkQueueLow, &Program->ApplicationMemoryArena, Megabytes(128));
            
            TranState->HighPriorityQueue = Memory.HighPriorityQueue;
            TranState->LowPriorityQueue = Memory.LowPriorityQueue;
            for (uint32 TaskIndex = 0;
                    TaskIndex < ArrayCount(TranState->Tasks);
                    ++TaskIndex) {
                task_with_memory *Task = TranState->Tasks + TaskIndex;

                Task->BeingUsed = false;
                SubArena(&Task->Arena, &TranState->TranArena, Megabytes(1));

                Memory.HighPriorityQueue = (platform_work_queue*) WorkQueueHigh.Base;
                Memory.LowPriorityQueue  = (platform_work_queue*) WorkQueueLow.Base;
            }

        }
    } catch (std::bad_alloc& e) {
        std::cerr << e.what()<<std::endl;
    }catch(std::runtime_error& e)
    {
        std::cerr<<e.what()<<std::endl;
    }

    ////////////////////////////////////////////////////////////////////////////
    //Initialize the application
    ////////////////////////////////////////////////////////////////////////////

    int a[Megabytes(1)];


    ////////////////////////////////////////////////////////////////////////////
    //Run the application
    ////////////////////////////////////////////////////////////////////////////
    std::cerr << "Memorybase             Address: " << BaseAddress << "\n";
    std::cerr << "PermanentStorage       Address: " << Memory.PermanentStorage << "\n";
    std::cerr << "TransientStorage       Address: " << Memory.TransientStorage << "\n";
    std::cerr << "ApplicationMemoryArena Address: " << (void*) Program->ApplicationMemoryArena.Base << "\n";
    std::cerr << "ApplicationMemoryArena Size   : " << Program->ApplicationMemoryArena.Size / 1024 / 1024 << " MB\n";
    std::cerr << "ApplicationMemoryArena Used Memory:" << Program->ApplicationMemoryArena.Used / 1024 / 1024 << "MB\n";
    std::cerr << "Sizeof App             Address: " << (void*) sizeof (app) << "\n";


    std::cerr << "\n";


    //Memory.PlatformAPI->RunJob(nullptr);
    //Memory.PlatformAPI->WaitJob(nullptr);
    ////////////////////////////////////////////////////////////////////////////
    //Clear up the application
    ////////////////////////////////////////////////////////////////////////////
    Memory.PlatformAPI->DeallocateMemory(Memory.TransientStorage, Memory.TransientStorageSize);
    Memory.PlatformAPI->DeallocateMemory(Memory.PermanentStorage, Memory.PermanentStorageSize);
    Memory.PlatformAPI = nullptr;
    dlclose(CodeHandle);





    return 0;
}

