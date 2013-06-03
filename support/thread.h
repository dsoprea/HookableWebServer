#ifndef __THREAD_H
#define __THREAD_H

#include <pthread.h>

#include <vector>
#include <algorithm>
#include <string>

#include "thread_constants.h"

#define THREADS_ALLOC_STEP_SIZE 10
#define THREADS_MAX_THREADS 100

namespace threading
{
    // This passes an instance of the thread so that the reference-count of the 
    // instance will always be at least one as long as the thread function is 
    // executing.
    typedef bool (*thread_boot_t)(void *param);

    class ThreadInfo
    {
        int sn;
        char state;
        thread_boot_t threadInnerBoot;
        void *data;

        void *manager;
        void *thread;
        unsigned int id;
        pthread_t handle;

        public:
            ThreadInfo(int sn, char state, thread_boot_t threadInnerBoot,
                       void *data, void *manager, void *thread,
                       unsigned int id, pthread_t handle);
            ThreadInfo(int sn);

            int GetSn() const { return sn; }
            char GetState() const { return state; }
            thread_boot_t GetThreadInnerBoot() const
                { return threadInnerBoot; }
            void *GetData() const { return data; }
            void *GetManager() const { return manager; }
            void *GetThread() const { return thread; }
            unsigned int GetId() const { return id; }
            pthread_t GetHandle() const { return handle; }

            void SetSn(int value) { sn = value; }
            void SetState(char value) { state = value; }
            void SetThreadInnerBoot(thread_boot_t value)
                { threadInnerBoot = value; }
            void SetData(void *value) { data = value; }
            void SetManager(void *value) { manager = value; }
            void SetThread(void *value) { thread = value; }
            void SetId(pthread_t value) { id = value; }
            void SetHandle(pthread_t value) { handle = value; }
    };

    void *_WindowsThreadHandler(void *param);

    class ThreadWrapper
    {
        static int sn;

        thread_boot_t threadBoot;
        void *threadData;
        ThreadInfo *threadInfo;
        std::string description;

        void ThreadFunc(void *param);

    public:
        ThreadWrapper(std::string description, thread_boot_t threadBoot_,
                      void *data=NULL);
        ~ThreadWrapper();
        ThreadInfo *GetThreadInfo() const { return threadInfo; };
        bool Start();
        bool Join();
    };

    class ThreadManager
    {
        pthread_mutex_t slotLocker;
        std::vector<ThreadInfo *> threads;
        std::vector<int> availableSlots;
        int nextIndex;
        int GetAvailableSn();

    public:
        ThreadManager();
        ~ThreadManager();
        bool Cleanup();
        bool AllocateSn(int &sn, ThreadInfo **threadInfoPtr);
// TODO: Contemplate renaming this back to AddAvailableSn().
        bool ReleaseSn(int sn);
        bool NotifyThreadStop(ThreadInfo *threadInfo);
    };
}

#endif
