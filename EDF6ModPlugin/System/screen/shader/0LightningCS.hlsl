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

// new
cbuffer lightning_params : register(b1) {
	float3 g_lightning_color;
	float g_lightning_fade;
	float g_lightning_intensity;
	float g_lightning_shadow_darken;
	float g_lightning_sky_amount;
	float2 g_lightning_pad;
};

//===========================================================
Texture2D<float2> g_metallic_roughness : register(t0);
Texture2D<float3> g_normalXYZ : register(t2);
Texture2D<float4> g_viewXYZ_id : register(t4);
Texture2DArray<float> g_shadow_depth : register(t15);

RWTexture2D<float4> g_color_out : register(u0);
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
// 返回 float3：xy = 屏幕 uv，z = 是否可见（1 可见，0 不可见）
float3 WorldPosToScreen() {
	float4 worldPos;
	float3 cameraPos = float3(g_xgl_view_inverse[0].w, g_xgl_view_inverse[1].w, g_xgl_view_inverse[2].w);
	worldPos.xyz = cameraPos - g_xgl_light[0].m_vector * 300.0;
	worldPos.w = 1;

	float4 ClipPos;
	ClipPos.x = dot(worldPos, g_xgl_view_projection[0]);
	ClipPos.y = dot(worldPos, g_xgl_view_projection[1]);
	ClipPos.z = dot(worldPos, g_xgl_view_projection[2]);
	ClipPos.w = dot(worldPos, g_xgl_view_projection[3]);

    //if (ClipPos.w <= 0.0001) return float3(0, 0, 0);

    float3 ndc = ClipPos.xyz / ClipPos.w;
    float2 uv = ndc.xy * float2(0.5, -0.5) + 0.5;

    return float3(uv, 1.0);
}
//===========================================================
float SkyBolt(uint2 pixel, uint2 dim, float time) {
	float2 uv = (float2(pixel) + 0.5) / float2(dim);
	float2 p = uv * 2.0 - 1.0;
	p.x *= float(dim.x) / float(dim.y);

	float boltX = 0.0;
	boltX += sin(p.y * 7.0  + time * 18.0) * 0.06;
	boltX += sin(p.y * 21.0 - time * 33.0) * 0.025;
	boltX += sin(p.y * 53.0 + time * 61.0) * 0.010;

	float d = abs(p.x - boltX);
	float core = smoothstep(0.004, 0.0, d);
	float glow = smoothstep(0.090, 0.0, d) * 0.35;
	float skyMask = saturate(p.y * 0.5 + 0.5);
	return (core + glow) * skyMask;
}
// 天空光团：返回 0~N 的亮度
float SkyFlash(uint2 pixel, uint2 dim, float time) {
    float3 screen = WorldPosToScreen();
    if (screen.z < 0.5) return 0.0;   // 不可见

    float2 center = screen.xy;
    float2 uv = (float2(pixel) + 0.5) / float2(dim);
    float aspect = float(dim.x) / float(dim.y);

    float2 d = uv - center;
    d.x *= aspect;

    // 横向长半径：放大 y，竖直衰减更快
    d.y *= 1.0 / 0.35;

    float r = length(d);

    // 不规则边缘
    float ang = atan2(d.y, d.x);
    float wobble = 1.0
        + 0.10 * sin(ang * 3.0 + time * 2.0)
        + 0.06 * sin(ang * 7.0 - time * 3.5)
        + 0.03 * sin(ang * 13.0 + time * 5.0);
    float rr = r * wobble;

    float core  = smoothstep(0.020, 0.0, rr);
    float inner = smoothstep(0.150, 0.0, rr) * 0.7;
    //float outer = smoothstep(0.550, 0.0, rr) * 0.25;
	float outer1 = smoothstep(0.80, 0.0, rr) * 0.45;
	float outer2 = smoothstep(20, 0.0, rr) * 0.25;

    float flicker = 0.85 + 0.15 * sin(time * 60.0) * sin(time * 37.0);

    float cloud = smoothstep(0.9, 0.0, rr)
                * smoothstep(0.25, 1.0, 1.0 - abs(d.y) * 1.2)
                * 0.15;

    return (core + inner + outer1 + outer2 + cloud) * flicker;
}
//===========================================================
[numthreads(16, 16, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 dim;
	g_color_out.GetDimensions(dim.x, dim.y);
	if (threadID.x >= dim.x || threadID.y >= dim.y) return;

	uint2 pixel = threadID.xy;
	float3 color = g_color_out[pixel].rgb;

	float3 viewPos3 = g_viewXYZ_id[pixel].xyz;
	float4 viewPos  = float4(viewPos3, 1.0);
	bool isSky = (abs(viewPos.z) >= 65500.0);

	float3 lightning_color = g_lightning_color * g_lightning_fade;
	//------------------------------
	// 天空
	//------------------------------
	if (isSky) {
		float bolt = SkyFlash(pixel, dim, g_xgl_time);
		color += color * lightning_color * bolt * g_lightning_sky_amount;
		g_color_out[pixel] = float4(color, 1);
		return;
	}

	//------------------------------
	// 地面
	//------------------------------
	float3 N = g_normalXYZ[pixel];
	float nlen = length(N);
	if (nlen <= 0.001) { return; }
	N /= nlen;

	// 闪电方向 = 主光方向，与主 CS 的 NdotLight 完全一致
	float3 L = g_xgl_light[0].m_vector;
	float NdotL = saturate(dot(-N, L));

	// 主光阴影：0=完全阴影，1=完全照亮
	float mainShadow = CalculateShadow(viewPos);

	float2 mr = g_metallic_roughness[pixel];
	float metallic  = saturate(mr.x);
	float roughness = saturate(max(mr.y, 0.04));

	// 1) 金属没有漫反射：金属度越高，闪电漫反射越少
	float diffuse = NdotL * (1.0 - metallic);

	// 2) 粗糙度调制衰减曲线：
	//    粗糙面过渡更缓（指数 < 1），光滑面更陡（指数 > 1）
	float roughExp = lerp(1.6, 0.6, roughness);   // 光滑→陡，粗糙→缓
	diffuse = pow(diffuse, roughExp);

	// 3) 可选：粗糙面整体略暗，避免 pow 抬亮后过曝
	diffuse *= lerp(1.0, 0.85, roughness);

	float3 lit = g_lightning_color * g_lightning_intensity * diffuse;

	float vis = mainShadow * lerp(0.22, 1.0, diffuse);
	color += color * lit * vis;

	// 阴影压暗照旧
	float darken = lerp(1.0, g_lightning_shadow_darken, saturate(g_lightning_intensity));
	color = lerp(color, color * darken, 1.0 - mainShadow);

	g_color_out[pixel] = float4(color, 1);
}
