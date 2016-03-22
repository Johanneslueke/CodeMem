/* 
 * File:   Platform.h
 * Author: johannes
 *
 * Created on 23 December 2015, 22:23
 * 
 * 
 * 
 * Inspired by http://blog.molecular-matters.com/ and 
 * http://mollyrocket.com/casey/
 * 
 * 
 */

#ifndef PLATFORM_H
#define	PLATFORM_H

#include <atomic>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <ctime>
#include <vector>
#include <functional>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;

typedef uint8* MemoryAdress;

#define PointerToU32(Pointer) ((u32)(memory_index)(Pointer))

#define Bytes(Value)     (Value)
#define Kilobytes(Value) (Bytes(Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

#ifdef DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression) if(!(Expression)) \
         {std::cerr<<"Error: "<<__FILE__<<"->"<<__LINE__<<" "<<"\n";}
#endif

typedef struct platform_file_handle {
    b32 NoErrors;
    void *Platform;
} platform_file_handle;

typedef struct platform_file_group {
    u32 FileCount;
    void *Platform;
} platform_file_group;

typedef enum platform_file_type {
    PlatformFileType_AssetFile,
    PlatformFileType_SavedGameFile,

    PlatformFileType_Count,
} platform_file_type;


struct platform_work_queue;
struct platform_job;

template <typename T, typename S>
struct parallel_for_job_data {
    typedef T DataType;
    typedef S SplitterType;

    parallel_for_job_data(DataType* data, unsigned int count, void (*function)(DataType*, unsigned int), const SplitterType& splitter)
    : data(data)
    , count(count)
    , function(function)
    , splitter(splitter) {
    }

    DataType* data;
    unsigned int count;
    void (*function)(DataType*, unsigned int);
    SplitterType splitter;
};


struct  MemoryArena
{
    MemoryAdress StartAdress;
    memory_index Used;
    memory_index Size;
    uint64 ID;
    /* data */
};

typedef void (*JobFunction)(platform_job*, const void*);


////////File operations/////////////////////////////////////////////////////////
#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group name(platform_file_type Type)
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);

#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);

#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup)
typedef PLATFORM_OPEN_FILE(platform_open_next_file);

#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Source, u64 Offset, u64 Size, void *Dest)
typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);

#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
typedef PLATFORM_FILE_ERROR(platform_file_error);

#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)

////////Queue operations////////////////////////////////////////////////////////
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_job *job, char* Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

#define PLATFORM_WORK_QUEUE_PUSH(name) void name(const platform_job* job) 
typedef PLATFORM_WORK_QUEUE_PUSH(platform_work_queue_push);

#define PLATFORM_WORK_QUEUE_POP(name) platform_job* name(void) 
typedef PLATFORM_WORK_QUEUE_POP(platform_work_queue_pop);
typedef PLATFORM_WORK_QUEUE_POP(platform_work_queue_steal);

////////Memory operations///////////////////////////////////////////////////////
#define PLATFORM_ALLOCATE_MEMORY(name) void* name(memory_index Size,void* adress) 
/**
* \name platform_allocate_virtual_memory
* \param memory_index Size
* \param void* Memory
* 
* This function wraps around an OS function which allocate virtual memory
* from the system i.e. under the linux platform "mmap" does the job. The void* Memory
* pointer is the start adress of the memory you want to allocate and memory_index
* is an typedef for an std::size_t and tells the OS to which point it should allocate
* new memory. If the given pointer to void is an nullptr, the OS choose randomly the start
* adress for you.
*/ 
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_virtual_memory);
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

/**
* \name platform_create_arena
* \param memory_index Size
* \param void* Memory
*
* This function creates an arena from any given startadress. The startadress
* cannot be an nullptr and must point into pre-allocated memory namespace 
* i.e. with "platform_allocate_virtual_memory". It does NOT check if the available
* pre-allocated memory does fit the requested size parameter. If the requested Size
* of the arena is bigger than the available pre-allocated memory it will result an 
* an Segmentation Fault later on!!!!
*
* 
* It returns an pointer of void which must be casted into a struct of type MemoryArena.
* The returned MemoryArena actually takes space from the requested memory. This results 
* that the requested MemoryArena is technically slightly bigger. The effectiv allocated
* arena size is equal to (requested Size)+(sizeof(struct MemoryArena)).
*
* \void* aka struct to MemoryArena 
*/
typedef PLATFORM_ALLOCATE_MEMORY(platform_create_arena);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(void *Memory,memory_index Size)
/**
* \name platform_deallocate_virtual_memory
* \param void* Memory
* \param memory_index Size
* 
* This function wraps around an OS function which deallocate virtual memory
* from the system i.e under the linux platform "munmap" does the job. The void*
* pointer is the start adress of memory you want to deallocate and memory_index
* is an typedef for an std::size_t which determined last adress of the memory
* relativ to the starting adress.
*/ 
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_virtual_memory);

/**
* \name platform_deallocate_virtual_memory
* \param void* Memory
* \param memory_index Size
*
*/
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

/**
* \name platform_delete_arena
* \param void* Memory
* \param memory_index Size
* 
* This function takes an struct of type MemoryArena casted into an pointer of
* void. The second parameter is useless. After executing this Function the given
* pointer of void is invalid and cannot be used afterwards as a MemoryArena.
*/ 
typedef PLATFORM_DEALLOCATE_MEMORY(platform__delete_arena);


typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, char *Data);
typedef void platform_complete_all_work(platform_work_queue *Queue);

////////Jobsystem specific operations///////////////////////////////////////////
#define PLATFORM_ALLOCATE_JOB(name) platform_job* name(void) 
typedef PLATFORM_ALLOCATE_JOB(platform_allocate_Job);

#define PLATFORM_CREATE_JOB(name) platform_job* name(platform_work_queue* workqueue,platform_work_queue_callback* func) 
typedef PLATFORM_CREATE_JOB(platform_create_Job);

#define PLATFORM_CREATE_JOB_AS_CHILD(name) platform_job* name(platform_work_queue* workqueue,platform_job* parent,platform_work_queue_callback* func) 
typedef PLATFORM_CREATE_JOB_AS_CHILD(platform_create_Job_as_Child);

#define PLATFORM_GET_JOB(name) platform_job* name(platform_work_queue* workqueue)
typedef PLATFORM_GET_JOB(platform_get_Job);

#define PLATFORM_FUNC_JOB(name) void name(platform_job* job)
typedef PLATFORM_FUNC_JOB(platform_run_Job);
typedef PLATFORM_FUNC_JOB(platform_execute_Job);
typedef PLATFORM_FUNC_JOB(platform_wait_Job);
typedef PLATFORM_FUNC_JOB(platform_finish_Job);

#define PLATFORM_CYCLECOUNT(name)  uint64_t name(void)
typedef PLATFORM_CYCLECOUNT(platform_CycleCount);

////////////////////////////////////////////////////////////////////////////////
typedef struct platform_job {
    JobFunction function;
    platform_job* parent;
    std::atomic_int unfinishedJobs; 
    char* padding;
} Job;

typedef struct platform_work_queue {
    Job* jobqueue;
} WorkQueue;


typedef struct PLATFORM_API {
    
    //JobSystem
    platform_add_entry *AddEntry;
    platform_complete_all_work *CompleteAllWork;

    platform_allocate_Job *Allocate_Job;
    platform_create_Job *CreateJob;
    platform_create_Job_as_Child *CreateJobAsChild;
    platform_get_Job *GetJob;
    platform_run_Job *RunJob;
    platform_wait_Job *WaitJob;
    platform_execute_Job *ExecuteJob;
    platform_finish_Job *FinishJob;

    platform_work_queue_pop *QueuePop;
    platform_work_queue_push *QueuePush;
    platform_work_queue_steal* QueueSteal;


    //Filesystem
    platform_get_all_files_of_type_begin *GetAllFilesOfTypeBegin;
    platform_get_all_files_of_type_end *GetAllFilesOfTypeEnd;
    platform_open_next_file *OpenNextFile;
    platform_read_data_from_file *ReadDataFromFile;
    platform_file_error *FileError;

    //MemorySystem
    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;
} platform_API;

inline u32 GetThreadID(void) {
    u32 ThreadID;
#if defined(__APPLE__) && defined(__x86_64__)
    asm("mov %%gs:0x00,%0" : "=r"(ThreadID));
#elif defined(__i386__)
    asm("mov %%gs:0x08,%0" : "=r"(ThreadID));
#elif defined(__x86_64__)
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
#else
#error Unsupported architecture
#endif

    return (ThreadID);
}

#endif	/* PLATFORM_H */

