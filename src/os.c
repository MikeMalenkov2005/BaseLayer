#include <defines.h>

#if defined(OS_WIN)
#include "os/win_os.c"
#elif defined(OS_LINUX)
#include "os/linux_os.c"
#elif defined(OS_MAC)
#include "os/mac_os.c"
#else
#error OS is not implemented.
#endif
