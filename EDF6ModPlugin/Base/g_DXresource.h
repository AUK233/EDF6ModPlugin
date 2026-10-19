#pragma once
#include <d3d11.h>


typedef struct g_D3D11DeviceInfo_t {
	ID3D11Device* device; // maybe?
	ID3D11DeviceContext* context;
	char pad10[0x680];
	ID3D11UnorderedAccessView** pCurrentCSUnorderedAccessViews[3];
	char pad6A8[0x310];
	ID3D11ComputeShader* pCurrentCSShader;
	char pad9C0[0x288];
	ID3D11SamplerState** pCurrentCSSamplerState[12];
	char padCA8[0x250];
	ID3D11Buffer** pCurrentCSBuffer[11];
	char padF50[0x1418];
	ID3D11ShaderResourceView** pCurrentCSShaderResourceView[16];
} *Pg_D3D11DeviceInfo;
#if 1
static_assert(offsetof(g_D3D11DeviceInfo_t, pCurrentCSUnorderedAccessViews) == 0x690);
static_assert(offsetof(g_D3D11DeviceInfo_t, pCurrentCSShader) == 0x9B8);
static_assert(offsetof(g_D3D11DeviceInfo_t, pCurrentCSSamplerState) == 0xC48);
static_assert(offsetof(g_D3D11DeviceInfo_t, pCurrentCSBuffer) == 0xEF8);
static_assert(offsetof(g_D3D11DeviceInfo_t, pCurrentCSShaderResourceView) == 0x2368);
#endif

// ===============================================================

union xgl_gs_ShaderResource_u {
	ID3D11ComputeShader* cs;
};

typedef struct xgl_gs_ShaderResource_t {
	void* vf_table;
	char pad8[0x8];
	xgl_gs_ShaderResource_u m_shader;
	char pad18[0x8];
}*Pxgl_gs_ShaderResource;
#if 1
static_assert(offsetof(xgl_gs_ShaderResource_t, m_shader) == 0x10);
#endif

typedef struct xgl_gs_Shader_t {
	void* vf_table;
	xgl_gs_ShaderResource_u* pShader;
	char pad10[0x10];
}*Pxgl_gs_Shader;
#if 1
static_assert(offsetof(xgl_gs_Shader_t, pShader) == 8);
#endif

typedef struct g_D3D_ResourceInfo_t {
	ID3D11Texture2D* pTexture;
	ID3D11RenderTargetView* pRTV;
	ID3D11ShaderResourceView* pSRV;
	ID3D11UnorderedAccessView* pUAV;
	UINT width;
	UINT height;
} *Pg_D3D_ResourceInfo;

typedef struct xgl_gs_TextureBase_t {
	void* vf_table; // edf.dll+179ED18, size 0x20
	char pad8[0x8];
	Pg_D3D_ResourceInfo pInfo;
	char pad18[8];
}*Pxgl_gs_TextureBase;
#if 1
static_assert(offsetof(xgl_gs_TextureBase_t, pInfo) == 0x10);
#endif

typedef struct xgl_gs_TextureResource_t : xgl_gs_TextureBase_t {
	// vft is edf.dll+1A3B8B8, size 0x28
	char pad20[8];
}*Pxgl_gs_TextureResource;
#if 1
static_assert(offsetof(xgl_gs_TextureResource_t, pInfo) == 0x10);
#endif

typedef struct xgl_gs_Texture_t {
	void* vf_table; // edf.dll+1762008
	void* pDXGI; // PGameDXGIRender
	Pg_D3D_ResourceInfo pInfo;
	Pxgl_gs_TextureResource pLock;
}*Pxgl_gs_Texture;
#if 1
static_assert(offsetof(xgl_gs_Texture_t, pInfo) == 0x10);
#endif

typedef struct g_DeferredRenderSystem_GBufferData_t {
	void* pDXGI; // PGameDXGIRender
	void* pad8;
	void* pLock; // vft is edf.dll+1A3BE28, size 0x18
	xgl_gs_Texture_t RT[8];
}*Pg_DeferredRenderSystem_GBufferData;
#if 1
static_assert(offsetof(g_DeferredRenderSystem_GBufferData_t, RT[0].pInfo) == 0x28);
#endif

typedef struct g_DeferredRenderSystem_GBuffer_t {
	void* vf_table; // edf.dll+1AE2540, size 0x128
	int pad8[2];
	g_DeferredRenderSystem_GBufferData_t data;
}*Pg_DeferredRenderSystem_GBuffer;
#if 1
static_assert(sizeof(g_DeferredRenderSystem_GBuffer_t) == 0x128);
#endif


typedef struct alignas(16) G_TextureBackBuffer_t {
	void* vf_table;
	char pad08[8];
	Pg_D3D_ResourceInfo pInfo; void* pResource;
}*PG_TextureBackBuffer;
#if 1
static_assert(offsetof(G_TextureBackBuffer_t, pInfo) == 0x10);
#endif

// ===============================================================

typedef struct xgl_gs_ConstantBufferData_t {
	ID3D11Buffer* pBuffer;
	char pad8[0x10];
	int m_size;
}*Pxgl_gs_ConstantBufferData;
#if 1
static_assert(offsetof(xgl_gs_ConstantBufferData_t, m_size) == 0x18);
#endif

typedef struct xgl_gs_ConstantBufferResource_t {
	void* vf_table; // edf.dll+1A3B8B8, size 0x28
	char pad8[0x8];
	Pxgl_gs_ConstantBufferData pInfo;
	char pad18[0x10];
}*Pxgl_gs_ConstantBufferResource;
#if 1
static_assert(offsetof(xgl_gs_TextureResource_t, pInfo) == 0x10);
#endif

typedef struct xgl_gs_ConstantBuffer_t {
	void* vf_table; // edf.dll+178FB38
	Pxgl_gs_ConstantBufferData m10; // same as pInfo
	Pxgl_gs_ConstantBufferResource pLock;
	void* pDXGI; // PGameDXGIRender
	Pxgl_gs_ConstantBufferData pInfo;
}*Pxgl_gs_ConstantBuffer;
#if 1
static_assert(sizeof(xgl_gs_ConstantBuffer_t) == 0x28);
#endif

typedef struct alignas(16) xgl_system_Light_t {
	float m_vector[3], m_pad0;
	float m_color[3], m_pad1;
	float m_specular_color[3], m_pad2;
}*Pxgl_system_Light;

typedef struct alignas(16) xgl_system_CB_t {
	float g_xgl_view[4][4];
	float g_xgl_view_inverse[4][4];
	float g_xgl_projection[4][4];
	float g_xgl_view_projection[4][4];
	float g_xgl_view_projection_inverse[4][4];
	UINT32 g_xgl_target_dimension[2];
	float g_xgl_time;
	int g_xgl_id;
	xgl_system_Light_t g_xgl_light[4];
	float g_xgl_ambient_color[3];
	float g_xgl_sub_light_specular_factor_max;
	float g_xgl_fog_mul;
	float g_xgl_fog_add;
	float g_xgl_fog_curve;
	float g_xgl_brightness_fog_mul;
	float g_xgl_brightness_fog_add;
	float g_xgl_brightness_fog_curve;
	float g_xgl_fade_mul;
	float g_xgl_fade_add;
	float g_xgl_fog_color[4];
	float g_xgl_brightness_fog_color[4];
}*Pxgl_system_CB;
#if 1
static_assert(sizeof(xgl_system_CB_t) == 0x260);
#endif

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
	float g_outline_color; // new
	UINT32 g_light_count;
	UINT32 g_point_light_count;
	float g_pad[2];
	float g_bg_color[4];
	float g_light_view_proj[4][4];
}*Pxgl_additional_light_setting;
#if 1
static_assert(sizeof(xgl_additional_light_setting_t) == 0x190);
#endif

// ===============================================================
typedef struct alignas(16) G_XglSystemRenderer_t {
	void* pUnk000; // Invalid
	void* pUnk008; // vft: d3d11.dll+1CC6B8 
	xgl_system_CB_t xgl_system;
	char pad270[0x10];
	char pad280[0x130];
	xgl_additional_light_setting_t xgl_additional_light_setting;
	char pad540[0x158];
	xgl_gs_TextureBase_t pTexture6A8;
	char pad6B8[0x40];
	float f6F8, f6FC;
	char pad700[0x50];
	g_DeferredRenderSystem_GBufferData_t GBuffer;
	char pad868[0x270];
	xgl_gs_Shader_t mainColorCS;
	xgl_gs_Shader_t mainColorCS_UG;
	char padB18[0x28];
	xgl_gs_ConstantBuffer_t CB_xgl_system;
	xgl_gs_ConstantBuffer_t CB_xgl_additional_light_setting;
} *PG_XglSystemRenderer, *PG_Renderer2139A98; // edf.dll+2139A98
#if 1
static_assert(offsetof(G_XglSystemRenderer_t, xgl_system) == 0x10);
static_assert(offsetof(G_XglSystemRenderer_t, xgl_additional_light_setting.g_shadow_far_fade) == 0x4D0);
static_assert(offsetof(G_XglSystemRenderer_t, pTexture6A8.pInfo) == 0x6A8);
static_assert(offsetof(G_XglSystemRenderer_t, GBuffer) == 0x750);
static_assert(offsetof(G_XglSystemRenderer_t, GBuffer.RT[0].pInfo) == 0x778);
static_assert(offsetof(G_XglSystemRenderer_t, GBuffer.RT[5].pInfo) == 0x818);
static_assert(offsetof(G_XglSystemRenderer_t, GBuffer.RT[7].pInfo) == 0x858);
static_assert(offsetof(G_XglSystemRenderer_t, mainColorCS) == 0xAD8);
static_assert(offsetof(G_XglSystemRenderer_t, mainColorCS_UG) == 0xAF8);
static_assert(offsetof(G_XglSystemRenderer_t, CB_xgl_system) == 0xB40);
#endif

// ===============================================================

void __fastcall XGS_DXresource_Initialize(PBYTE hmodDLL);

PG_TextureBackBuffer __fastcall DXGI_GetTextureBackBuffer2137090();
PG_Renderer2139A98 __fastcall DXGI_GetRenderer2139A98();
Pg_D3D11DeviceInfo __fastcall DXGI_D3D11Device2137130();

extern "C"{
	Pg_D3D_ResourceInfo __fastcall Get_g_xgl_draw_utility_color_textrure();
}