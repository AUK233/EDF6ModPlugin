//===========================================================
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

struct LocalEnv {
	float4 m_min_point;
	float4 m_inv_unit_size;
	float4 m_unit_size;
	uint4 m_grid_divide_count;
	uint m_debug_is_used_local_cubemap;
	uint m_debug_preview_env;
	uint m_debug_color_out;
	uint pad;
};

struct LocalEnvGrid {
	uint m_cubemap_index[8];       // Offset:    0
	uint m_coner_index[8];         // Offset:   32
	uint4 m_count;                 // Offset:   64
};

struct PointLight {
	float3 m_position;
	float m_radius;                // Offset:   12
	float3 m_color;                // Offset:   16
	float m_inv_radius;            // Offset:   28
	float3 m_vector;               // Offset:   32
	uint m_type;                   // Offset:   44
	float m_min_z;                 // Offset:   48
	float m_max_z;                 // Offset:   52
	float m_smooth;                // Offset:   56
	float m_curve; // Offset:   60
};

struct PointLightMask {
	uint m_light_mask[64];         // Offset:    0
	bool m_is_anything_passed;     // Offset:  256
};

//===========================================================
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

cbuffer xgl_localcubemap : register(b10) {
	LocalEnv g_envinfo;
};

//===========================================================
SamplerState g_xgl_compute_environment_map_sampler : register(s10);
SamplerState g_ambient_occlusion_sampler : register(s11);

Texture2D<float3> g_albedoRGB : register(t0);
Texture2D<float3> g_specularRGB : register(t1);
Texture2D<float3> g_normalXYZ : register(t2);
Texture2D<float2> g_metallic_roughness : register(t3);
Texture2D<float4> g_viewXYZ_id : register(t4);
Texture2D<float3> g_light_accumulate : register(t5);
Texture2D<float3> g_specular_accumulate : register(t6);
TextureCubeArray<float4> g_local_cubemap_array : register(t8);
Texture2D<float2> g_ambient_occlusion : register(t9);

StructuredBuffer<LocalEnvGrid> g_local_cubemap_grid : register(t10);
StructuredBuffer<PointLight> g_xgl_compute_point_light_array : register(t11);
StructuredBuffer<PointLightMask> g_point_light_mask_x : register(t12);
StructuredBuffer<PointLightMask> g_point_light_mask_y : register(t13);
StructuredBuffer<PointLightMask> g_point_light_mask_z : register(t14);
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

float3 NoAOColor(uint2 pixel) {
	// line 358-361
	float3 albedo = g_albedoRGB[pixel];
	float3 lightAccum = g_light_accumulate[pixel];
	float3 specAccum = g_specular_accumulate[pixel];
	return albedo * lightAccum + specAccum;
}

float3 AddOutline(float3 color, uint2 id, float EdgeSensitivity, float EdgeStrength, float EdgeWidth) {
	float3 originalColor = color;
	
	int radius = max(1, int(EdgeWidth * 2));
	
	float3 centerNormal = g_normalXYZ[id.xy].xyz;
	float2 centerAO = g_ambient_occlusion[id.xy].rg;
	float3 centerViewPos = g_viewXYZ_id[id.xy].xyz;
	
	float minAO = centerAO.r;
	float maxAO = centerAO.r;
	float minDepth = centerAO.g;
	float maxDepth = centerAO.g;
	float maxDepthGradient = 0;
	float maxNormalDiff = 0;
	int validSamples = 0;
	
	[unroll]
	for (int y = -radius; y <= radius; y++) {
		[unroll]
		for (int x = -radius; x <= radius; x++) {
			if (x == 0 && y == 0) continue;
			
			int2 samplePos = id.xy + int2(x, y);
			
			uint width, height;
			g_ambient_occlusion.GetDimensions(width, height);
			if (samplePos.x < 0 || samplePos.x >= (int)width || 
				samplePos.y < 0 || samplePos.y >= (int)height) continue;
			
			float2 sampleAO = g_ambient_occlusion[samplePos].rg;
			float3 sampleNormal = g_normalXYZ[samplePos].xyz;
			float3 sampleViewPos = g_viewXYZ_id[samplePos].xyz;
			
			minAO = min(minAO, sampleAO.r);
			maxAO = max(maxAO, sampleAO.r);
			minDepth = min(minDepth, sampleAO.g);
			maxDepth = max(maxDepth, sampleAO.g);
			
			float depthGradient = distance(centerViewPos, sampleViewPos);
			maxDepthGradient = max(maxDepthGradient, depthGradient);
			
			float normalDiff = 1.0 - dot(centerNormal, sampleNormal);
			maxNormalDiff = max(maxNormalDiff, normalDiff);
			
			validSamples++;
		}
	}
	
	if (validSamples < 4) return originalColor;
	
	float aoRange = maxAO - minAO;
	float depthRange = maxDepth - minDepth;
	float aoChangeRate = aoRange / (depthRange + 0.001);
	
	float edgeConfidence = 0;
	if (aoRange > 0.05 && depthRange > EdgeSensitivity) {
		float rateFactor = 1.0 - abs(aoChangeRate - 1.0) * 0.5;
		rateFactor = saturate(rateFactor);
		float depthFactor = min(0.6f, depthRange / (EdgeSensitivity * 3));
		edgeConfidence = depthFactor * rateFactor;
	}
	
	float normalEdge = 0;
	if (maxNormalDiff > 0.15) {
		normalEdge = saturate((maxNormalDiff - 0.15) / 0.35) * 0.15;
	}
	
	float combinedEdge = max(edgeConfidence, normalEdge);
	float edgeStrength = combinedEdge * EdgeStrength;

	float3 edgeColor;
	if (g_outline_color > 0.01) {
		edgeColor = 1.5;
		edgeStrength *= g_outline_color;
	} else {
		edgeColor = 0;
	}
	originalColor *= 1.25;
	
	return lerp(originalColor, edgeColor, edgeStrength);
}

float AddOutline_VegetationRatio(uint2 id, int radius){
	float vegetationWeight = 0;
	int totalSamples = 0;

	bool selfIsVegetation = true;
	
	for (int y = -radius; y <= radius; y++)
	{
		for (int x = -radius; x <= radius; x++)
		{
			float2 samplePBR = g_metallic_roughness[id.xy + int2(x, y)];
			bool isVegetation = (samplePBR.x < 0.01 && samplePBR.y > 0.99);

			if (x == 0 && y == 0) selfIsVegetation = isVegetation;

			vegetationWeight += isVegetation ? 1 : 0;
			totalSamples++;
		}
	}

	float outV = vegetationWeight / totalSamples;
	if (!selfIsVegetation) outV *= 0.5;

	return outV;
}

float3 AddBlackBorder(float3 color, uint2 id, float EdgeSensitivity, float EdgeStrength, float EdgeWidth) {
	int radius = max(1, int(EdgeWidth * 2));

	// float vegetationRatio = AddOutline_VegetationRatio(id, radius);
	// if (vegetationRatio > 0.2) return color;

	float3 spmRGB = g_specularRGB[id.xy];
	float spmDot = dot(spmRGB, spmRGB);
	if (spmDot < 0.0001) return color;

	float3 originalColor = color;

	float minDepth = 1e10;
	float maxDepth = -1e10;
	
	
	for (int y = -radius; y <= radius; y++)
	{
		for (int x = -radius; x <= radius; x++)
		{
			if (x == 0 && y == 0) continue;
			
			float depth = g_ambient_occlusion[id.xy + int2(x, y)].g;
			minDepth = min(minDepth, depth);
			maxDepth = max(maxDepth, depth);
		}
	}
	
	float centerDepth = g_ambient_occlusion[id.xy].g;
	float depthRange = maxDepth - minDepth;
	
	bool nearMin = abs(centerDepth - minDepth) < depthRange * 0.2;
	bool nearMax = abs(centerDepth - maxDepth) < depthRange * 0.2;
	
	float edgeStrength = 0;
	if ((nearMin || nearMax) && depthRange > EdgeSensitivity)
	{
		edgeStrength = min(0.6f, depthRange / (EdgeSensitivity * 3));
		edgeStrength = edgeStrength * EdgeStrength;
	}
	
	float3 edgeColor;
	if (g_outline_color > 0.01) {
		edgeColor = 1;
		edgeStrength *= g_outline_color;
	} else {
		edgeColor = 0;
		//originalColor *= 1.125;
	}
	
	return lerp(originalColor, edgeColor, edgeStrength);
}

//===========================================================
// entry point
//===========================================================
[numthreads(16, 16, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID) {
	// line 0-5
	uint2 texDim;
	g_albedoRGB.GetDimensions(texDim.x, texDim.y);
	if (threadID.x >= texDim.x || threadID.y >= texDim.y) {
		return;
	}
	
	uint2 pixel = threadID.xy;
	
	// line 8, 16
	float4 viewPos = float4(g_viewXYZ_id[pixel].xyz, 1);
	
	// line 9-15
	if (abs(viewPos.z) >= 65534.0) {
		g_color_out[pixel] = float4(g_bg_color.xyz, 0.0);
		return;
	}
	
	// line 17-19
	float3 worldPos;
	worldPos.x = dot(viewPos, g_xgl_view_inverse[0]);
	worldPos.y = dot(viewPos, g_xgl_view_inverse[1]);
	worldPos.z = dot(viewPos, g_xgl_view_inverse[2]);

	// line 20-25
	float3 cameraPos = float3(g_xgl_view_inverse[0].w, g_xgl_view_inverse[1].w, g_xgl_view_inverse[2].w);
	float3 viewDir = worldPos - cameraPos;
	float distance = length(viewDir);

	// line 25-33
	float fade = distance * g_xgl_fade_mul + g_xgl_fade_add;
	if (fade >= 1.0) {
		g_color_out[pixel] = float4(g_bg_color.xyz, 0.0);
		return;
	}

	// line 34-44
	float3 normal = g_normalXYZ[pixel];
	float normalLen = length(normal);
	
	float2 fog;
	fog.x = saturate(distance * g_xgl_fog_mul + g_xgl_fog_add);
	fog.y = saturate(distance * g_xgl_brightness_fog_mul + g_xgl_brightness_fog_add);
	fog = pow(fog, float2(g_xgl_fog_curve, g_xgl_brightness_fog_curve));

	float fogWeight = fog.x * g_xgl_fog_color.w;
	float fogBrightWeight = fog.y * g_xgl_brightness_fog_color.w;

	// line 45-47, r7.w
	float fadeInv = 1.0 - saturate(fade);
	
	float3 color;
	
	// line 47-48
	if (normalLen > 0.001) {
		// line 49-53
		normal /= normalLen;
		viewDir /= distance;
		float3 reflectDir = reflect(viewDir, normal);
		

		// line 54-62
		// uint2 aoDim;
		// g_ambient_occlusion.GetDimensions(aoDim.x, aoDim.y);
		// float2 aoUV = float2(pixel) / float2(aoDim);
		// float ao = g_ambient_occlusion.SampleLevel(g_ambient_occlusion_sampler, aoUV, 0).x;
		float ao = g_ambient_occlusion[pixel].r;

		float aoStrength = ao * normalLen;
		float3 ambientColor = aoStrength * g_xgl_ambient_color;
		
		// line 63-65
		float3 specularColor = g_specularRGB[pixel];
		float2 metallicRoughness = g_metallic_roughness[pixel];
		float metallic = saturate(metallicRoughness.x);
		float roughness = saturate(metallicRoughness.y);

		// new block
#ifdef _AddMetallic
		if (normalLen > 1.0) {
			float normalRoughnessFactor = saturate((normalLen - 1.0) * 0.5);
			roughness = lerp(roughness, 1.0, normalRoughnessFactor);
		}
		aoStrength = lerp(1.0, aoStrength, roughness);
		ambientColor = aoStrength * g_xgl_ambient_color;
		metallic = metallic * metallic * (3 - 2 * metallic); // smoothstep
#endif

		// line 66-68
		float roughnessClamped = max(roughness, 0.04);
		// Fresnel Reflectance
		float F0 = metallic * 0.96 + 0.04;
		float3 specular_color = F0 * specularColor;
		
		// line 69-114
		float shadow = CalculateShadow(viewPos);
		
		// line 115: r2.y, -r5.xyzx, r4.xyzx
		float VdotN = saturate(dot(-normal, viewDir));
		
		// line 116-122
		float roughness4 = pow(roughnessClamped, 4);
		float roughness2 = roughness4 - 1;
		
		float specBias = roughness4 * 0.5 + 0.000001; // r2.w
		float diffBias = 1.0 - specBias; // r10.w
		// Smith-GGX
		float G1 = VdotN / (VdotN * diffBias + specBias); // r11.w
		
		// line 123-127
		float3 specular_inv_color = 1.0 - specular_color; // r9.xyz
		// shadow as r9.w
		float3 diffuseAccum = ambientColor;
		bool subLight = false; // r12.w = 0
		float3 specularAccum = 0; // r13.xyz
		
		// line 128-130
		for (uint i = 0; i < g_light_count; i++) {
			// line 131-132
			if (shadow > 0.0) {
				// line 133-135
				float3 lightDir = g_xgl_light[i].m_vector;
				float NdotLight = saturate(dot(-normal, lightDir));
				float lightFactor = NdotLight * 3.14159265;
				// line 136-142
				//float3 halfDir = normalize(viewDir + lightDir);
				float3 halfDir = viewDir + lightDir;
				float halfDirLen = length(halfDir);
				if (halfDirLen > 0.000001) {
					halfDir = halfDir / halfDirLen;
				}
				// line 143-148, GGX's D term
				float NdotHalf = saturate(dot(-normal, halfDir));
				NdotHalf = NdotHalf * NdotHalf * roughness2 + 1;
				NdotHalf = roughness4 / (NdotHalf * NdotHalf * 3.14159265);
				// line 149-151, Smith-GGX's G term
				float G = NdotLight / (NdotLight * diffBias + specBias);
				G *= G1;
				// line 152-156, Schlick
				float VdotH = 1 - saturate(dot(halfDir, viewDir));
				// float pow5 = VdotH * VdotH;
				// pow5 = pow5 * pow5;
				// line 156-157, Schlick Fresnel
				float3 SF = pow(VdotH, 5) * specular_inv_color + specular_color;
				// line 158-165
				float D_GS = NdotHalf * G;
				float specBRDF = D_GS / (VdotN * NdotLight * 4.0 + 0.000001);
				float3 specular = SF * specBRDF * lightFactor;
				
				if (subLight) {
					specular = min(specular, g_xgl_sub_light_specular_factor_max);
				}
				
				// line 166-169
				diffuseAccum += g_xgl_light[i].m_color * NdotLight * shadow;
				specularAccum += g_xgl_light[i].m_specular_color * specular * shadow;
			}
			// line 172-173
			shadow = 1;
			subLight = true;
		}
		
		// line 175-178
		float nonMetallic = 1.0 - metallic;
		float3 diffuse_accum = diffuseAccum * normalLen * nonMetallic; // r2.xyz
		float3 specular_accum = specularAccum * normalLen; // r4.xyz
		
		// line 179-184
		float4 worldPosH = float4(worldPos, 1.0);
		float lightDepth = dot(worldPosH, g_light_view_proj[2]) / dot(worldPosH, g_light_view_proj[3]);
		
		float2 texUV = float2(pixel) / float2(texDim);
		float3 f_lightUVW = float3(texUV, lightDepth) * 128;
		uint3 lightUVW = uint3(f_lightUVW);
		
		// line 185-188
		float nonRoughness = 1.0 - metallicRoughness.y;
		float2 nonRoughness2 = float2(0.2, 200) * nonRoughness; // r9.xy
		float2 specRoughness = nonRoughness < 0 ? float2(0, 0.000001) : float2(nonRoughness2.y, nonRoughness); // r6.yz
		
		// line 189-193
		bool maskX = g_point_light_mask_x[lightUVW.x].m_is_anything_passed;
		bool maskY = g_point_light_mask_y[lightUVW.y].m_is_anything_passed;
		bool maskZ = g_point_light_mask_z[lightUVW.z].m_is_anything_passed;
		// line 194
		if (maskX && maskY && maskZ) {
			// line 195-198
			uint numGroups = (g_point_light_count + 31) / 32;
			float3 pointDiffuse = diffuse_accum; // r10.yzw
			float3 pointSpecular = 0; // r11.xyz
			// line 200
			for (uint group = 0; group < numGroups; group++) {
				uint lightOffset = group * 32; // r9.w
				// line 204-208
				uint g_maskX = g_point_light_mask_x[lightUVW.x].m_light_mask[group];
				uint g_maskY = g_point_light_mask_y[lightUVW.y].m_light_mask[group];
				uint g_maskZ = g_point_light_mask_z[lightUVW.z].m_light_mask[group];
				uint g_mask = g_maskX & g_maskY & g_maskZ;
				// line 209-213
				uint lightIndex = firstbitlow(g_mask);
				float3 pointDiffAccum = pointDiffuse; // r12.xyz
				float3 pointSpecAccum = pointSpecular; // r13.xyz
				uint tempMask = g_mask; // r9.y
				uint currentLight = lightIndex; // r11.w
				// line 214-216
				while (currentLight < 32) {
					// line 217-224
					tempMask &= ~(1u << currentLight);
					uint globalIndex = lightOffset + currentLight;
					PointLight light = g_xgl_compute_point_light_array[globalIndex];
					float3 toLight = worldPos - light.m_position;
					float dist = length(toLight);
					// line 225
					[branch]
					if (dist < light.m_radius) {
						//line 227-228
						float attenZ = light.m_smooth;
						// line 229
						toLight /= dist;
						// line 230-231
						float spotAtten = attenZ - (dot(toLight, normal) * (1 - attenZ));
						// line 232
						[branch]
						if (spotAtten > 0.0) {
							// line 234-239
							float atten = 1.0 - dist * light.m_inv_radius;
							float light_curve = light.m_curve;
							float finalAtten = pow(atten, light_curve);
							// line 240-242
							finalAtten *= saturate(spotAtten);
							pointDiffAccum += light.m_color * finalAtten;
							
							// line 243-251, Phong
							float RdotL = dot(toLight, reflectDir);
							float specBase = max(-RdotL, 0.0001);
							float specIntensity = pow(pow(atten, 0.1 * light_curve) * specBase, specRoughness.x);
							pointSpecAccum += specIntensity * light.m_color;
						}
						// end
					}
					// end
				
					// line 254
					currentLight = firstbitlow(tempMask);
				}
				
				// line 256-257
				pointDiffuse = pointDiffAccum;
				pointSpecular = pointSpecAccum;
			}
			// line 260-261
			diffuse_accum = pointDiffuse;
			specular_accum += pointSpecular * specRoughness.y;
		}
		
		// line 263-266
		float3 albedo = g_albedoRGB[pixel];
		float3 lightAccum = g_light_accumulate[pixel];
		float3 finalDiffuse = diffuse_accum + lightAccum;
		finalDiffuse *= albedo;
		
		// line 267-270
		uint c_cube3[3], cubemapSize;
		g_local_cubemap_array.GetDimensions(0, c_cube3[0], c_cube3[1], c_cube3[2], cubemapSize);
		
		float roughnessFactor = max(-nonRoughness * nonRoughness + 1, 0);
		float mipLevel = cubemapSize * roughnessFactor;
		/*
		float roughnessFactor = roughness * roughness;
		float mipLevel = cubemapSize * (0.5 * log2(roughnessFactor + 0.0001));
		mipLevel = clamp(mipLevel, 0, cubemapSize);*/
		
		// line 271-278
		float3 gridPos = worldPos - g_envinfo.m_min_point.xyz;
		float3 gridCoord = gridPos * g_envinfo.m_inv_unit_size.xyz;
		uint3 gridIndex = uint3(gridCoord);
		uint3 maxGridIndex = g_envinfo.m_grid_divide_count.xyz - 1;
		gridIndex = min(gridIndex, maxGridIndex);

		uint g_tempY = gridIndex.z * g_envinfo.m_grid_divide_count.y;
		uint g_tempZ = gridIndex.y * g_envinfo.m_grid_divide_count.x;
		uint gridIdx = g_tempY * g_envinfo.m_grid_divide_count.x + g_tempZ;
		gridIdx += gridIndex.x;
		// line 279-283
		LocalEnvGrid envGrid = g_local_cubemap_grid[gridIdx];
		
		// line 284
		float3 envColor; // r6.yzw
		uint envCount = envGrid.m_count.x;
		if (envCount == 1) {
			// line 285-288
			uint cubemapIndex = envGrid.m_cubemap_index[0];
			envColor = g_local_cubemap_array.SampleLevel(
				g_xgl_compute_environment_map_sampler,
				float4(reflectDir, cubemapIndex),
				mipLevel).rgb;
		}
		else {
			// line 289-293
			float3 gridOffset = -float3(gridIndex) * g_envinfo.m_unit_size.xyz + gridPos;
			float3 gridFrac = gridOffset * g_envinfo.m_inv_unit_size.xyz;
			float3 gridInvFrac = 1 - gridFrac;
			// line 294-302
			float gr4w = gridInvFrac.x * gridInvFrac.y;
			float gr9y = saturate(gr4w * gridInvFrac.z);
			float2 gr3xy = gridInvFrac.yx * gridFrac.xy;
			float2 gr9zw = saturate(gr3xy * gridInvFrac.z);
			float gr5x = gridFrac.x * gridFrac.y;
			float gr3z = saturate(gr5x * gridInvFrac.z);
			gr4w = saturate(gr4w * gridFrac.z);
			gr3xy = saturate(gr3xy * gridFrac.z);
			gr5x = saturate(gr5x * gridFrac.z);
			
			// line 303
			float3 envColors[8];
			for (uint j = 0; j < envCount; j++) {
				// line 307-320
				uint cubeIndex = envGrid.m_cubemap_index[j];
				envColors[j] = g_local_cubemap_array.SampleLevel(
								g_xgl_compute_environment_map_sampler,
								float4(reflectDir, cubeIndex),
								mipLevel).rgb;
			}
			// line 323-346
			/*
			float3 weightedColor = envColors[envGrid.m_coner_index[1]] * gr9zw.x;
			float totalWeight = gr9zw.x;
			weightedColor += envColors[envGrid.m_coner_index[0]] * gr9y;
			totalWeight += gr9y;
			weightedColor += envColors[envGrid.m_coner_index[2]] * gr9zw.y;
			totalWeight += gr9zw.y;
			weightedColor += envColors[envGrid.m_coner_index[3]] * gr3z;
			totalWeight += gr3z;
			
			weightedColor += envColors[envGrid.m_coner_index[4]] * gr4w;
			totalWeight += gr4w;
			weightedColor += envColors[envGrid.m_coner_index[5]] * gr3xy.x;
			totalWeight += gr3xy.x;
			weightedColor += envColors[envGrid.m_coner_index[6]] * gr3xy.y;
			totalWeight += gr3xy.y;
			weightedColor += envColors[envGrid.m_coner_index[7]] * gr5x;
			totalWeight += gr5x;*/
			
			float weight[8];
			weight[0] = gr9y;
			weight[1] = gr9zw.x;
			weight[2] = gr9zw.y;
			weight[3] = gr3z;
			weight[4] = gr4w;
			weight[5] = gr3xy.x;
			weight[6] = gr3xy.y;
			weight[7] = gr5x;
			float3 weightedColor = 0;
			float totalWeight = 0;
			for (uint weightIdx = 0; weightIdx < 8; weightIdx++) {
				weightedColor += envColors[envGrid.m_coner_index[weightIdx]] * weight[weightIdx];
				totalWeight += weight[weightIdx];
			}
			
			
			envColor = weightedColor / totalWeight;
		}
		
		// line 348-349
		
		float metallic2 = metallicRoughness.x * metallicRoughness.x;
		float specMetal = metallic2 * -nonRoughness2.x + nonRoughness2.x;
		/*
		float metallic2 = metallic * metallic;
		float specWeight = nonRoughness * nonRoughness * 0.5 + 0.5;
		float specMetal = metallic2 * specWeight;*/
		
		// line 350-356
		float3 finalColor = albedo * aoStrength * envColor - finalDiffuse;
		finalColor = finalColor * metallic2 + finalDiffuse;
		finalColor += envColor * specMetal;

#ifdef _AddMetallic
		//specular_accum *= specRoughness.x * 0.075;
		specular_accum *= envColor * specRoughness.x * 0.125;
#endif

		float3 specAccum = g_specular_accumulate[pixel] + specular_accum;
		color = finalColor + specAccum;

#ifdef _AddBlackBorder
		color = AddBlackBorder(color, pixel, 0.75, 2, 0.5);
#endif
	} else {
		// line 358-361
		color = NoAOColor(pixel);
	}

	// line 363-367
	color = max(color, fogBrightWeight * g_xgl_brightness_fog_color.xyz);
	color = lerp(color, g_xgl_fog_color.xyz, fogWeight);
	g_color_out[pixel] = float4(color, fadeInv);
}
