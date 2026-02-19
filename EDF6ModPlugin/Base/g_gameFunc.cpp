#include "../pch.h"
#include "g_system.h"

#include "g_gameFunc.h"

extern "C" {
	uintptr_t _F_DetectionPathCollisionObject_; // edf.dll+11A70B0

	uintptr_t _F_CreateObjectFromObjectFactory_; // edf.dll+1194280
}

void __fastcall XGS_GetGameFunction_Initialize(PBYTE hmodDLL)
{
	_F_DetectionPathCollisionObject_ = (uintptr_t)(hmodDLL + 0x11A70B0);

	_F_CreateObjectFromObjectFactory_ = (uintptr_t)(hmodDLL + 0x1194280);
}

void __fastcall Game_DetectionPathCollisionObject(void* pObject, void* pSystem, float* pStartPos, float* pEndPos, float* pHitbox)
{
	// pStartPos is 4 floats
	// pEndPos is 4 floats
	// pHitbox is 4 floats
	typedef void(__fastcall* CallFunc)(void* pObject, void* pSystem, float* pStartPos, float* pEndPos, float* pHitbox);
	CallFunc fn = (CallFunc)_F_DetectionPathCollisionObject_;
	return fn(pObject, pSystem, pStartPos, pEndPos, pHitbox);
}

void* __fastcall Game_CreateObjectFromObjectFactory(void* useless, Matrix3D_t* pMatrix, void* pFactory, void* pObjectParameter)
{
	typedef void* (__fastcall* CallFunc)(PXGS_System pSys, Matrix3D_t* pMatrix, void* pFactory, void* pObjectParameter);
	CallFunc fn = (CallFunc)_F_CreateObjectFromObjectFactory_;
	return fn(XGS_GetXGSSystemPointer(), pMatrix, pFactory, pObjectParameter);
}
