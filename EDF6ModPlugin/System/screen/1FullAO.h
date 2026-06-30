#pragma once
#include <d3d11.h>

void HookFunction_D3D11_FullAO(PBYTE hmodDLL);
void HookFunction_D3D11_FullAO_Install(ID3D11Device* pDevice);
