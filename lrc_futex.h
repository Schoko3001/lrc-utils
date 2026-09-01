#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <limits.h>

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__APPLE__)
	#error APPLE_IS_NOT_SUPPORTED
#elif defined(__ANDROID__)
	#error ANDROID_IS_NOT_SUPPORTED
#elif defined(__linux__)
	#include <linux/futex.h>
	#include <sys/syscall.h>
	#include <unistd.h>
#else
	#error UNKOWN/MISSING_OS_SPECIFIER
#endif



static inline void lrc_sleepWhile(_Atomic int* ptr, int expected) {
	atomic_thread_fence(memory_order_seq_cst);
	do {
#if defined(_WIN32)
		WaitOnAddress(
			ptr,
			&expected,
			sizeof(int),
			INFINITE
		);
#elif defined(__linux__)
		syscall(
			SYS_futex,
			ptr,
			FUTEX_WAIT,
			expected,
			NULL,
			NULL,
			0
		);
#endif
	} while(atomic_load_explicit(ptr, memory_order_acquire) == expected);
	atomic_thread_fence(memory_order_seq_cst);
}


static inline void lrc_tryWakeup(_Atomic int* ptr) {
	atomic_thread_fence(memory_order_seq_cst);
#if defined(_WIN32)
	WakeByAddressAll(ptr);
#elif defined(__linux__)
	syscall(
		SYS_futex,
		ptr,
		FUTEX_WAKE,
		INT_MAX,
		NULL,
		NULL,
		0,
	);
#endif
	atomic_thread_fence(memory_order_seq_cst);
}