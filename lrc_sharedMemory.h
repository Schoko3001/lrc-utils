#pragma once
#include <stdint.h>
#include <stdio.h>


#if defined(_WIN32)
#include <windows.h>

typedef HANDLE lrc_hndl;
#define lrc_hndl_invalid INVALID_HANDLE_VALUE

static inline lrc_hndl lrc_sharedMemory_create(void** buffer_p, char* name, size_t size) {
	char full_name[255] = "Global\\";
	int j = 7;
	int i = 0;
	while (i < strlen(name) + 1) {
		full_name[j++] = name[i++];
	}

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
}

static inline lrc_hndl lrc_sharedMemory_open(void** buffer_p, char* name, size_t size) {
	char full_name[255] = "Global\\";
	int j = 7;
	int i = 0;
	while (i < strlen(name) + 1) {
		full_name[j++] = name[i++];
	}
	
	// open kernel object
	HANDLE hMap = OpenFileMappingA(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		full_name);
	if (!hMap) {
		printf("lrc_sharedMemory_create, OpenFileMappingA failed: %lu\n", GetLastError());
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
		printf("lrc_sharedMemory_create, MapViewOfFile failed: %lu\n", GetLastError());
		CloseHandle(hMap);
		return lrc_hndl_invalid;
	}

	return hMap;
}

static inline void lrc_sharedMemory_close(void* ptr, char* name, size_t size, lrc_hndl hndl) {
	UnmapViewOfFile(ptr);
	CloseHandle(hndl);
}

#elif defined(__APPLE__)
	#error APPLE_IS_NOT_SUPPORTED

#elif defined(__ANDROID__)
	#error ANDROID_IS_NOT_SUPPORTED


#elif defined(__linux__)
#include <fcntl.h>
#include <sys/mman.h>

typedef int lrc_hndl;
#define lrc_hndl_invalid -1


static inline lrc_hndl lrc_sharedMemory_create(void** buffer_p, char* name, size_t size) {
	char full_name[255] = "/";
	int j = 1;
	int i = 0;
	while (i < strlen(name) + 1) {
		full_name[j++] = name[i++];
	}


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
}

static inline lrc_hndl lrc_sharedMemory_open(void** buffer_p, char* name, size_t size) {
	char full_name[255] = "/";
	int j = 1;
	int i = 0;
	while (i < strlen(name) + 1) {
		full_name[j++] = name[i++];
	}

	// open kernel object
	int fd = shm_open(full_name, O_RDWR, 0666);
	if (fd == -1) {
		perror("lrc_sharedMemory_open, shm_open failed");
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
}

static inline void lrc_sharedMemory_close(void* ptr, char* name, size_t size, lrc_hndl hndl) {
	char full_name[255] = "/";
	int j = 1;
	int i = 0;
	while (i < strlen(name) + 1) {
		full_name[j++] = name[i++];
	}
	shm_unlink(full_name)
	munmap(ptr, size);
	close(hndl);
}

#else
	#error UNKOWN/MISSING_OS_SPECIFIER
#endif