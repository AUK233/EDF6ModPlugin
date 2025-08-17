#pragma once

//#include "..\utiliy.h"
//#include "..\commonNOP.h"
#include "ASFunc_Definition.hpp"

extern "C" {
	void __fastcall ASMaddAngelScriptFunction();
	extern uintptr_t ptr_edf20B2988;
}


void hook_addAngelScriptFunction(PBYTE hmodDLL) {

	// find "::bbb& opAssign(const ::bbb& in)", is mov rcx, qword ptr [rbp+5768]
	// edf.dll+1F539E
	int i_addAngelScriptFunction = 0x1F539E;
	hookGameBlockWithInt3((void*)(hmodDLL + i_addAngelScriptFunction), (uintptr_t)ASMaddAngelScriptFunction);
	WriteHookToProcess((void*)(hmodDLL + i_addAngelScriptFunction + 15), (void*)&nop10, 10U);
	WriteHookToProcess((void*)(hmodDLL + i_addAngelScriptFunction + 25), (void*)&nop9, 9U);
}


void ASFunc_GivePlayerBox(int armor, int weapon) {
	Pedf20B2988 p20B2988 = *(Pedf20B2988*)ptr_edf20B2988;

	p20B2988->TotalWeaponBox += weapon;
	p20B2988->AcquiredWeaponBox += weapon;

	p20B2988->TotalArmorBox += armor;
	p20B2988->AcquiredArmorBox += armor;

	/*std::wstring wstr = L"GivePlayerBox(" + std::to_wstring(armor) + L")";
	MessageBoxW(nullptr, wstr.c_str(), L"check", MB_OK);*/
}

extern "C" void __fastcall AngelScript_RegisterGlobalFunction(void* asCScriptEngine, asCScriptEngine_RegisterGlobalFunction callc) {
	asSFuncPtr funcPtr(2);
	funcPtr.ptr.f.func = (asFUNCTION_t)ASFunc_GivePlayerBox;
	callc(asCScriptEngine, "void ASFunc_GivePlayerBox(int, int)", funcPtr, asCALL_CDECL, 0);
}
