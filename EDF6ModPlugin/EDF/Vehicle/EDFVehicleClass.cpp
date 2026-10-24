#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "EDFVehicleClass.h"

extern "C" {
	uintptr_t fnVehicleCommon_SetAISeat;
	uintptr_t fnVehicleCommon_SetAISeat_BMX10;
	uintptr_t fnVehicleCommon_CheckSeatIsAllowShooting;
}

void __fastcall EDF_HookFunction_Vehicle(PBYTE hmodDLL) {
	// EDF.dll+633030
	fnVehicleCommon_SetAISeat = (uintptr_t)(hmodDLL + 0x633030);
	// EDF.dll+6490C0
	fnVehicleCommon_SetAISeat_BMX10 = (uintptr_t)(hmodDLL + 0x6490C0);
	// EDF.dll+65F6F0
	fnVehicleCommon_CheckSeatIsAllowShooting = (uintptr_t)(hmodDLL + 0x65F6F0);
}

void __fastcall VehicleVFT190_SetAISeat(void* pThis, void* rdx, int muliSeat) {
	typedef void(__fastcall* callfunc)(void* pThis, void* rdx);
	callfunc fn;
	if (muliSeat){
		fn = (callfunc)fnVehicleCommon_SetAISeat_BMX10;
	} else {
		fn = (callfunc)fnVehicleCommon_SetAISeat;
	}
	fn(pThis, rdx);
}

void __fastcall VehicleVFT230_CheckSeatIsAllowShooting(void* pThis, int seatIndex, void* r8) {
	typedef void(__fastcall* callfunc)(void* pThis, int seatIndex, void* r8);
	auto fn = (callfunc)fnVehicleCommon_CheckSeatIsAllowShooting;
	fn(pThis, seatIndex, r8);
}
