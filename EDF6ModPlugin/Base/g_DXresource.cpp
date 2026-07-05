#include "pch.h"

#include "g_DXresource.h"

extern "C" {
	uintptr_t pOutRenderColor2139A78; // edf.dll+2139A78
	PG_TextureBackBuffer* pTextureBackBuffer2137090; // edf.dll+2137090
	PG_Renderer2139A98* pRenderer2139A98; // edf.dll+2139A98
}

void __fastcall XGS_DXresource_Initialize(PBYTE hmodDLL) {
	uintptr_t tempP;
	pOutRenderColor2139A78 = (uintptr_t)(hmodDLL + 0x2139A78);

	tempP = (uintptr_t)(hmodDLL + 0x2137090);
	pTextureBackBuffer2137090 = (PG_TextureBackBuffer*)tempP;

	tempP = (uintptr_t)(hmodDLL + 0x2139A98);
	pRenderer2139A98 = (PG_Renderer2139A98*)tempP;
}

PG_TextureBackBuffer __fastcall DXGI_GetTextureBackBuffer2137090() {
	return *pTextureBackBuffer2137090;
}

PG_Renderer2139A98 __fastcall DXGI_GetRenderer2139A98(){
	return *pRenderer2139A98;
}
