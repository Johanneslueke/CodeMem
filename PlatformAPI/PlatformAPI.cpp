#include "../include/Platform.h"


#include <cstdlib>
#include <sys/mman.h>
#include <assert.h>
#include <iostream>
#include <x86intrin.h>
#include <chrono>
#include <ctime>


////////////////////////////////////////////////////////////////////////////////
////// http://blog.quasardb.net/a-portable-high-resolution-timestamp-in-c/ /////
////////////////////////////////////////////////////////////////////////////////
#if __win32__

struct Clock {

    std::uint64_t filetimeoffset(void) {
        FILETIME ft;
        std::uint64_t tmpres = 0;
        // 100-nanosecond intervals since January 1, 1601 (UTC) 
        // which means 0.1 us 
        GetSystemTimeAsFileTime(&ft);
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
        // January 1st, 1970 - January 1st, 1601 UTC ~ 369 years 
        // or 116444736000000000 us 
        staticconst std::uint64_t deltaepoch = 116444736000000000;
        tmpres -= deltaepoch;
        return tmpres;
    }
    // offset in microseconds 

    static std::uint64_t zerotime(void) {
        return filetime_offset();
    }

    static std::uint64_t timestamp(void) {
        LARGEINTEGER li;
        QueryPerformanceCounter(&li);
        // there is an imprecision with the initial value, 
        // but what matters is that timestamps are monotonic and consistent 
        return static_cast<std::uint64_t> (li.QuadPart);
    }

    static std::uint64_t updatefrequency(void) {
        LARGEINTEGER li;
        if (!QueryPerformanceFrequency(&li) || !li.QuadPart) {
            // log something 
            std::terminate();
        }
        return static_cast<std::uint64_t> (li.QuadPart);
    }

};
#elif __linux__

struct Clock {

    static std::uint64_t zerotime(void) {
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        // in 100-ns intervals 
        return static_cast<uint64_t> (ts.tv_sec)* 10000000u + static_cast<uint64_t> (ts.tv_nsec) / 100u;
    }

    static std::uint64_t timestamp(void) {
        timespec ts;
        // cannot fail, parameters are correct and we checked earlier we can access the clock 
        clock_gettime(CLOCK_MONOTONIC, &ts);
        // in ns 
        return static_cast<std::uint64_t> (ts.tv_sec)* 1000000000u + static_cast<std::uint64_t> (ts.tv_nsec);
    }

    static std::uint64_t updatefrequency(void) {
        timespec ts;
        if (clock_getres(CLOCK_MONOTONIC, &ts) < 0) {
            // log error 
            std::terminate();
        }

        assert(!ts.tv_sec);

        // this is the precision of the clock, we want the number of updates per second, which is 
        // 1 / ts.tvnsec * 1,000,000,000 
        static const std::uint64_t billion = 1000000000;

        return billion / static_cast<std::uint64_t> (ts.tv_nsec);

    }
};

#endif

template<typename T>
struct hiresclock {

    hiresclock(void) : _zerotime(T::zerotime()),_offset(T::timestamp()),_frequency(T::updatefrequency()) {
        assert(_offset > 0u); 
        assert(_frequency > 0u); 
        assert(_zerotime > 0u); 
    }

    std::uint64_t now(void)const {
        assert(_offset > 0u);
        assert(_frequency > 0u);
        const std::uint64_t delta = T::timestamp() -_offset;
        // because the frequency is in update per seconds, we have to multiply the delta by 10,000,000 
        const std::uint64_t deltainus = delta * 10000000u / _frequency;
        return deltainus + _zerotime;
    }

private:
    std::uint64_t _zerotime;
    std::uint64_t _offset;
    std::uint64_t _frequency;
};
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


bool HasJobCompleted(Job* job) {
    return true;
}

namespace {

    extern "C" PLATFORM_CYCLECOUNT(platform_CycleCount) {

        //hiresclock<Clock> a;
        
        return (new hiresclock<Clock>)->now();;
    }


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

    ///////////////////////////////////////////////////////////////
    ////////////////MEMORY-OPERATIONS//////////////////////////////

    extern "C" PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory) {

        std::cerr << "Allocate Memory. Source: " << __FILE__ << " " << __LINE__ << "\n";

        return mmap(adress, Size,
                PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE,
                -1, 0);
    };


    extern "C" PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory) {
        int res = munmap(Memory, Size);
        std::cerr << "Deallocate Memory. Result:" << res << "| Source: " << __FILE__ << " " << __LINE__ << "\n";

    };

    extern "C" PLATFORM_ALLOCATE_MEMORY(platform_create_arena)
    {
        MemoryArena* arena = (MemoryArena*) adress;

        arena->Used = 0;
        arena->Size = Size;
        arena->StartAdress = (MemoryAdress)(arena + sizeof(arena));

        return (void*) arena;
    };

}
