#pragma once
#include <stdio.h>


#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>

#define lrc_pid DWORD

static inline void _lrc_safeStrcpy(char* buffer, const char* source, size_t len) {
	if (len == 0) return;

	int i = 0;
	for (;i < len-1 && source[i] != '\0'; i++)
		buffer[i] = source[i];

	if (source[i] != 0)
		printf("WARNING: lrc_sharedMemory.h, name too long\n");
	
	buffer[i] = '\0';
}


/* process creation */
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

/* process id */
static inline lrc_pid lrc_getParentPID() {
	DWORD currentPID = GetCurrentProcessId();
	DWORD parentPID = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (snapshot == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &pe)) {
		do {
			if (pe.th32ProcessID == currentPID) {
				parentPID = pe.th32ParentProcessID;
				break;
			}
		} while (Process32Next(snapshot, &pe));
	}

	CloseHandle(snapshot);
}
static inline lrc_pid lrc_getPID() {
	return GetCurrentProcessId();
}

/* environmental variables */
static inline void lrc_envVariableSet(
	const char* var_name, 
	const char* str
) {
	SetEnvironmentVariableA(
		var_name,
		str
	);
}
static inline size_t lrc_envVariableGet(
	const char* var_name, 
	char* buffer, 
	size_t buffer_size
) {
	DWORD len = GetEnvironmentVariableA(
		var_name,
		buffer,
		buffer_size
	);
	return len;
}


#elif defined(__APPLE__)
	#error APPLE_IS_NOT_SUPPORTED

#elif defined(__ANDROID__)
	#error ANDROID_IS_NOT_SUPPORTED

#elif defined(__linux__)
#include <spawn.h>
#include <unistd.h>
#include <sys/types.h>

#define lrc_pid pid_t

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

static inline lrc_pid lrc_getParentPID() {
	return getppid();
}

static inline lrc_pid lrc_getPID() {
	return getpid();
}

/* environmental variables */
static inline void lrc_envVariableSet(
	const char* var_name, 
	const char* str
) {
	setenv(var_name, str, 1);
}
static inline size_t lrc_envVariableGet(
	const char* var_name, 
	char* buffer, 
	size_t buffer_size
) {
	const char* source = getenv(var_name);
	if (source == NULL) return 0;
	if (buffer_size == 0) return strlen(source) + 1;

	size_t i = 0;
	for (;i < buffer_size-1 && source[i] != '\0'; i++)
		buffer[i] = source[i];

	buffer[i] = '\0';
	if (source[i] != '\0') {
		while (source[++i] != '\0');
		i++;
	}

	return i;
}

#else
	#error UNKOWN/MISSING_OS_SPECIFIER
#endif

