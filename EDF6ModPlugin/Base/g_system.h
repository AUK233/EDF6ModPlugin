#pragma once
#include <d3d11.h>
#pragma comment(lib, "D3D11.lib")

// size is 0xC20, Initialize at edf.dll+F488E
typedef struct XGS_System_Camera_t {
	void* vf_table;
	char pad8[0x1F8];
	bool bIsPlayerHudShow, pad201[7];
	char pad208[0x148];
	void* pGameObject;
} *PXGS_System_Camera;
static_assert(offsetof(XGS_System_Camera_t, bIsPlayerHudShow) == 0x200);
static_assert(offsetof(XGS_System_Camera_t, pGameObject) == 0x350);

typedef struct XGS_System_Player_t {
	PXGS_System_Camera pCamera;
	char pad8[0x180];
} *PXGS_System_Player;
static_assert(sizeof(XGS_System_Player_t) == 0x188);

// xgs::sysmte::functional in edf.dll+20B2958
typedef struct XGS_System_t {
	void* vf_table; // is edf.dll+1AE2278
	char pad8[8];
	void* ptr10;
	char pad18[0x4C0];
	XGS_System_Player_t player[4]; // really?
} *PXGS_System;
static_assert(offsetof(XGS_System_t, player) == 0x4d8);

void __fastcall XGS_SystemFunction_Initialize(PBYTE hmodDLL);

PXGS_System __fastcall XGS_GetXGSSystemPointer();
PXGS_System_Player __fastcall XGS_GetSystemPlayerPointer(PXGS_System pSystem, int index);

DXGI_SWAP_CHAIN_DESC* __fastcall GetDXGISwapChainDescPointer();
