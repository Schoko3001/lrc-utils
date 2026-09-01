#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdatomic.h>


#if defined(_WIN32)
	#include <windows.h>
	typedef HANDLE lrc_hndl;
	#define lrc_hndl_invalid INVALID_HANDLE_VALUE
#elif defined(__APPLE__)
	#error APPLE_IS_NOT_SUPPORTED
#elif defined(__ANDROID__)
	#error ANDROID_IS_NOT_SUPPORTED
#elif defined(__linux__)
	#include <linux/futex.h>
	#include <sys/syscall.h>
	#include <sys/types.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <stdint.h>
	#include <fcntl.h>
	typedef int lrc_hndl;
	#define lrc_hndl_invalid -1
#else
	#error UNKOWN/MISSING_OS_SPECIFIER
#endif

/* internal header function */
static inline void _lrc_safeStrcpy(char* buffer, const char* source, size_t len) {
	if (len == 0) return;

	int i = 0;
	for (;i < len-1 && source[i] != '\0'; i++)
		buffer[i] = source[i];

	if (source[i] != '\0')
		printf("WARNING: lrc_sharedMemory.h, name too long\n");
	
	buffer[i] = '\0';
}





static inline lrc_hndl lrc_sharedMemory_create(void** buffer_p, const char* name, size_t size) {
#if defined(_WIN32)
	char full_name[MAX_PATH + 7 + 1] = "Global\\";
	_lrc_safeStrcpy(full_name + 7, name, sizeof(full_name) - 7);

	// create kernel object
	HANDLE hMap = CreateFileMappingA(
		INVALID_HANDLE_VALUE,   // not a real file
		NULL,                   // idk
		PAGE_READWRITE,
		0,
		size,
		full_name);
	if (!hMap) {
		printf("lrc_sharedMemory_create, CreateFileMappingA failed: %lu\n", GetLastError());
		return lrc_hndl_invalid;
	}
	
	// create actual memory
	*buffer_p = MapViewOfFile(
		hMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		size);
	if (!*buffer_p) {
		printf("lrc_sharedMemory_create, MapViewOfFile failed: %lu\n", GetLastError());
		CloseHandle(hMap);
		return lrc_hndl_invalid;
	}

	return hMap;
#elif defined(__linux__)
	char full_name[NAME_MAX + 1 + 1] = "/";
	_lrc_safeStrcpy(full_name + 1, name, sizeof(full_name) - 1);

	// create kernel object
	int fd = shm_open(full_name, O_CREAT | O_RDWR, 0666);
	if (fd == -1) {
		perror("lrc_sharedMemory_create, shm_open failed");
		return lrc_hndl_invalid;
	}
	if (ftruncate(fd, size) == -1) {
		perror("lrc_sharedMemory_create, ftruncate failed");
		close(fd);
		return -1;
	}

	// create actual memory
	*buffer_p = mmap(
		0,
		size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd,
		0);
	if (*buffer_p == MAP_FAILED) {
		perror("lrc_sharedMemory_create, mmap failed");
		close(fd);
		return lrc_hndl_invalid;
	}

	return fd;
#endif
}



static inline lrc_hndl lrc_sharedMemory_open(void** buffer_p, const char* name, size_t size) {
#if defined(_WIN32)
char full_name[MAX_PATH + 7 + 1] = "Global\\";
	_lrc_safeStrcpy(full_name + 7, name, sizeof(full_name) - 7);
	
	// open kernel object
	HANDLE hMap = OpenFileMappingA(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		full_name);
	if (!hMap) {
		return lrc_hndl_invalid;
	}

	// gain access to actual memory
	*buffer_p = MapViewOfFile(
		hMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		size);
	if (!*buffer_p) {
		printf("lrc_sharedMemory_open, MapViewOfFile failed: %lu\n", GetLastError());
		CloseHandle(hMap);
		return lrc_hndl_invalid;
	}

	return hMap;
#elif defined(__linux__)
	char full_name[NAME_MAX + 1 + 1] = "/";
	_lrc_safeStrcpy(full_name + 1, name, sizeof(full_name) - 1);

	// open kernel object
	int fd = shm_open(full_name, O_RDWR, 0666);
	if (fd == -1) {
		return lrc_hndl_invalid;
	}

	// gain access to actual memory
	*buffer_p = mmap(
		0,
		size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd,
		0);
	if (*buffer_p == MAP_FAILED) {
		perror("lrc_sharedMemory_open, mmap failed");
		close(fd);
		return lrc_hndl_invalid;
	}

	return fd;
#endif
}


static inline void lrc_sharedMemory_close(const char* name, lrc_hndl hndl) {
#if defined(_WIN32)
	CloseHandle(hndl);
#elif defined(__linux__)
	char full_name[NAME_MAX + 1 + 1] = "/";
	_lrc_safeStrcpy(full_name + 1, name, sizeof(full_name) - 1);

	shm_unlink(full_name);
	close(hndl);
#endif
}


static inline void lrc_sharedMemory_unmap(void* ptr, size_t size) {
#if defined(_WIN32)
	UnmapViewOfFile(ptr);
#elif defined(__linux__)
	munmap(ptr, size);
#endif
}