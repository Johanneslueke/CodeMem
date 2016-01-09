/* 
 * File:   CSkript.h
 * Author: johannes
 *
 * Created on 20 December 2015, 09:33
 * 
 * 
 * Inspired by http://blog.molecular-matters.com/ and 
 * http://mollyrocket.com/casey/
 * 
 */

#ifndef CSKRIPT_H
#define	CSKRIPT_H

using namespace std;

#include "Application_Memory.h"
#include "Platform.h"




typedef struct ApplicationMemory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(jojo): REQUIRED to be cleared to zero at startup

    uint64 TransientStorageSize;
    void *TransientStorage; // NOTE(jojo): REQUIRED to be cleared to zero at startup

    uint64 DebugStorageSize;
    void *DebugStorage; // NOTE(jojo): REQUIRED to be cleared to zero at startup

    WorkQueue *HighPriorityQueue;
    WorkQueue *LowPriorityQueue;

    b32 ExecutableReloaded;
    platform_API* PlatformAPI;
} app_memory;

typedef struct Application
{
    bool            IsInitialized;
    platform_API*   Platform;
    memory_arena    ApplicationMemoryArena;
}app;

struct task_with_memory
{
    b32 BeingUsed;
    b32 DependsOnGameMode;
    memory_arena Arena;

    temporary_memory MemoryFlush;
};

struct transient_state
{
    bool32 IsInitialized;
    memory_arena TranArena;    

    task_with_memory Tasks[4];

    //game_assets *Assets;

    uint32 GroundBufferCount;
    //ground_buffer *GroundBuffers;
    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;

};

static platform_API Platform;

static task_with_memory *BeginTaskWithMemory(transient_state *TranState, b32 DependsOnGameMode);
static void EndTaskWithMemory(task_with_memory *Task);



bool LinuxCopyFile(const std::string& SourceSOName, const std::string& TempSOName);
timespec LinuxGetLastWriteTime(const std::string& filename);


template<typename Code>
Code* AppLoadCode(const std::string& source,const std::string& worksource);

template<typename Code>
void AppUnloadCode(Code*);





#endif 	/* CSKRIPT_H */

