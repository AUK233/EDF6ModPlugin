#pragma once
#include <d3d11.h>

typedef struct g_D3D_ResourceInfo_t {
	ID3D11Texture2D* pTexture;
	ID3D11RenderTargetView* pRTV;
	ID3D11ShaderResourceView* pSRV;
	ID3D11UnorderedAccessView* pUAV;
	int width;
	int height;
} *Pg_D3D_ResourceInfo;


typedef struct alignas(16) G_TextureBackBuffer_t {
	void* vf_table;
	char pad08[8];
	Pg_D3D_ResourceInfo pInfo; void* pResource;
}*PG_TextureBackBuffer;
#if 1
static_assert(offsetof(G_TextureBackBuffer_t, pInfo) == 0x10);
#endif

void __fastcall XGS_DXresource_Initialize(PBYTE hmodDLL);
PG_TextureBackBuffer __fastcall DXGI_GetTextureBackBuffer2137090();

extern "C"{
	Pg_D3D_ResourceInfo __fastcall Get_g_xgl_draw_utility_color_textrure();
}