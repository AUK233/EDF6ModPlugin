#pragma once

// Injects hook into game process
void __fastcall WriteHookToProcess(void* addr, void* data, size_t len);
// update game's original functions with interruption, need 15 bytes
void __fastcall hookGameBlockWithInt3(void* targetAddr, uintptr_t dataAddr);
