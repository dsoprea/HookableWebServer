#include "thread_windows.h"

namespace threading
{
    int ThreadWrapper::sn = 0;

    ThreadManager threadManager;

    DWORD WINAPI _WindowsThreadHandler(LPVOID param)
    {
        // A new thread starts here, and then calls the function originally passed by the user.

        thread_info_t *threadInfo = (thread_info_t *)param;
        threadInfo->id = GetCurrentThreadId();

        if(threadInfo->thread_inner_boot(threadInfo) == false)
            return -1;

        return 0;
    }

    ThreadManager::ThreadManager()
    {
        InitializeCriticalSection(&cs);

        nextIndex = 0;
    }

    ThreadManager::~ThreadManager()
    {
        vector<thread_info_t *>::iterator it = threads.begin();
        while(it != threads.end())
        {
            if(*it == NULL)
                break;

            delete *it;
        }

        DeleteCriticalSection(&cs);
    }

    bool ThreadManager::AllocateSn(int &sn, thread_info_t **thread_info_ptr)
    {
        // Allocate a slot for the information for a new thread.

        EnterCriticalSection(&cs);

        int slots = threads.size();
        if(nextIndex >= slots)
        {
            slots += WINDOWS_THREADS_ALLOC_STEP_SIZE;
            threads.resize(slots, NULL);
        }

        sn = nextIndex++;
        thread_info_t *thread_info = (thread_info_t *)malloc(sizeof(thread_info_t));
        memset(thread_info, 0, sizeof(thread_info_t));

        thread_info->sn = sn;
        thread_info->active = true;

        threads[sn] = thread_info;
        *thread_info_ptr = thread_info;

        LeaveCriticalSection(&cs);

        return true;
    }

    ThreadWrapper::ThreadWrapper(thread_boot_f thread_boot_, void *data)
    {
        thread_boot = thread_boot_;
        thread_data = data;
    }

    bool ThreadWrapper::Start()
    {
        // We have been asked to start the thread.

        int sn;
        thread_info_t *thread_info;

        if(threadManager.AllocateSn(sn, &thread_info) == false)
            return false;

        thread_info->thread = this;
        thread_info->thread_inner_boot = thread_boot;
        thread_info->data = thread_data;
        
        HANDLE handle;
        if((handle = CreateThread(NULL, 0, _WindowsThreadHandler, thread_info, 0, NULL)) == NULL)
            return false;

        thread_info->handle = handle;

        return true;
    }

    bool ThreadWrapper::Join()
    {
        // We have been asked to block on the thread until it quits, and then clean it up.

        return -1;
    }
}