#include "pch.h"
#include "framework.h"

#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

//#include <shlwapi.h>
//#pragma comment(lib, "shlwapi.lib")

typedef void(WINAPI* EDFImportDLL)();
EDFImportDLL EDF_CPP_OnBoot;
PBYTE hmodDLL;

extern "C" __declspec(dllexport) void CPP_OnBoot() {
    timeBeginPeriod(1);
    EDF_CPP_OnBoot();
    timeEndPeriod(1);
};

// Injects hook into game process
void __fastcall WriteHookToProcess(void* addr, void* data, size_t len) {
    DWORD oldProtect;
    VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(addr, data, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
}

// update game's original functions
void __fastcall hookGameBlock(void* targetAddr, uintptr_t dataAddr) {
    uint8_t hookFunction[] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
        0xFF, 0xE0                                                  // jmp rax
    };
    memcpy(&hookFunction[2], &dataAddr, sizeof(dataAddr));

    WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

void __fastcall hookGameBlockRAXWithInt3(void* targetAddr, uint64_t dataAddr)
{
    uint8_t hookFunction[] = {
           0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
           0xFF, 0xE0, // jmp rax
           0xCC        // int3
    };
    memcpy(&hookFunction[2], &dataAddr, sizeof(dataAddr));

    WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

// update game's original functions with 14 bytes
void __fastcall hookGameBlock14(void* targetAddr, uint64_t dataAddr) {
    uint8_t hookFunction[] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,            // jmp
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // addr
    };
    memcpy(&hookFunction[6], &dataAddr, sizeof(dataAddr));

    WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

// update game's original functions with interruption, need 15 bytes
void __fastcall hookGameBlockWithInt3(void* targetAddr, uint64_t dataAddr) {
    uint8_t hookFunction[] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,				// jmp
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // addr
        0xCC											// int3
    };
    memcpy(&hookFunction[6], &dataAddr, sizeof(dataAddr));

    WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

extern "C" {
    void __fastcall ASMreadMissionSavaData();
    uintptr_t readMissionSavaDataRetAddr;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        //csharp_RequestBoot
        //csharp_RequestExit
        auto moduleHandle = LoadLibraryW(L"edf.dll");
        if (moduleHandle != nullptr)
        {
            EDF_CPP_OnBoot = (EDFImportDLL)GetProcAddress(moduleHandle, "CPP_OnBoot");

            hmodDLL = (PBYTE)moduleHandle;

            // find PackageName function.
            // edf.dll+E2970, Determine mission savedata location, R8 is 1 when it is online.
            hookGameBlockWithInt3((void*)(hmodDLL + 0xE2970), (uintptr_t)ASMreadMissionSavaData);
            readMissionSavaDataRetAddr = (uintptr_t)(hmodDLL + 0xE2980);
        }
        else {
            // to crash!
            *(int*)0 = 0;
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

