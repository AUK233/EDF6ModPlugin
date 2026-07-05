#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "Base/g_DXresource.h"
#include "ASFunc_Graphic.h"

namespace angelscriptFunc {
	void SetShadowIntensity(float intensity) {
		auto pRenderer2139A98 = DXGI_GetRenderer2139A98();
		pRenderer2139A98->xgl_additional_light_setting.g_shadow_intensity = intensity;
	}
// end
}
