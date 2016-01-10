#include "../include/Platform.h"


#include <cstdlib>
#include <sys/mman.h>
#include <assert.h>
#include <iostream>

bool HasJobCompleted(Job* job)
{
    return true;
}



namespace {

    extern "C" PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory) {

        std::cerr << "Allocate Memory. Source: " << __FILE__ << " " << __LINE__ << "\n";

        return mmap(adress, Size,
                PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE,
                -1, 0);
    };

    extern "C" PLATFORM_ALLOCATE_JOB(platform_allocate_Job) {

    };

    extern "C" PLATFORM_CREATE_JOB(platform_create_Job) {
//        Job* job = platform_allocate_Job();
//        job->function = function;
//        job->parent = nullptr;
//        job->padding = jobdata;
//        job->unfinishedJobs = 1;
//
//        return job;
    };

    extern "C" PLATFORM_CREATE_JOB_AS_CHILD(platform_create_Job_as_Child) {
//        Job* job = platform_allocate_Job();
//        job->function = Callback;
//        job->parent = parent;
//        job->padding = Data;
//        job->unfinishedJobs = 1;

//        return job;
    };

    extern "C" PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory) {
        int res = munmap(Memory, Size);

        std::cerr << "Deallocate Memory. Result:" << res << "| Source: " << __FILE__ << " " << __LINE__ << "\n";

    };

    extern "C" PLATFORM_FUNC_JOB(platform_execute_Job) {

    };

    extern "C" PLATFORM_FUNC_JOB(platform_finish_Job) {
        
        const int32_t unfinishedJobs = job->unfinishedJobs--;

        if ((unfinishedJobs == 0) && (job->parent)) {
            platform_finish_Job(job->parent);
        }
        
    };

    extern "C" PLATFORM_GET_JOB(platform_get_Job) {
    };

    extern "C" PLATFORM_FUNC_JOB(platform_wait_Job) {

        while (!HasJobCompleted(job)) {
            Job* nextJob = platform_get_Job(nullptr);
            if (nextJob) {
                platform_execute_Job(nextJob);
            }
        }
    };

    extern "C" PLATFORM_FUNC_JOB(platform_run_Job) {
    };

    extern "C" PLATFORM_WORK_QUEUE_POP(platform_work_queue_pop) {

    };

    extern "C" PLATFORM_WORK_QUEUE_PUSH(platform_work_queue_push) {
    };

    extern "C" PLATFORM_WORK_QUEUE_POP(platform_work_queue_steal) {

    };
}
