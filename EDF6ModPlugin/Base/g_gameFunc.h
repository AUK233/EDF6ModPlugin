#pragma once

void __fastcall XGS_GetGameFunction_Initialize(PBYTE hmodDLL);

typedef void* (__fastcall* CallFunc_DetectionPathCollisionObject)(void* pObject, void* pSystem, float* pStartPos, float* pEndPos, float* pHitbox);
CallFunc_DetectionPathCollisionObject __fastcall Get_DetectionPathCollisionObject();