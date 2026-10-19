#pragma once

typedef void(__fastcall* VehicleVFT230_CheckSeatIsAllowShooting)(void* pThis, int seatIndex, void* r8);

void __fastcall EDF_HookFunction_Vehicle403_Tank(PBYTE hmodDLL);
void __fastcall EDF_Vehicle403_Tank_CheckSeatIsAllowShooting230(void* pThis, int seatIndex, void* r8);
