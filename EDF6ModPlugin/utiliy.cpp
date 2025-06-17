#include "pch.h"
#include "utiliy.h"

extern PBYTE hmodDLL;

// Injects hook into game process
void __fastcall WriteHookToProcess(void* addr, void* data, size_t len) {
	DWORD oldProtect;
	VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(addr, data, len);
	VirtualProtect(addr, len, oldProtect, &oldProtect);
	// Refresh cpu instruction?
	//FlushInstructionCache(handleEXE, addr, len);
}

void __fastcall ThrowsProblemAddressInformation(void* addr)
{
	uintptr_t rva = (uintptr_t)addr - (uintptr_t)hmodDLL;
	std::wstring message = L"Wrong address: +0x";
	message += std::format(L"{:0X}", rva);
	HANDLE tempHND = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ForceCrashGame, NULL, NULL, NULL);
	if (tempHND) {
		CloseHandle(tempHND);
	}

	MessageBoxW(NULL, message.c_str(), L"warning", MB_OK);
}

// This will check if the previous address is ecx
void __fastcall WriteHookToProcessCheckECX(void* addr, void* data, size_t len)
{
	if (*((BYTE*)addr - 1) == 0xB9) {
		return WriteHookToProcess(addr, data, len);
	}
	else {
		return ThrowsProblemAddressInformation(addr);
	}
}

// This will check if the previous address is edx
void __fastcall WriteHookToProcessCheckEDX(void* addr, void* data, size_t len)
{
	if (*((BYTE*)addr - 1) == 0xBA) {
		return WriteHookToProcess(addr, data, len);
	}
	else {
		return ThrowsProblemAddressInformation(addr);
	}
}

// update game's original functions
void __fastcall hookGameBlock(void* targetAddr, uintptr_t dataAddr) {
	BYTE hookFunction[] = {
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
		0xFF, 0xE0                                                  // jmp rax
	};
	memcpy(&hookFunction[2], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

void __fastcall hookGameBlockRAXWithInt3(void* targetAddr, uint64_t dataAddr)
{
	BYTE hookFunction[] = {
		   0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
		   0xFF, 0xE0, // jmp rax
		   0xCC        // int3
	};
	memcpy(&hookFunction[2], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

// update game's original functions with 14 bytes
void __fastcall hookGameBlock14(void* targetAddr, uintptr_t dataAddr) {
	BYTE hookFunction[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,            // jmp
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // addr
	};
	memcpy(&hookFunction[6], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

// update game's original functions with interruption, need 15 bytes
void __fastcall hookGameBlockWithInt3(void* targetAddr, uintptr_t dataAddr) {
	BYTE hookFunction[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,				// jmp
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // addr
		0xCC											// int3
	};
	memcpy(&hookFunction[6], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

// update game's original functions with call, need 12bytes
void __fastcall CallGameBlock(void* targetAddr, uintptr_t dataAddr) {
	BYTE hookFunction[] = {
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
		0xFF, 0xD0                                                  // call rax
	};
	memcpy(&hookFunction[2], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

void* __fastcall __newToAlignedMalloc(size_t size)
{
	void* p = _aligned_malloc(size, 0x10);
	if (!p) {
		*(int*)0 = 0xDE202;
	}
	return p;
}

// Force the game to end if the user is not responding for a long time
void __fastcall ForceCrashGame()
{
	Sleep(60000);
	*(int*)0 = 0;
}

int CheckSkipEOS() {
	FILE* file = nullptr;
	errno_t err = fopen_s(&file, "LaunchGame.ini", "r");
	if (err || !file) {
		return 0;
	}

	char line[256];
	int value = 0;

	while (fgets(line, sizeof(line), file)) {
		line[strcspn(line, "\r\n")] = '\0';

		if (!strncmp(line, "NoOperation=", 12)) {
			value = atoi(line + 12);
			break;
		}
	}

	fclose(file);
	return value;
}
