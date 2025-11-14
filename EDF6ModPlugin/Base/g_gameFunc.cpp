#include "../pch.h"
#include "g_gameFunc.h"

extern "C" {
	uintptr_t _F_DetectionPathCollisionObject_; // edf.dll+11A70B0
}

void __fastcall XGS_GetGameFunction_Initialize(PBYTE hmodDLL)
{
	_F_DetectionPathCollisionObject_ = (uintptr_t)(hmodDLL + 0x11A70B0);
}

CallFunc_DetectionPathCollisionObject __fastcall Get_DetectionPathCollisionObject()
{
	return (CallFunc_DetectionPathCollisionObject)_F_DetectionPathCollisionObject_;
}
