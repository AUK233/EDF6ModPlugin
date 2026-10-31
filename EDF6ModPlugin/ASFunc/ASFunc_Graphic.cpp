#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "Base/g_DXresource.h"
#include "System/screen/1FullAO.h"

#include "ASFunc_Graphic.h"

extern D3D::PEnhanceFullAO g_EnhanceFullAO;

namespace angelscriptFunc {

	void SetEnvAdditionalLightIntensity(int type, float intensity) {
		enum EnvAdditionalLight {
			EnvAdditionalLight_ShadowIntensity = 0,
			EnvAdditionalLight_OutlineColor = 1,
		};

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

	void SetEnvLightningStatus(int type, float f1, float f2, float f3) {
		auto pAO = g_EnhanceFullAO;
		if (!pAO) return;

		enum EnvLightningStatus {
			EnvLightningStatus_Enable = 0,
			EnvLightningStatus_Disable = 1,
			EnvLightningStatus_Color = 2,
			EnvLightningStatus_Intensity = 3,
			EnvLightningStatus_SkyIntensity = 4,
			EnvLightningStatus_Shadow = 5,
			EnvLightningStatus_Interval = 6,
		};

		switch (type) {
		case EnvLightningStatus_Enable: {
			auto pRenderer = DXGI_GetRenderer2139A98();

			pAO->m_lightning.phase = D3D::AddLightningState_t::Phase::Idle;
			pAO->m_lightning.startTime = pRenderer->xgl_system.g_xgl_time + f1;
			return;
		}
		case EnvLightningStatus_Disable:
			pAO->m_lightning.Reset();
			return;
		case EnvLightningStatus_Color:
			pAO->m_lightning.CB.g_lightning_color[0] = f1;
			pAO->m_lightning.CB.g_lightning_color[1] = f2;
			pAO->m_lightning.CB.g_lightning_color[2] = f3;
			return;
		case EnvLightningStatus_Intensity:
			pAO->m_lightning.CB.g_lightning_intensity = f1;
			return;
		case EnvLightningStatus_SkyIntensity:
			pAO->m_lightning.CB.g_lightning_sky_amount = f1;
			return;
		case EnvLightningStatus_Shadow:
			pAO->m_lightning.CB.g_lightning_shadow_darken = f1;
			return;
		case EnvLightningStatus_Interval:
			pAO->m_lightning.strikeInterval = f1;
			return;
		default: return;
		}
	}

	void SetEnvLookupTableBlend(int lutindex, float blendFactor, float brightness) {
		//if (pD3DPostProcess) {
		//	pD3DPostProcess->SetPostProcessInfo(lutindex, blendFactor, brightness);
		//}
		// end
	}
// end
}
