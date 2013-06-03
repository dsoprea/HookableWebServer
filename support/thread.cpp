#include "thread.h"
#include <pthread.h>

namespace threading
{
    int ThreadWrapper::sn = 0;

    ThreadManager threadManager;

    ThreadInfo::ThreadInfo(int sn_, char state_,
                           thread_boot_t threadInnerBoot_, void *data_,
                           void *manager_, void *thread_, unsigned int id_,
                           pthread_t handle_)
    {
        sn = sn_;
        state = state_;
        threadInnerBoot = threadInnerBoot_;
        data = data_;
        manager = manager_;
        thread = thread_;
        id = id_;
        handle = handle_;
    }

    ThreadInfo::ThreadInfo(int sn_)
    {
        sn = sn_;
        state = 0;
        threadInnerBoot = NULL;
        data = NULL;
        manager = NULL;
        thread = NULL;
        id = 0;
        handle = 0;
    }

    void *_WindowsThreadHandler(void *param)
    {
        // A new thread starts here, and then calls the function originally
        // passed by the user.

        ThreadInfo *threadInfo = (ThreadInfo *)param;
        threadInfo->SetId(pthread_self());

        threadInfo->SetState(TS_RUNNING);
        threadInfo->GetThreadInnerBoot()(threadInfo->GetData());
        threadInfo->SetState(TS_ENDED);

        ThreadManager *manager = (ThreadManager *)threadInfo->GetManager();
        manager->NotifyThreadStop(threadInfo);

        pthread_exit(0);
    }

    ThreadManager::ThreadManager()
    {
        pthread_mutex_init(&slotLocker, NULL);

        nextIndex = 0;
    }

    ThreadManager::~ThreadManager()
    {
        Cleanup();
    }

    bool ThreadManager::Cleanup()
    {
// We disabled doing this because it made thread lifecycles more ambiguous.
/*        size_t i = 0;
        while(i < threads.size())
        {
            ThreadInfo *thread = threads[i];

            if(thread != NULL)
                delete thread;

            i++;
        }
*/
        pthread_mutex_destroy(&slotLocker);

        return true;
    }

    int ThreadManager::GetAvailableSn()
    {
        if(availableSlots.empty())
        {
            // No thread resource slots are available (allocated and unused).
            // Allocate more.

            int nextSlotIndex = threads.size();
            int newSize = std::min(nextSlotIndex + THREADS_ALLOC_STEP_SIZE,
                                   THREADS_MAX_THREADS);

            // Have we hit the max?
            if(newSize >= THREADS_MAX_THREADS)
                return -1;

            // A vector that describes all threads.
            threads.resize(nextSlotIndex + THREADS_ALLOC_STEP_SIZE, NULL);

            // Load the sorted list of available serial-numbers/slot-numbers.

            int i = 0;
            while(i < THREADS_ALLOC_STEP_SIZE)
            {
                if(ReleaseSn(nextSlotIndex + i) == false)
                    return -1;

                i++;
            }
        }

        int nextSlotIndex = availableSlots[0];
        availableSlots.erase(availableSlots.begin());

        return nextSlotIndex;
    }

    bool ThreadManager::NotifyThreadStop(ThreadInfo *threadInfo)
    {
        // A thread has stopped. Add the SN to a sorted list of freed slots, 
        // and cleanup the memory.

        pthread_mutex_lock(&slotLocker);

        int sn = threadInfo->GetSn();

        //delete threadInfo;
        ReleaseSn(sn);

        pthread_mutex_unlock(&slotLocker);

        //if(result == false)
            //return false;

        return true;
    }

    bool ThreadManager::AllocateSn(int &sn, ThreadInfo **threadInfoPtr)
    {
        // Allocate a slot for the information for a new thread.

        pthread_mutex_lock(&slotLocker);

        if((sn = GetAvailableSn()) == -1)
        {
            pthread_mutex_unlock(&slotLocker);
            return false;
        }

        ThreadInfo *threadInfo = new ThreadInfo(sn);

        threadInfo->SetSn(sn);

        threads[sn] = threadInfo;
        *threadInfoPtr = threadInfo;

        pthread_mutex_unlock(&slotLocker);

        return true;
    }

    bool ThreadManager::ReleaseSn(int sn)
    {
        unsigned insert_at = 0;
        if(availableSlots.empty() == false)
            while(insert_at < availableSlots.size() &&
                  sn > availableSlots[insert_at])
                insert_at++;

        availableSlots.insert(availableSlots.begin() + insert_at, sn);

        return true;
    }

    ThreadWrapper::ThreadWrapper(std::string description_,
                                             thread_boot_t thread_boot_,
                                             void *data)
    {
        //stringstream ss;
        //ss << "Creating thread [" << description_ << "].";
        //log_info(ss.str(), this);

        description = description_;
        threadBoot = thread_boot_;
        threadData = data;
        threadInfo = NULL;
    }

    ThreadWrapper::~ThreadWrapper()
    {
        if(threadInfo != NULL)
            delete threadInfo;
    }

    bool ThreadWrapper::Start()
    {
        // We have been asked to start the thread.

        //stringstream ss;
        //ss << "Starting thread [" << description << "].";
        //log_info(ss.str(), this);

        // Make sure that this is the first/only start.
        if(threadInfo != NULL)
            return false;

        int sn;
        if(threadManager.AllocateSn(sn, &threadInfo) == false)
            return false;

        threadInfo->SetThread(this);
        threadInfo->SetManager(&threadManager);
        threadInfo->SetThreadInnerBoot(threadBoot);
        threadInfo->SetData(threadData);

        pthread_t handle;
        if((pthread_create(&handle, NULL, _WindowsThreadHandler,
                           threadInfo)) != 0)
            return false;

        threadInfo->SetHandle(handle);

        return true;
    }

    bool ThreadWrapper::Join()
    {
        // Block on the thread until it quits, and then clean it up.

        pthread_join(threadInfo->GetHandle(), NULL);

        return true;
    }
/*
    void threading_system_shutdown_trigger()
    {
        threadManager.Cleanup();
    }
*/
}
