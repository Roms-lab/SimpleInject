#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

// Function to find Process ID from name
DWORD GetProcId(const wchar_t* procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32FirstW(hSnap, &procEntry)) {
            do {
                if (!_wcsicmp(procEntry.szExeFile, procName)) {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

int main() {
    std::string dllPath;
    std::string tempProcName;

    // 1. Get User Input Safely
    std::cout << "--- SimpleInject v1.0 ---\n";
    std::cout << "Enter Full DLL Path: ";
    std::getline(std::cin, dllPath);

    std::cout << "Enter Target Process (e.g. CalculatorApp.exe): ";
    std::getline(std::cin, tempProcName);

    // 2. Convert string to Wide String for Windows API
    std::wstring procName(tempProcName.begin(), tempProcName.end());

    // 3. Find the Process
    DWORD procId = GetProcId(procName.c_str());
    if (!procId) {
        std::cerr << "[!] Error: Could not find process '" << tempProcName << "'\n";
        system("pause");
        return 1;
    }

    // 4. Open Process with Permissions
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!hProc) {
        std::cerr << "[!] Error: OpenProcess failed. Try running as Administrator.\n";
        system("pause");
        return 1;
    }

    // 5. Allocate memory in target for the DLL path string
    void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!loc) {
        std::cerr << "[!] Error: Memory allocation failed.\n";
        CloseHandle(hProc);
        return 1;
    }

    // 6. Write DLL path to target memory
    if (!WriteProcessMemory(hProc, loc, dllPath.c_str(), dllPath.length() + 1, 0)) {
        std::cerr << "[!] Error: Failed to write to process memory.\n";
        VirtualFreeEx(hProc, loc, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    // 7. Create Remote Thread to load the DLL
    HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

    if (hThread) {
        std::cout << "[+] Success! DLL injected into " << tempProcName << " (PID: " << procId << ")\n";
        CloseHandle(hThread);
    }
    else {
        std::cerr << "[!] Error: CreateRemoteThread failed (Error Code: " << GetLastError() << ")\n";
    }

    // Cleanup
    CloseHandle(hProc);
    system("pause");
    return 0;
}