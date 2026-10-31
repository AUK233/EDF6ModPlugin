#pragma once
#include <d3d11.h>

void __fastcall HookFunction_D3D11_FullAO();
void __fastcall HookFunction_D3D11_FullAO_Install(ID3D11Device* device);
void __fastcall D3D11_FullAO_ReloadComputeShader();

void __fastcall D3D11_FullAO_Execute(PG_XglSystemRenderer pRender, Pg_D3D11DeviceInfo pD3D);

void __fastcall D3D11_FullAO_MixGroundAndSky(void* noRCX, Pg_D3D11DeviceInfo pD3D, Pxgl_gs_TextureBase outTexture, Pxgl_gs_TextureBase inTexture);

namespace D3D {
	constexpr int MAXFullAOLevel = 5;


	typedef struct alignas(16) AddLightningStateConstants_t {
		float g_lightning_color[3];
		float g_lightning_fade;
		float g_lightning_intensity;
		float g_lightning_shadow_darken;
		float g_lightning_sky_amount;
		float g_lightning_pad;
	}*PAddLightningStateConstants;

	class AddLightningState_t {
	public:
		enum class Phase { Disable, Idle, Wait, Strike, Fade };

		Phase phase;
		int flashCount, flashIndex;
		float startTime, strikeDuration, fadeDuration, flashInterval;
		float strikeInterval;

		AddLightningStateConstants_t CB;

		void Reset();
		void ResetCB();

		void UpdatePhase(const float* pTime);
	};

	class EnhanceFullAO_t {
	public:
		bool m_enable; UINT m_level;
		ID3D11ComputeShader* m_cs[MAXFullAOLevel];
		ID3D11ComputeShader* m_reload_cs; // For use as runtime override
		ID3D11ComputeShader* m_csMixSky;

		ID3D11Buffer* CB_Lightning;
		ID3D11ComputeShader* m_csLightning;
		AddLightningState_t m_lightning;

		void Initialize(ID3D11Device* device);
		void LoadMainComputeShader(ID3D11Device* device, UINT level);
		void LoadVolLightComputeShader(ID3D11Device* device);
		void LoadLightningComputeShader(ID3D11Device* device);
		void GetLevel(int in);

		const D3D_SHADER_MACRO* GetShaderMacro(UINT index);
		void ReloadMainComputeShader(ID3D11Device* device, LPCWSTR pFileName);
		void ReloadVolLightComputeShader(ID3D11Device* device, LPCWSTR pFileName);
		void ReloadLightningComputeShader(ID3D11Device* device, LPCWSTR pFileName);

		bool UpdateLightning(PG_XglSystemRenderer pRender, ID3D11DeviceContext* context);
	};
	typedef EnhanceFullAO_t* PEnhanceFullAO;
}
