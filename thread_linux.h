#ifndef __THREAD_LINUX_H
#ifdef linux

namespace threading
{
    class _Thread
    {
    public:
        _Thread();
        int Start();
        int Join();
    };
}

#endif
#endif
