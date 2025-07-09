#pragma once

// Injects hook into game process
void __fastcall WriteHookToProcess(void* addr, void* data, size_t len);
void __fastcall ThrowsProblemAddressInformation(void* addr);
// This will check if the previous address is ecx
void __fastcall WriteHookToProcessCheckECX(void* addr, void* data, size_t len);
// This will check if the previous address is edx
void __fastcall WriteHookToProcessCheckEDX(void* addr, void* data, size_t len);
// update game's original functions, need 12 bytes
void __fastcall hookGameBlock(void* targetAddr, uintptr_t dataAddr);
// update game's original functions use RAX with interruption, need 13 bytes
void __fastcall hookGameBlockRAXWithInt3(void* targetAddr, uintptr_t dataAddr);
// update game's original functions with 14 bytes
void __fastcall hookGameBlock14(void* targetAddr, uintptr_t dataAddr);
// update game's original functions with interruption, need 15 bytes
void __fastcall hookGameBlockWithInt3(void* targetAddr, uintptr_t dataAddr);
// update game's original functions with call, need 12bytes
void __fastcall CallGameBlock(void* targetAddr, uintptr_t dataAddr);
//
extern "C" void* __fastcall __newToAlignedMalloc(size_t size);
//
void __fastcall ForceCrashGame();
