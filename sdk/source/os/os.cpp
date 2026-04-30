#include "os/os.hpp"

#ifdef      ADAM_PLATFORM_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#elifdef    ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam::os
{
}
