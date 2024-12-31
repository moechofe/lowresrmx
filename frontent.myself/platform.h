#pragma once

#if defined(linux) || defined(__linux) || defined(__linux__)
	#undef IS_LINUX
	#define IS_LINUX 1
#endif

#if defined(ANDROID) || defined(__ANDROID__)
	#undef IS_ANDROID
	#undef IS_LINUX
	#define IS_ANDROID 1
#endif

#if defined(__APPLE__)
	#include "AvailabilityMacros.h"
	#include "TargetConditionals.h"
	#if TARGET_OS_IPHONE
		#undef IS_IOS
		#define IS_IOS 1
		#undef IS_MACOS
	#else
		#undef IS_MACOS
		#define IS_MACOS 1
	#endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
	#undef IS_WINDOWS
	#define IS_WINDOWS 1
#endif
