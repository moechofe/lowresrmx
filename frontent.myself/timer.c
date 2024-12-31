#include "timer.h"
#include "platform.h"

#if IS_LINUX
	#define _POSIX_C_SOURCE 199309L
#endif
#include <time.h>

unsigned long long int get_frequency()
{
	// This is a dummy implementation. The actual implementation should be platform-specific. Apple use mach_timebase_info()
	return 1000000000LL;
}

unsigned long long int get_counter()
{
	unsigned long long int ticks;
	struct timespec now;
	// This is a dummy implementation. The actual implementation should be platform-specific. Apple use mach_timebase_info()
	clock_gettime(CLOCK_MONOTONIC_RAW,&now);
	ticks=now.tv_sec*1000000000LL+now.tv_nsec;
	return ticks;
}
