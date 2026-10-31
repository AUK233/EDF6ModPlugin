#pragma once

namespace angelscriptFunc{
	void SetEnvAdditionalLightIntensity(int type, float intensity);
	void SetEnvLightningStatus(int type, float f1, float f2, float f3);
	void SetEnvLookupTableBlend(int lutindex, float blendFactor, float brightness);
}
