#include "pch.h"

#include "lib/nvsdk_ngx.h"
#include "lib/nvsdk_ngx_defs.h"
#include "lib/nvsdk_ngx_params.h"
#include "lib/nvsdk_ngx_helpers.h"

#include "Base/g_system.h"
#include "1SetPostProcess.h"

#include "0SetDLSS.h"

//#define DEBUGMODE
__declspec(align(16)) typedef struct NGX_DLSS_t {
	ID3D11Texture2D* ColorBuffer;
	ID3D11Texture2D* OutColor;
	ID3D11Texture2D* DepthBuffer;
	int resolution[2];
	NVSDK_NGX_Parameter* m_ngxParameters;
	NVSDK_NGX_Handle* m_dlssFeature;
	int m_bDlssAvailable;
	int JitterIndex;
	int IsReset;
	int IsReady;
	int IsSplitScreen;
} *PNGX_DLSS;

PNGX_DLSS pNGX_dlss = nullptr;
D3D::PD3DPostProcess pD3DPostProcess = nullptr;

extern "C" {
	extern int Config_PostProcess;
	extern int Config_DLAA;
}

void __fastcall DLSS_CreateFeature(int playerCount) {
	if (!pD3DPostProcess) return;
	if (pD3DPostProcess->PlayerCount == playerCount) return;

	pD3DPostProcess->PlayerCount = playerCount;

	auto pTexBack = DXGI_GetTextureBackBuffer2137090();
	auto pBuffer = pTexBack->pInfo;

	int res[2];
	res[0] = pBuffer->width;
	res[1] = pBuffer->height;
	if (playerCount == 2) {
		res[0] /= 2;
	}

	pD3DPostProcess->SetBuffer(res[0], res[1]);

	if (pNGX_dlss) {
		if (playerCount == 2) {
			pNGX_dlss->IsSplitScreen = 1;
		} else {
			pNGX_dlss->IsSplitScreen = 0;
		}

		DLSS_SetFeature(pD3DPostProcess->Context, res[0], res[1]);
	}
	// end
}

void* __fastcall DLSS_Draw(ID3D11DeviceContext* pContext, int* saveRAX){
	if (!pD3DPostProcess) return saveRAX;

	int playerIndex = 0;
	// yeah, use this check is p1 or p2
	if (saveRAX[0x60 / 4]) playerIndex = 1;


	auto pPlayer = XGS_GetSystemPlayerPointer(XGS_GetXGSSystemPointer(), playerIndex);
	auto pCamera = pPlayer->pCamera;
	if (pCamera) {
		auto pGameObject = (int*)(pCamera->pGameObject);
		int currentTime = pGameObject[0x310 / 4]; // it's a float second, but we use int to check if it has changed.
		if (currentTime != pD3DPostProcess->CurrentTime[playerIndex]) {
			pD3DPostProcess->CurrentTime[playerIndex] = currentTime;
		} else {
			return saveRAX;
		}
		// end
	}else{ return saveRAX; }


	//auto pColorRes = Get_g_xgl_draw_utility_color_textrure();
	auto pColorRes = (Pg_D3D_ResourceInfo)pPlayer->pDrawColorInfo;
	if (!pColorRes) return saveRAX;
	if (!pColorRes->pUAV) return saveRAX;

	auto pDSVInfo = (Pg_D3D_ResourceInfo)pPlayer->pDSVInfo;
	if (!pDSVInfo) return saveRAX;
	if (!pDSVInfo->pSRV) return saveRAX;


	D3D11_TEXTURE2D_DESC inDesc;
	auto pTexture = pColorRes->pTexture;
	pTexture->GetDesc(&inDesc);
	if (inDesc.Format != DXGI_FORMAT_R16G16B16A16_FLOAT) return saveRAX;

	if (Config_PostProcess) {
		pContext->CSSetShaderResources(0, 1, &pColorRes->pSRV);
		pContext->CSSetShaderResources(1, 1, &pDSVInfo->pSRV);
		pContext->CSSetShader(pD3DPostProcess->PostProcessCS, nullptr, 0);
		pContext->CSSetUnorderedAccessViews(0, 1, &pD3DPostProcess->OutUAV[playerIndex], nullptr);
		pContext->Dispatch((pColorRes->width + 15) / 16, (pColorRes->height + 15) / 16, 1);
	} else if (pNGX_dlss) {
		//pContext->CopyResource(pD3DPostProcess->OutColor[playerIndex], Pg_D3D_ResourceInfo(pPlayer->pDSV140Info)->pTexture);
		pContext->CopyResource(pD3DPostProcess->OutColor[playerIndex], pColorRes->pTexture);
	} else { return saveRAX; }

	// evaluate dlss
	if (pNGX_dlss) {
		pNGX_dlss->ColorBuffer = pD3DPostProcess->OutColor[playerIndex];
		pD3DPostProcess->Context = pContext;

		pNGX_dlss->OutColor = pColorRes->pTexture;
		pNGX_dlss->resolution[0] = pColorRes->width;
		pNGX_dlss->resolution[1] = pColorRes->height;

		pNGX_dlss->DepthBuffer = pDSVInfo->pTexture;

		DLSS_Evaluate();
	} else {
		pContext->CopyResource(pColorRes->pTexture, pD3DPostProcess->OutColor[playerIndex]);
	}

	return saveRAX;
}

void __fastcall DLSS_Release(){

	if (pD3DPostProcess) {
		pD3DPostProcess->ReleaseBuffer();
		_aligned_free(pD3DPostProcess);
		pD3DPostProcess = nullptr;
	}

	if (pNGX_dlss) {
		NVSDK_NGX_D3D11_DestroyParameters(pNGX_dlss->m_ngxParameters);
		if (pNGX_dlss->m_bDlssAvailable) {
			NVSDK_NGX_D3D11_ReleaseFeature(pNGX_dlss->m_dlssFeature);
		}

		_aligned_free(pNGX_dlss);
		pNGX_dlss = nullptr;

		NVSDK_NGX_D3D11_Shutdown1(nullptr);
	}
	// end
}

void __fastcall DLSS_Initialization(ID3D11Device** ppDevice, ID3D11DeviceContext** ppImmediateContext, DXGI_SWAP_CHAIN_DESC* pChainDesc) {
	auto device = *ppDevice;
	auto context = *ppImmediateContext;

	if (!pD3DPostProcess) {
		auto pD3D = (D3D::PD3DPostProcess)_aligned_malloc(sizeof(D3D::D3DPostProcess_t), 16U);
		if (pD3D) {
			pD3DPostProcess = pD3D;
			ZeroMemory(pD3D, sizeof(D3D::D3DPostProcess_t));
			pD3D->Initialize(device, context, pChainDesc);
		}
		// end
	}

	if (!Config_DLAA || pNGX_dlss) return;

	auto p = (PNGX_DLSS)_aligned_malloc(sizeof(NGX_DLSS_t), 16U);
	if (!p) return;

	ZeroMemory(p, sizeof(NGX_DLSS_t));
	pNGX_dlss = p;
	p->JitterIndex = 1;

	/*
	NVSDK_NGX_FeatureCommonInfo featureCommonInfo = {};
	featureCommonInfo.LoggingInfo.DisableOtherLoggingSinks = false;
	featureCommonInfo.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_VERBOSE;
	
	NVSDK_NGX_D3D11_Init(231313132, L"Z:\\TEMP", device, &featureCommonInfo);
	0x3AC09EF
	*/

	auto slresult =  NVSDK_NGX_D3D11_Init(231313132, L".", device); // this id is from ngx_dlss_demo 
	if (slresult != NVSDK_NGX_Result_Success) {
		DLSS_TriggerFailureResult((UINT32)slresult - NVSDK_NGX_Result_Fail, 1);
		return;
	}

	slresult = NVSDK_NGX_D3D11_GetCapabilityParameters(&pNGX_dlss->m_ngxParameters);
	if (slresult != NVSDK_NGX_Result_Success) {
		DLSS_TriggerFailureResult((UINT32)(slresult - NVSDK_NGX_Result_Fail) + 200, 1);
		return;
	}

	NVSDK_NGX_Parameter_SetUI(p->m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, NVSDK_NGX_DLSS_Hint_Render_Preset_Default);
}

void __fastcall DLSS_TriggerFailureResult(UINT32 slresult, int FreeDLSS) {
#if defined(DEBUGMODE)
	MessageBoxW(NULL, std::to_wstring((UINT32)slresult).c_str(), L"error", MB_OK);
#endif

	if (FreeDLSS) {
		if (pNGX_dlss) {
			_aligned_free(pNGX_dlss);
			pNGX_dlss = nullptr;
		}
	}
}

void __fastcall DLSS_SetFeature(ID3D11DeviceContext* pContext, UINT Width, UINT Height) {
	auto p = pNGX_dlss;

	if (!p) return;
	p->m_bDlssAvailable = 0;

	if (p->m_bDlssAvailable) {
		NVSDK_NGX_D3D11_ReleaseFeature(pNGX_dlss->m_dlssFeature);
		DLSS_Reset();
	}

	NVSDK_NGX_DLSS_Create_Params DlssCreateParams;
	memset(&DlssCreateParams, 0, sizeof(DlssCreateParams));

	DlssCreateParams.Feature.InWidth = Width;
	DlssCreateParams.Feature.InHeight = Height;
	DlssCreateParams.Feature.InTargetWidth = Width;
	DlssCreateParams.Feature.InTargetHeight = Height;
	DlssCreateParams.Feature.InPerfQualityValue = NVSDK_NGX_PerfQuality_Value_DLAA;
	DlssCreateParams.InFeatureCreateFlags = NVSDK_NGX_DLSS_Feature_Flags_None | NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
	DlssCreateParams.InEnableOutputSubrects = false;


	auto ResultDLSS = NGX_D3D11_CREATE_DLSS_EXT(pContext, &p->m_dlssFeature, p->m_ngxParameters, &DlssCreateParams);
	if (ResultDLSS != NVSDK_NGX_Result_Success) {
		DLSS_TriggerFailureResult((UINT32)ResultDLSS, 1);
		return;
	}

	p->m_bDlssAvailable = 1;
}

void __fastcall DLSS_GetBuffer(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView){
	if (!pNGX_dlss) return;

	ID3D11Texture2D* pResource;
	D3D11_TEXTURE2D_DESC inDesc;

	if (NumViews == 7){
		DLSS_ClearBuffer();
		pDepthStencilView->GetResource((ID3D11Resource**)&pNGX_dlss->DepthBuffer);
		pNGX_dlss->IsReady = 1;
	}
	// end
}

void __fastcall DLSS_ClearBuffer(){

	pNGX_dlss->ColorBuffer = nullptr;
	pNGX_dlss->OutColor = nullptr;
	pNGX_dlss->DepthBuffer = nullptr;

	/*
	if (pNGX_dlss->ColorBuffer) {
		pNGX_dlss->ColorBuffer->Release();
		pNGX_dlss->ColorBuffer = nullptr;
	}

	if (pNGX_dlss->MVBuffer) {
		pNGX_dlss->MVBuffer->Release();
		pNGX_dlss->MVBuffer = nullptr;
	}

	if (pNGX_dlss->DepthBuffer) {
		pNGX_dlss->DepthBuffer->Release();
		pNGX_dlss->DepthBuffer = nullptr;
	}
	*/
	
	pNGX_dlss->IsReady = 0;
}

void __fastcall DLSS_Reset() {
	if (pD3DPostProcess){
		pD3DPostProcess->CurrentTime[0] = -1;
		pD3DPostProcess->CurrentTime[1] = -1;
	}

	if (!pNGX_dlss) return;

	pNGX_dlss->JitterIndex = 1;
	pNGX_dlss->IsReset = 1;
	DLSS_ClearBuffer();
}

float __fastcall DLSS_Halton(int index, int base){
	float result = 0.0f;
	float f = 1.0f / base;
	int i = index;
	while (i > 0) {
		result += f * (i % base);
		i = i / base;
		f = f / base;
	}
	result -= 0.5f;
	return result;
}

void __fastcall DLSS_GetJitter(float* out){
	if (!pNGX_dlss) return;

	if (pNGX_dlss->IsSplitScreen) {
		*(UINT64*)out = 0;
		pNGX_dlss->IsReset = 1;
		return;
	}

	out[0] = DLSS_Halton(pNGX_dlss->JitterIndex, 2);
	out[1] = DLSS_Halton(pNGX_dlss->JitterIndex, 3);
	pNGX_dlss->JitterIndex++;
}

void __fastcall DLSS_Evaluate(){

	float jitter[2];
	DLSS_GetJitter(jitter);
	NVSDK_NGX_D3D11_DLSS_Eval_Params D3D11DlssEvalParams;
	memset(&D3D11DlssEvalParams, 0, sizeof(D3D11DlssEvalParams));

	D3D11DlssEvalParams.Feature.pInColor = pNGX_dlss->ColorBuffer;
	D3D11DlssEvalParams.Feature.pInOutput = pNGX_dlss->OutColor;
	D3D11DlssEvalParams.Feature.InSharpness = 1;

	D3D11DlssEvalParams.pInDepth = pNGX_dlss->DepthBuffer;
	D3D11DlssEvalParams.pInMotionVectors = pD3DPostProcess->BlackMV;

	D3D11DlssEvalParams.InJitterOffsetX = jitter[0];
	D3D11DlssEvalParams.InJitterOffsetY = jitter[1];
	D3D11DlssEvalParams.InRenderSubrectDimensions.Width = pNGX_dlss->resolution[0];
	D3D11DlssEvalParams.InRenderSubrectDimensions.Height = pNGX_dlss->resolution[1];

	D3D11DlssEvalParams.InReset = pNGX_dlss->IsReset;
	D3D11DlssEvalParams.InMVScaleX = 1.0;
	D3D11DlssEvalParams.InMVScaleY = 1.0;

	NVSDK_NGX_Parameter_SetF(pNGX_dlss->m_ngxParameters, NVSDK_NGX_Parameter_Denoise, 1.0);
	auto result = NGX_D3D11_EVALUATE_DLSS_EXT(pD3DPostProcess->Context, pNGX_dlss->m_dlssFeature, pNGX_dlss->m_ngxParameters, &D3D11DlssEvalParams);

	pNGX_dlss->IsReset = 0;

	DLSS_ClearBuffer();
}

void __fastcall DLSS_Reload() {
	if (!pD3DPostProcess) return;
	pD3DPostProcess->LoadComputeShader();
}
