#include "pch.h"
#include <windows.h>
#include <iostream>
#include <string>

void RunConsole(HMODULE hModule) {
    // 1. Create the console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);

    // Loop
    while (true) {

    }

    // 2. Cleanup
    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunConsole, hModule, 0, NULL);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}