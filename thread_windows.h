#ifndef __THREAD_WINDOWS_H
#ifdef WIN32

#include <windows.h>
#include <vector>

using namespace std;

#define WINDOWS_THREADS_ALLOC_STEP_SIZE 10

namespace threading
{
    DWORD WINAPI _WindowsThreadHandler(LPVOID lpParam);

    typedef bool (*thread_boot_f)(void *);

    class ThreadWrapper
    {
        static int sn;

        thread_boot_f thread_boot;
        void *thread_data;

        DWORD WINAPI ThreadFunc(LPVOID *param);

    public:
        ThreadWrapper(thread_boot_f thread_boot, void *data=NULL);
        bool Start();
        bool Join();
    };

    typedef struct {
        int sn;
        bool active;
        thread_boot_f thread_inner_boot;
        void *data;

        ThreadWrapper *thread;
        DWORD id;
        HANDLE handle;
    } thread_info_t;

    class ThreadManager
    {
        CRITICAL_SECTION cs;
        vector<thread_info_t *> threads;
        int nextIndex;

    public:
        ThreadManager();
        ~ThreadManager();
        bool AllocateSn(int &sn, thread_info_t **);
    };
}

#endif
#endif