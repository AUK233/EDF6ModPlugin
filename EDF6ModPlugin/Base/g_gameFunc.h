#pragma once
#include "SSE.hpp"

void __fastcall XGS_GetGameFunction_Initialize(PBYTE hmodDLL);

void __fastcall Game_DetectionPathCollisionObject(void* pObject, void* pSystem, float* pStartPos, float* pEndPos, float* pHitbox);

extern "C" {
	void* __fastcall Game_CreateObjectFromObjectFactory(void* useless, Matrix3D_t* pMatrix, void* pFactory, void* pObjectParameter);
}
