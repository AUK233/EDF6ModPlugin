#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "Base/g_sgoData.h"
#include "EDFVehicle403_Tank.h"

extern "C" {
	void __fastcall ASMeVehicle403_Tank_Initialization();
}

void __fastcall EDF_HookFunction_Vehicle403_Tank(PBYTE hmodDLL) {
	// edf.dll+5FE209
	//int newSize = sizeof(NEW_Vehicle403_Tank_t);
	//WriteHookToProcessCheckECX((void*)(hmodDLL + 0x5FE209 + 1), &newSize, 4);

	// EDF.dll+5FE966
	int i_Init = 0x5FE966;
	hookGameBlockWithInt3((void*)(hmodDLL + i_Init), (uintptr_t)ASMeVehicle403_Tank_Initialization);

	// EDF.dll+17DEC40
	//uintptr_t* VehicleBigBegaruta_VFT = (uintptr_t*)(hmodDLL + 0x17DEC40);

	// EDF.dll+17D8FA0
	uintptr_t* Vehicle403_Tank_VFT = (uintptr_t*)(hmodDLL + 0x17D8FA0);

	auto newFunc190 = (uintptr_t)EDF_Vehicle403_Tank_SetAISeat190;
	WriteHookToProcess(&Vehicle403_Tank_VFT[0x190 / 8], &newFunc190, 8U);

	auto newFunc230 = (uintptr_t)EDF_Vehicle403_Tank_CheckSeatIsAllowShooting230;
	WriteHookToProcess(&Vehicle403_Tank_VFT[0x230 / 8], &newFunc230, 8U);

}

void __fastcall Vehicle403_Tank_Initialization(P_NEW_Vehicle403_Tank pIn) {
	pIn->m_bIsMultipleAISeats = false;

	int bIsMultAI = 0;
	XGS_SGOData_ReadValue_INT32(pIn, L"ai_all_character_enable", &bIsMultAI);
	if (bIsMultAI) pIn->m_bIsMultipleAISeats = true;
}

void __fastcall EDF_Vehicle403_Tank_SetAISeat190(P_NEW_Vehicle403_Tank pThis, void* rdx) {
	VehicleVFT190_SetAISeat(pThis, rdx, pThis->m_bIsMultipleAISeats);
}

void __fastcall EDF_Vehicle403_Tank_CheckSeatIsAllowShooting230(P_NEW_Vehicle403_Tank pThis, int seatIndex, void* r8) {
	VehicleVFT230_CheckSeatIsAllowShooting(pThis, seatIndex, r8);

	if (pThis->m_bIsMultipleAISeats) {
		VehicleVFT230_CheckSeatIsAllowShooting(pThis, 1, r8);
		VehicleVFT230_CheckSeatIsAllowShooting(pThis, 2, r8);
	}
}
