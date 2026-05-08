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

void __fastcall XGS_DXresource_Initialize(PBYTE hmodDLL);

extern "C"{
	Pg_D3D_ResourceInfo __fastcall Get_g_xgl_draw_utility_color_textrure();
}