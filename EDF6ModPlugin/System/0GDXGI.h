#pragma once
#include <d3d11.h>
#include <dxgi1_6.h>
#pragma comment(lib, "D3D11.lib")
#include "Base/g_DXresource.h"

typedef struct GameDXGIRender_t {
	void* vf_table; // edf.dll+0x1A3B718
	void* pUnknown008;
	int nUnknown010[2];
	HWND OutputWindow;
	char pad20[0x18];
	int nUnknown038[2]; // {1, 1}
	char pad040[0x04];
	UINT WindowRes[2];
	char pad4C[0x30];
	UINT nFeatureLevel07C;
	ID3D11Device* pD3D11Device;
	ID3D11DeviceContext* pD3D11DeviceContext;
	IDXGISwapChain* pDXGISwapChain;
	int nUnknown098[2];      // {1, 1}
	Pxgl_gs_Texture pTextureBackBuffer;
	DXGI_SWAP_CHAIN_DESC DXGISwapChainDesc;
	char padF0[0x50];
	ID3D11Device* pD3D11Device_Copy;
	ID3D11DeviceContext* pD3D11DeviceContext_Copy;
} *PGameDXGIRender; // edf.dll+2136FF0
#if 1
static_assert(offsetof(GameDXGIRender_t, OutputWindow) == 0x018);
static_assert(offsetof(GameDXGIRender_t, WindowRes) == 0x044);
static_assert(offsetof(GameDXGIRender_t, pD3D11Device) == 0x080);
static_assert(offsetof(GameDXGIRender_t, pD3D11DeviceContext) == 0x088);
static_assert(offsetof(GameDXGIRender_t, pDXGISwapChain) == 0x090);
static_assert(offsetof(GameDXGIRender_t, pTextureBackBuffer) == 0x0A0);
static_assert(offsetof(GameDXGIRender_t, DXGISwapChainDesc) == 0x0A8);
static_assert(offsetof(GameDXGIRender_t, pD3D11Device_Copy) == 0x140);
#endif

typedef struct alignas(16) G_Application_t {
	void* vf_table; // edf.dll+17E91A8, size is 0xF0
	char pad08[0x8];
	float CurrentFPS;
	char pad14[0xC];
	int FramePerSecond;
	char pad24[0xC];
	float FrameInterval; // per second
	char pad34[0x34];
	float GameTime;
	char pad6C[0x24];
	xgl_gs_Texture_t DSVTexture;
}*PG_Application; // edf.dll+2139B90
#if 1
static_assert(offsetof(G_Application_t, CurrentFPS) == 0x10);
static_assert(offsetof(G_Application_t, FramePerSecond) == 0x20);
static_assert(offsetof(G_Application_t, FrameInterval) == 0x30);
static_assert(offsetof(G_Application_t, GameTime) == 0x68);
static_assert(offsetof(G_Application_t, DSVTexture) == 0x90);
#endif

void DXGI_Initialize(PBYTE hmodEXE);

PGameDXGIRender __fastcall DXGI_GetGameDXGIRender();
PG_Application __fastcall DXGI_GetApplication();
