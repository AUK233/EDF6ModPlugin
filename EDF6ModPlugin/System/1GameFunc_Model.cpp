#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "1GameFunc_Model.h"

extern "C" {
	uintptr_t SetModelDisplayStatus6C04B0; // rcx, dl
}

void __fastcall GetGameFunction_Model(PBYTE hmodDLL) {
	SetModelDisplayStatus6C04B0 = (uintptr_t)(hmodDLL + 0x6C04B0);
}
