#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "EDFVehicle403_Tank.h"

extern "C" {
	VehicleVFT230_CheckSeatIsAllowShooting fnVehicleCommon_CheckSeatIsAllowShooting;
}

void __fastcall EDF_HookFunction_Vehicle403_Tank(PBYTE hmodDLL) {
	// EDF.dll+17DEC40
	uintptr_t* VehicleBigBegaruta_VFT = (uintptr_t*)(hmodDLL + 0x17DEC40);
	uintptr_t SetVehicleSeatAI190 = VehicleBigBegaruta_VFT[0x190 / 8];

	// EDF.dll+17D8FA0
	uintptr_t* Vehicle403_Tank_VFT = (uintptr_t*)(hmodDLL + 0x17D8FA0);
	WriteHookToProcess(&Vehicle403_Tank_VFT[0x190 / 8], &SetVehicleSeatAI190, 8U);

	// EDF.dll+65F6F0
	fnVehicleCommon_CheckSeatIsAllowShooting = (VehicleVFT230_CheckSeatIsAllowShooting)(hmodDLL + 0x65F6F0);


	auto newFunc230 = (uintptr_t)EDF_Vehicle403_Tank_CheckSeatIsAllowShooting230;
	WriteHookToProcess(&Vehicle403_Tank_VFT[0x230 / 8], &newFunc230, 8U);
}

void __fastcall EDF_Vehicle403_Tank_CheckSeatIsAllowShooting230(void* pThis, int seatIndex, void* r8) {
	fnVehicleCommon_CheckSeatIsAllowShooting(pThis, seatIndex, r8);
	fnVehicleCommon_CheckSeatIsAllowShooting(pThis, 1, r8);
	fnVehicleCommon_CheckSeatIsAllowShooting(pThis, 2, r8);
}
