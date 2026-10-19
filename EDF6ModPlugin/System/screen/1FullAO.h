#pragma once
#include <d3d11.h>

void __fastcall HookFunction_D3D11_FullAO();
void __fastcall HookFunction_D3D11_FullAO_Install(ID3D11Device* device);
void __fastcall D3D11_FullAO_ReloadComputeShader();

void __fastcall D3D11_FullAO_Execute(PG_XglSystemRenderer pRender, Pg_D3D11DeviceInfo pD3D);

namespace D3D {
	constexpr int MAXFullAOLevel = 3;

	class EnhanceFullAO_t {
	public:
		bool m_enable; UINT m_level;
		ID3D11ComputeShader* m_cs[MAXFullAOLevel];
		ID3D11ComputeShader* m_reload_cs; // For use as runtime override

		void Initialize(ID3D11Device* device);
		void LoadComputeShader(ID3D11Device* device, UINT level);
		void GetLevel(int in);

		const D3D_SHADER_MACRO* GetShaderMacro(UINT index);
		void ReloadComputeShader(ID3D11Device* device, LPCWSTR pFileName);
	};
	typedef EnhanceFullAO_t* PEnhanceFullAO;
}
