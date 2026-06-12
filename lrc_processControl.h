#pragma once
#include <stdio.h>
#include <stdarg.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <tlhelp32.h>
	#define lrc_pid DWORD
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
#else
	#error UNKOWN/MISSING_OS_SPECIFIER
#endif


/* internal header function */
static inline void _lrc_safeStrcpy(char* buffer, const char* source, size_t len) {
	if (len == 0) return;

	int i = 0;
	for (;i < len-1 && source[i] != '\0'; i++)
		buffer[i] = source[i];

	if (source[i] != 0)
		printf("WARNING: lrc_sharedMemory.h, name too long\n");
	
	buffer[i] = '\0';
}



static inline int lrc_processClone(bool inheritEnv, size_t count, ...) {
	va_list args;
	va_start(args, count);

	// first arg varCount
	size_t varCount = va_arg(args, size_t);



#if defined(_WIN32)
	void* env; 

	if (count == 0) {
		env = NULL;
	} else {
		LPCH win_env = GetEnvironmentStringsA();

		// size of current environment
		size_t size_c = 0;
		while (win_env[size_c]) {
			size_c += strlen(&win_env[size_c]) + 1;
		}

		// size of new stuff
		int size_n;
		va_list args;
		va_start(args, count);
		for (int i = 0; i < count; i++) {
			size_n += strlen(va_arg(args, char*)) + 1; // var_name
			size_n += strlen(va_arg(args, char*)) + 1; // var_value
		}
		va_end(args);

		// new environment
		char* buffer = malloc(size_c + size_n + 1);
		for (int i = 0; i < size_c; i++) {
			buffer[i] = win_env[i];
		}
		va_start(args, count);
		for (int i = 0; i < count; i++) {
			for (int j = 0)
		}




	}




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
		env,
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
#elif defined(__linux__)
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
#endif
}


static inline lrc_pid lrc_getPID() {
#if defined(_WIN32)
	return GetCurrentProcessId();
#elif defined(__linux__)
	return getpid();
#endif
}



static inline lrc_pid lrc_getParentPID() {
#if defined(_WIN32)
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
#elif defined(__linux__)
	return getppid();
#endif
}


 
static inline void lrc_envVariableWrite(const char* var_name, const char* str) {
#if defined(_WIN32)
	SetEnvironmentVariableA(var_name, str);
#elif defined(__linux__)
	setenv(var_name, str, 1);
#endif
}


static inline size_t lrc_envVariableRead(const char* var_name, char* buffer, size_t buffer_size) {
#if defined(_WIN32)
	DWORD len = GetEnvironmentVariableA(var_name, buffer, buffer_size);
	return len;
#elif defined(__linux__)
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
#endif
}