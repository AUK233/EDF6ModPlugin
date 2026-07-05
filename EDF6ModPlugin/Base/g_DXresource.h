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

// ===============================================================

typedef struct alignas(16) DX_ShadowSetting_t {
	float m_vp_to_view[4][4];
	float m_proj[4][4];
	float m_distance;
	float m_shadow_adjust;
	float m_shadow_smooth;
	float m_pad;
}*PDX_ShadowSetting;

typedef struct alignas(16) xgl_additional_light_setting_t {
	DX_ShadowSetting_t g_shadow_setting[2];
	float g_shadow_far_fade;
	float g_shadow_far_fade_mul;
	float g_shadow_intensity; // new
	float g_light_setting_pad; // old is float2
	UINT32 g_light_count;
	UINT32 g_point_light_count;
	float g_pad[2];
	float g_bg_color[4];
	float g_light_view_proj[4][4];
}*Pxgl_additional_light_setting;

typedef struct alignas(16) G_Renderer2139A98_t {
	char pad00[0x3b0];
	xgl_additional_light_setting_t xgl_additional_light_setting;
}*PG_Renderer2139A98;
#if 1
static_assert(offsetof(G_Renderer2139A98_t, xgl_additional_light_setting.g_shadow_far_fade) == 0x4D0);
#endif

// ===============================================================

void __fastcall XGS_DXresource_Initialize(PBYTE hmodDLL);
PG_TextureBackBuffer __fastcall DXGI_GetTextureBackBuffer2137090();
PG_Renderer2139A98 __fastcall DXGI_GetRenderer2139A98();

extern "C"{
	Pg_D3D_ResourceInfo __fastcall Get_g_xgl_draw_utility_color_textrure();
}