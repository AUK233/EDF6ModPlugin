#pragma once

#include "utiliy.h"
#include "commonNOP.h"
#include <format>

extern "C" {
	int debug_MissionLoadingFail = 0;
	void __fastcall ASMdebug_hookScriptRun();
	uintptr_t Debug_hookScriptRunRetAddr0;
	uintptr_t Debug_hookScriptRunRetAddr1;
}

struct asSMessageInfo {
	const char* file_path;
	int file_line;
	int file_row;
	int msg_type;
	int pad14;
	const char* msg_info;
};

void __fastcall Debug_hookScriptFailedToBuild(void* as, const char* filename) {
	debug_MissionLoadingFail = 1;
}

void __fastcall Debug_hook_asCMessage(asSMessageInfo* msg) {
	// 1 is WARN, 2 is INFO
	if (msg->msg_type == 1)
		return;
	if (msg->msg_type == 2)
		return;

	std::string out_msg = std::format("in \"{}\" (line: {}, row: {})\n  {}\n",
									msg->file_path, msg->file_line, msg->file_row, msg->msg_info);
	MessageBoxA(NULL, out_msg.c_str(), "error", MB_OK);
}

void Debug_allowScriptFail(PBYTE hmodDLL) {
	// find "Script failed to build"
	// edf.dll+1292A8F
	int i_hookScriptFailedToBuild = 0x1292A8F;
	CallGameBlock((void*)(hmodDLL + i_hookScriptFailedToBuild), (uintptr_t)Debug_hookScriptFailedToBuild);
	WriteHookToProcess((void*)(hmodDLL + i_hookScriptFailedToBuild + 12), (void*)&nop6, 6U);
	WriteHookToProcess((void*)(hmodDLL + i_hookScriptFailedToBuild + 18), (void*)&nop6, 6U);

	// find 84C07410488B07488BCFFF90A001000033C0EB0C, then -5
	// edf.dll+70F256
	int i_debug_hookScriptRun = 0x70F256;
	hookGameBlockWithInt3((void*)(hmodDLL + i_debug_hookScriptRun), (uintptr_t)ASMdebug_hookScriptRun);
	WriteHookToProcess((void*)(hmodDLL + i_debug_hookScriptRun + 15), (void*)&nop6, 6U);
	Debug_hookScriptRunRetAddr0 = (uintptr_t)(hmodDLL + i_debug_hookScriptRun + 25);
	Debug_hookScriptRunRetAddr1 = (uintptr_t)(hmodDLL + i_debug_hookScriptRun + 25 + 12);

	// edf.dll+1292EE0
	// find "%s (%d, %d) : %s : %s\n", hook function header
	int i_debug_hookScriptInfo = 0x1292EE0;
	hookGameBlockWithInt3((void*)(hmodDLL + i_debug_hookScriptInfo), (uintptr_t)Debug_hook_asCMessage);
	WriteHookToProcess((void*)(hmodDLL + i_debug_hookScriptInfo + 15), (void*)&nop4, 4U);
}
