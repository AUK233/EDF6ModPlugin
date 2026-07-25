#pragma once
#include <d3d11.h>

void HookFunction_D3D11_FullAO();
void HookFunction_D3D11_FullAO_Install();
const D3D_SHADER_MACRO* D3D11_FullAO_GetShaderMacro(UINT index);
void __fastcall D3D11_FullAO_ReloadComputeShader();
