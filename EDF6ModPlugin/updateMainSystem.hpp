#pragma once

#include "utiliy.h"
#include "commonNOP.h"
#include "EDFPointerStruct.hpp"

#include "allowScriptFail.hpp"

void hook_updateMainSystem_common(PBYTE hmodDLL) {
	// edf.dll+703173, skip epic check
	if (CheckSkipEOS())
	{
		// skip eos
		BYTE skipEOS[] = {
			0xE9, 0x9C, 0x02, 0x00, 0x00,
			0x90
		};
		WriteHookToProcess((void*)(hmodDLL + 0x703173), skipEOS, 6U);
	}
}

extern "C" {
	void __fastcall ASMreadFile735C0(void* pRCX, PEDFWString wstr, void* pR8);
	uintptr_t readFile735C0RetAddr;
}

void __fastcall hook_readFile735C0(void* pRCX, PEDFWString wstr, void* pR8) {
	wchar_t newPath[MAX_PATH];

	if (wstr->size < 16 || wstr->size >= MAX_PATH) {
		goto ReturnFunction;
	}

	if (*(UINT64*)wstr->text != 0x006900720063002F) {
		goto ReturnFunction;
	}

	memcpy(newPath, wstr->text, wstr->size * 2);
	newPath[wstr->size] = 0;
	memcpy(newPath, L"./modtest/", 20U);
	if (std::filesystem::exists(newPath)) {
		memcpy(wstr->text, L"./modtest/", 20U);
	}

	ReturnFunction:
	return ASMreadFile735C0(pRCX, wstr, pR8);
}

void hook_updateMainSystem(PBYTE hmodDLL, int IsTest) {
	hook_updateMainSystem_common(hmodDLL);

	Debug_allowScriptFail(hmodDLL);


	// edf.dll+7B092E, change music file
	if (std::filesystem::exists(L"./sound/pc/add_Music.acb")) {
		int musicNameSize = 28;
		WriteHookToProcess((void*)(hmodDLL + 0x7B092E + 2), &musicNameSize, 4U);
		WriteHookToProcess(hmodDLL + 0x17F0A80, (void*)L"APP:/SOUND/ADX/ADD_MUSIC.ACB", 58U);
	}

	// find 48895C2418488974242055574156488BEC4883EC70488B057CAEF7014833C4488945F8498BF0488BFA488BD9488B01
	// EDF.dll+741c0
	int i_readFile = 0x741c0;
	hookGameBlockWithInt3((void*)(hmodDLL + i_readFile), (uintptr_t)hook_readFile735C0);
	WriteHookToProcess((void*)(hmodDLL + i_readFile + 15), (void*)&nop2, 2U);
	readFile735C0RetAddr = (uintptr_t)(hmodDLL + i_readFile + 17);
}
