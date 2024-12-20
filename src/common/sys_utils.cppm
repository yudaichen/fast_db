module ;

#include <cstdint>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

export module sys_utils;

export namespace fast::util{

    class SysUtil {
    public:
        static void MSleep(uint32_t msec) {
#ifdef WIN32
            ::Sleep(msec);
#else
            usleep(msec * 1000);
#endif
        }
    };

}