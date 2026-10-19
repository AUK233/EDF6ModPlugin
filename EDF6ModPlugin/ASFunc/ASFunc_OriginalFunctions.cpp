#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "ASFunc_OriginalFunctions.h"

extern "C" {
	uintptr_t ASFunc_IsOnlineMode;
}

void __fastcall ASFunc_OriginalFunctions_Initialize(PBYTE hmodDLL) {
	// edf.dll+1B5DF0
	ASFunc_IsOnlineMode = (uintptr_t)(hmodDLL + 0x1B5DF0);
}

namespace angelscriptFunc {
	bool __fastcall _IsOnlineMode() {
		typedef bool(__fastcall* callfunc)();
		callfunc func = (callfunc)ASFunc_IsOnlineMode;
		return func();
	}
// end
}
