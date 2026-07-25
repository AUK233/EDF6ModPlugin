#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "Base/g_DXresource.h"
#include "System/screen/DLSS/1SetPostProcess.h"

#include "ASFunc_Graphic.h"

extern D3D::PD3DPostProcess pD3DPostProcess;

namespace angelscriptFunc {
	enum EnvAdditionalLight {
		EnvAdditionalLight_ShadowIntensity = 0,
		EnvAdditionalLight_OutlineColor = 1,
	};

	void SetEnvAdditionalLightIntensity(int type, float intensity) {
		auto pRenderer2139A98 = DXGI_GetRenderer2139A98();
		switch (type) {
		case EnvAdditionalLight_ShadowIntensity:
			pRenderer2139A98->xgl_additional_light_setting.g_shadow_intensity = intensity;
			return;
		case EnvAdditionalLight_OutlineColor:
			pRenderer2139A98->xgl_additional_light_setting.g_outline_color = intensity;
			return;
		default: return;
		}
		// end
	}

	void SetEnvLookupTableBlend(int lutindex, float blendFactor, float brightness) {
		if (pD3DPostProcess) {
			pD3DPostProcess->SetPostProcessInfo(lutindex, blendFactor, brightness);
		}
		// end
	}
// end
}
