#pragma once
#include <stdio.h>
#include <stdarg.h>

#include <windows.h>


int main() {
	LPCH win_env = GetEnvironmentStringsA();

	printf("%s\n", win_env);

	FreeEnvironmentStringsA(win_env);
}