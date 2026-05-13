#pragma once


#if defined(_WIN32)
#include "windows.h"

static inline int lrc_processClone() {
	char path[MAX_PATH];

	GetModuleFileNameA(NULL, path, MAX_PATH);

	STARTUPINFOA si = {0};
	PROCESS_INFORMATION pi = {0};

	si.cb = sizeof(si);

	if (CreateProcessA(
		path,
		NULL,
		NULL, NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)
	) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return 0;
	} else {
		return 1;
	}
}



#elif defined(__APPLE__)
	#error APPLE_IS_NOT_SUPPORTED

#elif defined(__ANDROID__)
	#error ANDROID_IS_NOT_SUPPORTED

#elif defined(__linux__)
#include <spawn.h>

extern char **environ;

static inline int lrc_processClone() {
	pid_t pid;

	char *argv[] {
		"/proc/self/exe",
		NULL
	};

	if (posix_spawn(
		&pid,
		"/proc/self/exe",
		NULL,
		NULL,
		argv,
		environ)
	) {
		return 1;
	} else {
		return 0;
	}
}


#else
	#error UNKOWN/MISSING_OS_SPECIFIER
#endif

