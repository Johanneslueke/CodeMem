///* 
// * File:   JobSystem.h
// * Author: johannes
// *
// * Created on 20 December 2015, 15:44
// */
//
//#ifndef JOBSYSTEM_H
//#define	JOBSYSTEM_H
//#include <atomic>
//#include <x86intrin.h>
//#include <queue>
//#include <thread>
//#include <typeinfo>
//#include <typeindex>
//
//#include "Application.h"
//
//using namespace std;
//#define MAX_JOB_COUNT 12*12
//
//struct platform_work_queue;
//typedef platform_work_queue* WorkQueue;
//
//namespace jobsystem {
//
//    static constexpr unsigned int NUMBER_OF_JOBS = 4096u;
//    static constexpr unsigned int MASK = NUMBER_OF_JOBS - 1u;
//
//
//
//    struct Job;
//    typedef struct platform_work_queue {
//    int m_bottom = 0, m_top = 0;
//    memory_arena* m_workmemory;
//    Job* m_jobs[NUMBER_OF_JOBS];
//
//   
//
//    void Push(const Job* job) {
//        long b = m_bottom;
//        m_jobs[b & MASK] = (Job*) job;
//
//        // ensure the job is written before b+1 is published to other threads.
//        // on x86/64, a compiler barrier is enough.
//        asm volatile ("mfence" : : : "memory");
//
//        m_bottom = b + 1;
//    }
//
//    Job* Pop(void) {
//        long b = m_bottom - 1;
//        m_bottom = b;
//
//        long t = m_top;
//        if (t <= b) {
//            // non-empty queue
//            Job* job = m_jobs[b & MASK];
//            if (t != b) {
//                // there's still more than one item left in the queue
//                return job;
//            }
//
//            // this is the last item in the queue
//            if (__sync_val_compare_and_swap(&m_top, t + 1, t) != t) {
//                // failed race against steal operation
//                job = nullptr;
//            }
//
//            m_bottom = t + 1;
//            return job;
//        } else {
//            // deque was already empty
//            m_bottom = t;
//            return nullptr;
//        }
//    }
//
//    Job* Steal(void) {
//        long t = m_top;
//        long b = m_bottom;
//        if (t < b) {
//            // non-empty queue
//            Job* job = m_jobs[t & MASK];
//
//            if (__sync_val_compare_and_swap(&m_top, t + 1, t) != t) {
//                // a concurrent steal or pop operation removed an element from the deque in the meantime.
//                return nullptr;
//            }
//
//            return job;
//        } else {
//            // empty queue
//            return nullptr;
//        }
//    }
//} WorkQueue;
//
//    
//
//
//    ////////////////////////////////////////////////////////////////////////////////
//
//    
//
////    class JobSystem {
////    public:
////        static WorkQueue mWorkingQueue;
////
////        static Job g_jobAllocator[MAX_JOB_COUNT];
////        static uint32_t g_allocatedJobs;
////
////
////    public:
////
////        static bool IsEmptyJob(const Job* job) {
////            if (job->function == nullptr)
////                return true;
////            return false;
////        }
////
////        static WorkQueue GetWorkerThreadQueue() {
////            return mWorkingQueue;
////        }
////
////        static Job* AllocateJob(void) {
////            const uint32_t index = g_allocatedJobs++;
////            return &g_jobAllocator[index & (MAX_JOB_COUNT - 1u)];
////        }
////
////        static Job* CreateJob(JobFunction function,char* jobdata) {
////            Job* job = AllocateJob();
////            job->function = function;
////            job->parent = nullptr;
////            job->padding = jobdata;
////            job->unfinishedJobs = 1;
////
////            return job;
////        }
////
////        static Job* CreateJobAsChild(Job* parent, JobFunction function,char* jobdata) {
////            parent->unfinishedJobs++;
////
////            Job* job = AllocateJob();
////            job->function = function;
////            job->parent = parent;
////            job->padding = jobdata;
////            job->unfinishedJobs = 1;
////
////            return job;
////        }
////
////        static bool HasJobCompleted(const Job* job) {
////            if (job->unfinishedJobs != 0)
////                return false;
////            return true;
////        }
////
////        static void Run(const Job* job) {
////            WorkQueue queue = GetWorkerThreadQueue();
////            queue->Push(job);
////        }
////
////        static void Execute(const Job* job) {
////            job->function((Job*) job, job->padding);
////        }
////
////        static Job* GetJob() {
////            WorkQueue queue = GetWorkerThreadQueue();
////
////            auto GenerateRandomNumber = [](int a, int b) {
////
////                std::srand(std::time(nullptr));
////
////                return std::rand() % b;
////
////            };
////
////            Job* job = queue->Pop();
////            if (IsEmptyJob(job)) {
////                // this is not a valid job because our own queue is empty, so try stealing from some other queue
////                unsigned int randomIndex = GenerateRandomNumber(0, 1 + 1);
////                WorkQueue stealQueue = mWorkingQueue;
////                if (stealQueue == queue) {
////                    // don't try to steal from ourselves
////                    std::this_thread::yield();
////                    return nullptr;
////                }
////
////                Job* stolenJob = stealQueue->Steal();
////                if (IsEmptyJob(stolenJob)) {
////                    // we couldn't steal a job from the other queue either, so we just yield our time slice for now
////                    std::this_thread::yield();
////                    return nullptr;
////                }
////
////                return stolenJob;
////            }
////
////            return job;
////        }
////
////        static void Wait(const Job* job) {
////            // wait until the job has completed. in the meantime, work on any other job.
////            while (!HasJobCompleted(job)) {
////                Job* nextJob = GetJob();
////                if (nextJob) {
////                    Execute(nextJob);
////                }
////            }
////        }
////
////        static void Finish(Job* job) {
////            const int32_t unfinishedJobs = job->unfinishedJobs--;
////
////            if ((unfinishedJobs == 0) && (job->parent)) {
////                Finish(job->parent);
////            }
////        }
////    };
//
////    WorkQueue* JobSystem::mWorkingQueue = new WorkQueue;
//////    Job JobSystem::g_jobAllocator[];
//////    uint32_t JobSystem::g_allocatedJobs = 0u;
//
//    ///////////////////////////////////////////////////////////////////////////
//    
//    class CountSplitter;
//    class DataSizeSplitter;
//
//    
//    class CountSplitter {
//    public:
//
//        explicit CountSplitter(unsigned int count)
//        : m_count(count) {
//        }
//
//        template <typename T = int>
//        bool Split(unsigned int count) const {
//            return (count >= m_count);
//        }
//
//    private:
//        unsigned int m_count;
//    };
//
//    class DataSizeSplitter {
//    public:
//
//        explicit DataSizeSplitter(unsigned int size)
//        : m_size(size) {
//        }
//
//        template <typename T>
//        inline bool Split(unsigned int count) const {
//            return (count * sizeof (T) > m_size);
//        }
//
//    private:
//        unsigned int m_size;
//    };
//
//    
//    template <typename JobData>
//    void parallel_for_job(Job* job, const void* jobData) {
//        const JobData* data = static_cast<const JobData*> (jobData);
//        const typename JobData::SplitterType& splitter = data->splitter;
//        typedef typename JobData::DataType datatype;
//        datatype t;
//        
//        auto temp = splitter.Split(data->count);
//
//        if (temp) { 
//            // split in two
//            const unsigned int leftCount = data->count / 2u;
//            const JobData leftData(data->data, leftCount, data->function, splitter);
//            Job* left = JobSystem::CreateJobAsChild(job, &parallel_for_job<JobData>, (char*)&leftData);
//            JobSystem::Run(left);
//
//            const unsigned int rightCount = data->count - leftCount;
//            const JobData rightData(data->data + leftCount, rightCount, data->function, splitter);
//            Job* right = JobSystem::CreateJobAsChild(job, &parallel_for_job<JobData>, (char*)&rightData);
//            JobSystem::Run(right);
//        } else {
//            // execute the function on the range of data
//            (data->function)(data->data, data->count);
//        }
//    }
//
//    template <typename T, typename S>
//    Job* parallel_for(T* data, unsigned int count, void (*function)(T*, unsigned int), const S& splitter) {
//        typedef parallel_for_job_data<T, S> JobData;
//        JobData jobData(data, count, function, splitter);
//
//        Job* job = JobSystem::CreateJob(&parallel_for_job<JobData>, ((char*)&jobData));
//
//        return job;
//    }
//
//
//
//}
//
//#endif	/* JOBSYSTEM_H */
//
