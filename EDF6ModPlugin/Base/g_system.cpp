#include "../pch.h"
#include "../utiliy.h"
#include "g_gameFunc.h"
#include "g_system.h"

extern "C" {
	uintptr_t _F_XGS_System_Pointer_; // edf.dll+20B2958
}

DXGI_SWAP_CHAIN_DESC* pDXGISwapChainDesc;

void __fastcall XGS_SystemFunction_Initialize(PBYTE hmodDLL)
{
	_F_XGS_System_Pointer_ = (uintptr_t)(hmodDLL + 0x20B2958);

	// yeah
	auto tempP = hmodDLL + 0x2136FF0 + 0xA8;
	pDXGISwapChainDesc = (DXGI_SWAP_CHAIN_DESC*)tempP;

	XGS_GetGameFunction_Initialize(hmodDLL);
}

PXGS_System __fastcall XGS_GetXGSSystemPointer()
{
	return *(PXGS_System*)_F_XGS_System_Pointer_;
}

PXGS_System_Player __fastcall XGS_GetSystemPlayerPointer(PXGS_System pSystem, int index)
{
	// If it's null, it just crashes the game.
	return &pSystem->player[index];
}

DXGI_SWAP_CHAIN_DESC* __fastcall GetDXGISwapChainDescPointer()
{
	return pDXGISwapChainDesc;
}
