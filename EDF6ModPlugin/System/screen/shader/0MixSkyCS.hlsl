// ===================================================
struct Light {
	float3 m_vector;
	float m_pad0;
	float3 m_color;
	float m_pad1;
	float3 m_specular_color;
	float m_pad2;
};

struct ShadowSetting {
	row_major float4x4 m_vp_to_view; // 0-3
	row_major float4x4 m_proj; // 4-7
	float m_distance;
	float m_shadow_adjust;
	float m_shadow_smooth;
	float m_pad;
};

cbuffer xgl_system : register(b0) {
	row_major float4x4 g_xgl_view;
	row_major float4x4 g_xgl_view_inverse;
	row_major float4x4 g_xgl_projection;
	row_major float4x4 g_xgl_view_projection;
	row_major float4x4 g_xgl_view_projection_inverse;
	uint2 g_xgl_target_dimension;
	float g_xgl_time;
	int g_xgl_id;
	Light g_xgl_light[4];
	float3 g_xgl_ambient_color;
	float g_xgl_sub_light_specular_factor_max;
	float g_xgl_fog_mul;
	float g_xgl_fog_add;
	float g_xgl_fog_curve;
	float g_xgl_brightness_fog_mul;
	float g_xgl_brightness_fog_add;
	float g_xgl_brightness_fog_curve;
	float g_xgl_fade_mul;
	float g_xgl_fade_add;
	float4 g_xgl_fog_color;
	float4 g_xgl_brightness_fog_color;
};

cbuffer xgl_additional_light_setting : register(b2) {
	ShadowSetting g_shadow_setting[2];
	float g_shadow_far_fade;
	float g_shadow_far_fade_mul;
	float g_shadow_intensity;
	float g_outline_color;
	uint g_light_count;
	uint g_point_light_count;
	float2 g_pad;
	float4 g_bg_color;
	row_major float4x4 g_light_view_proj;
};
//===========================================================
Texture2D<float4> g_color : register(t0);
Texture2D<float3> g_normalXYZ : register(t2);
Texture2D<float4> g_viewXYZ_id : register(t4);
Texture2DArray<float> g_shadow_depth : register(t15);

RWTexture2D<float4> g_dest : register(u0);
//===========================================================
float CalculateShadow(float4 viewPos) {
	if (g_shadow_intensity < 0.0001) {
		return 1.0;
	}

	if (abs(viewPos.z) >= g_shadow_setting[1].m_distance) {
		// line 113
		return 1.0;
	}

	// line 71-72
	int shadowIndex = abs(viewPos.z) < g_shadow_setting[0].m_distance ? 0 : 1;

	// line 73-78
	float4 shadowPosView;
	shadowPosView.x = dot(viewPos, g_shadow_setting[shadowIndex].m_vp_to_view[0]);
	shadowPosView.y = dot(viewPos, g_shadow_setting[shadowIndex].m_vp_to_view[1]);

	float shadowViewZ = dot(viewPos, g_shadow_setting[shadowIndex].m_vp_to_view[2]);
	shadowPosView.z = shadowViewZ + g_shadow_setting[shadowIndex].m_shadow_adjust;
	shadowPosView.w = 1;

	// line 79-81
	float3 shadowPosProj;
	shadowPosProj.x = dot(shadowPosView, g_shadow_setting[shadowIndex].m_proj[0]);
	shadowPosProj.y = dot(shadowPosView, g_shadow_setting[shadowIndex].m_proj[1]);
	shadowPosProj.z = dot(shadowPosView, g_shadow_setting[shadowIndex].m_proj[2]);

	// line 82-84
	float4 shadowPosViewDepthLower = shadowPosView;
	shadowPosViewDepthLower.z = shadowViewZ - g_shadow_setting[shadowIndex].m_shadow_smooth;
	float receiverDepth = dot(shadowPosViewDepthLower, g_shadow_setting[shadowIndex].m_proj[2]) - shadowPosProj.z;

	// line 85-91
	uint2 shadowDim;
	uint shadowArg3;
	g_shadow_depth.GetDimensions(shadowDim.x, shadowDim.y, shadowArg3);

	float2 shadowUV = shadowPosProj.xy * float2(0.5, -0.5) + 0.5;
	shadowUV *= shadowDim;
	float2 fracUV = frac(shadowUV);
	int4 shadowIndex4;
	shadowIndex4.xy = int2(shadowUV);
	shadowIndex4.z = shadowIndex;
	shadowIndex4.w = 0;

	// line 92-100
	float4 depths;
	depths.x = g_shadow_depth.Load(shadowIndex4);
	depths.y = g_shadow_depth.Load(shadowIndex4 + int4(1,0,0,0));
	depths.z = g_shadow_depth.Load(shadowIndex4 + int4(0,1,0,0));
	depths.w = g_shadow_depth.Load(shadowIndex4 + int4(1,1,0,0));

	// line 101-103
	float4 diffDepth = depths - shadowPosProj.z;
	diffDepth /= receiverDepth;
	diffDepth = saturate(diffDepth + 1);
	// line 104-111
	float2 diff2 = diffDepth.yw - diffDepth.xz;
	diff2 = diff2 * fracUV.x + diffDepth.xz;
	diff2.y -= diff2.x;
	diff2.x = diff2.y * fracUV.y + diff2.x;
	diff2.y = abs(viewPos.z) - g_shadow_far_fade;
	diff2.y = saturate(diff2.y * g_shadow_far_fade_mul);

	float shadow = diff2.y * (1 - diff2.x) + diff2.x;
	shadow = lerp(1, shadow, g_shadow_intensity);
	return max(shadow, 0);
}
//===========================================================
static const int   VOL_STEPS          = 12;     // 8~16 性价比最高
static const float VOL_DENSITY        = 0.004;  // 散射系数/米, 0.001~0.02
static const float VOL_ANISOTROPY     = 0.5;   // HG g, 越大越集中向太阳
static const float VOL_INTENSITY      = 1.0;
static const float VOL_MAX_DISTANCE   = 240.0;  // 最大积分距离(米)
static const float VOL_HEIGHT_FALLOFF = 0.0125;  // 高度雾衰减(1/米), 0=关闭
static const float VOL_MIN_COS        = -0.2;   // 视线与太阳夹角余弦下限, 用于跳过
static const float VOL_PHASE_MAX      = 2;   // 相函数上限, 防朝阳过曝

float VolumetricDither(uint2 pixel) {
	// Interleaved Gradient Noise: 静态抖动, 打散步进条带
	return frac(52.9829189 * frac(dot(float2(pixel) + 0.5, float2(0.06711056, 0.00583715))));
}

float3 ComputeVolumetricLight(float3 viewPos, float3 worldPos, float3 cameraPos, uint2 pixel, bool isSky) {
	if (g_light_count == 0) return 0;
	if (g_shadow_intensity < 0.0001) return 0;

	float3 lightColor = g_xgl_light[0].m_color;
	if (dot(lightColor, lightColor) < 1e-6) return 0;

	float rayLen = length(viewPos);
	if (rayLen < 1.0) return 0;

	// 天空：积分到 VOL_MAX_DISTANCE，不被 rayLen / shadow 距离截断
	float maxDist = isSky
		? VOL_MAX_DISTANCE
		: min(rayLen, min(VOL_MAX_DISTANCE, g_shadow_setting[1].m_distance));
	if (maxDist < 1.0) return 0;

	float3 rayDir = viewPos / rayLen;

	// 世界 -> 视空间
	float3 toLightWorld = -g_xgl_light[0].m_vector;
	float3 toLightView;
	toLightView.x = dot(toLightWorld, g_xgl_view[0].xyz);
	toLightView.y = dot(toLightWorld, g_xgl_view[1].xyz);
	toLightView.z = dot(toLightWorld, g_xgl_view[2].xyz);
	toLightView = normalize(toLightView + 1e-6);

#ifdef _RealPhysics
	// Henyey-Greenstein 前向散射
	float cosTheta = dot(rayDir, toLightView);
	//if (cosTheta < VOL_MIN_COS) cosTheta = VOL_MIN_COS;

	float g = VOL_ANISOTROPY;
	// float denom = max(1.0 + g * g - 2.0 * g * cosTheta, 1e-3);
	// float phase = min((1.0 - g * g) / (12.5663706144 * pow(denom, 1.5)), VOL_PHASE_MAX);
	float denomF = max(1.0 + g*g - 2.0*g*cosTheta, 1e-3);
	float phaseF = (1.0 - g*g) / (12.5663706144 * pow(denomF, 1.5)); // 前向
	phaseF = phaseF / (1.0 + phaseF); 
	// 各向同性分量（背太阳也有的环境散射）
	float phaseIso = 1.0 / 12.5663706144; // 1/(4π)
	float blendFactor = saturate(cosTheta * 0.5 + 0.5);
	//blendFactor = blendFactor * blendFactor; 
	// 混合：朝太阳以前向为主，背太阳以各向同性为主
	float phase = lerp(phaseIso, phaseF, blendFactor);
	phase = min(phase, VOL_PHASE_MAX);
#else
	// 1. 基础散射（保证不管朝哪看，空气本身都被照亮）
	// 这个值决定了背对太阳时的底光
	float basePhase = 0.1; 

	// 2. 太阳高度对穿透力的影响
	// 假设 toLightView 是光照方向
	float sunHeight = toLightView.y; 
	// 太阳越高，光穿透力越强（正午）；越低，光被大地遮挡越多（黄昏）
	float sunHeightFactor = saturate(sunHeight + 0.5); 

	// 3. 视线与太阳的夹角（仅做微弱增强，不再主导）
	// 即便直视太阳，也不会像之前那样爆炸
	float cosTheta = dot(rayDir, toLightView);
	float slightForward = saturate(cosTheta * 0.5 + 0.5);

	// 4. 组装最终的 Phase 值
	// 这里我们让基础散射占主导，直视太阳只给一点点加成
	float phase = basePhase + (slightForward * 0.2) * sunHeightFactor;
	// 压制一下上限，保证不过曝
	phase = min(phase, VOL_PHASE_MAX);
#endif
	
	// ray march
	float stepLen = maxDist / VOL_STEPS;
	float offset  = VolumetricDither(pixel);
	float accum   = 0.0;

	[loop]
	for (int i = 0; i < VOL_STEPS; i++) {
		float dist = (i + offset) * stepLen;
		float frac = dist / rayLen;
		float3 pView = viewPos * frac; // 视空间射线上的点
		float shadow = CalculateShadow(float4(pView, 1.0));
		//shadow = max(shadow, 0.2);

		float density = 1.0;
		if (VOL_HEIGHT_FALLOFF > 0.0) {
			float3 pWorld = lerp(cameraPos, worldPos, frac);
			if (isSky) {
				// 天空的 worldPos 在远平面，y 极高，会 exp(-huge)≈0
				// 限制高度，让前段地形遮挡的密度有意义
				pWorld.y = min(pWorld.y, cameraPos.y + 50.0);
			}
			density = exp(-max(pWorld.y - cameraPos.y, 0.0) * VOL_HEIGHT_FALLOFF);
		}
		accum += shadow * density;
	}

	float scattered = accum * stepLen * VOL_DENSITY * VOL_INTENSITY;
	return lightColor * (scattered * phase);
}
//===========================================================
[numthreads(16, 16, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 pixel = threadID.xy;

	uint w, h;
	g_color.GetDimensions(w, h);
	if (pixel.x >= w || pixel.y >= h) return;

	float4 color = g_dest[pixel];
	float3 sky_color = g_color[pixel].xyz * (1.0f - color.w);
	float3 blended_color = color.xyz * color.w + sky_color;

#ifndef _NoVolumetricLight
	float4 viewPos = float4(g_viewXYZ_id[pixel].xyz, 1);
	bool isSky = (abs(viewPos.z) >= 65500.0);
	
	float3 worldPos;
	worldPos.x = dot(viewPos, g_xgl_view_inverse[0]);
	worldPos.y = dot(viewPos, g_xgl_view_inverse[1]);
	worldPos.z = dot(viewPos, g_xgl_view_inverse[2]);

	float3 cameraPos = float3(g_xgl_view_inverse[0].w, g_xgl_view_inverse[1].w, g_xgl_view_inverse[2].w);
	float3 viewDir = worldPos - cameraPos;
	float distance = length(viewDir);
	float fade = distance * g_xgl_fade_mul + g_xgl_fade_add;
	float2 fog;
	fog.x = saturate(distance * g_xgl_fog_mul + g_xgl_fog_add);
	fog.y = saturate(distance * g_xgl_brightness_fog_mul + g_xgl_brightness_fog_add);
	fog = pow(fog, float2(g_xgl_fog_curve, g_xgl_brightness_fog_curve));

	float fogWeight = fog.x * g_xgl_fog_color.w;
	float fadeInv = 1.0 - saturate(fade);

	float3 volColor = ComputeVolumetricLight(viewPos.xyz, worldPos, cameraPos, pixel, isSky);


	// float geoAtten = fadeInv * (1.0 - fogWeight);
	// float skyAtten = (1.0 - fogWeight);
	// float volAtten = lerp(skyAtten, geoAtten, color.w); // w=1 几何体, w=0 天空
	//float volAtten = isSky ? (1.0 - fogWeight) : fadeInv * (1.0 - fogWeight);

	float volAtten = (1.0 - fogWeight); 
	volColor *= volAtten;

	float3 base_color = blended_color;
	if (!isSky && abs(viewPos.z) < 1001.50525){
		blended_color *= 0.75;
	}
	
	blended_color += volColor;

	float3 normal = g_normalXYZ[pixel].xyz;
	if (!isSky) {
		float normalLen = length(normal);
		if(normalLen > 0.01){
			// 这不是天空，是实体表面
			// 计算表面朝向太阳的程度（0~1）
			float NdotL = saturate(dot(-(normal / normalLen), g_xgl_light[0].m_vector));
			
			// 如果表面朝向太阳，给它增加额外的亮度和光晕
			if (NdotL > 0.0) {
				float3 vol_color = saturate(base_color * 0.25) * volColor * 100;
				vol_color = vol_color / (1 + vol_color * 2);
				blended_color += vol_color * NdotL;
			}
		}

		blended_color = lerp(blended_color, g_xgl_fog_color.xyz, fogWeight);
	}

#endif

	g_dest[pixel] = float4(blended_color, 1.0f);
}
