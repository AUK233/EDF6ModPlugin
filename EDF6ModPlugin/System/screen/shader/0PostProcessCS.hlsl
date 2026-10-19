// ===================================================
// Constant!
static const float Saturation = 1.2;
static const float Contrast = 1.25;
static const float2 m_near_far_plane = float2(0.1, 1000); // edf6 is float2(0.1, 1001.50525)
// ===================================================
// in c++
SamplerState SamplerLinear : register(s11);
Texture3D LookupTexture : register(t9);

Texture2D<float4> InputColor : register(t0);
Texture2D<float> DepthMap : register(t1);

RWTexture2D<float4> OutputColor : register(u0);
RWTexture2D<float> OutputDepth : register(u1);
// ===================================================
float LinearizeDepth(float depth)
{
    return m_near_far_plane.x * m_near_far_plane.y / (m_near_far_plane.y - depth * (m_near_far_plane.y - m_near_far_plane.x));
}
// ===================================================
float3 RGBToHSV(float3 c)
{
	float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	float4 p = c.g < c.b ? float4(c.bg, K.wz) : float4(c.gb, K.xy);
	float4 q = c.r < p.x ? float4(p.xyw, c.r) : float4(c.r, p.yzx);
	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	float3 hsv = float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
	return hsv;
}

float3 HSVToRGB(float3 c)
{
	float3 rgb = clamp(abs(frac(c.x + float3(0.0, 2.0, 1.0) / 3.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0);
	return c.z * lerp(float3(1,1,1), rgb, c.y);
}

float4 ApplyAFPFilter(float4 color)
{
	float3 hsvColor = RGBToHSV(color.rgb);
	hsvColor.y *= Saturation;
	float3 saturatedColor = HSVToRGB(hsvColor);
	
	float3 contrastedColor = (saturatedColor - 0.5) * Contrast + 0.5;
	
	float lum = dot(contrastedColor, float3(0.2126, 0.7152, 0.0722));
	float balanceFactor = 0.9 + lum * 0.2; // 0.9 - 1.1
	balanceFactor = clamp(balanceFactor, 0.85, 1.15);
	
	float3 finalColor = contrastedColor * balanceFactor;
	
	return float4(finalColor, color.a);
}

// ===================================================

float3 AdjustBrightness_PS(float3 rgb, float brightness)
{
    float t = brightness / 255.0;
    
    float3 result = rgb + t;
    return saturate(result);
}

float3 AdjustLightness_PS_HueSat(float3 rgb, float lightness)
{
    float luminance = dot(rgb, float3(0.299, 0.587, 0.114));
    
    float t = lightness / 100.0;
    
    if (lightness >= 0.0)
    {
        return lerp(rgb, float3(1.0, 1.0, 1.0), t);
    }
    else
    {
        return lerp(rgb, float3(0.0, 0.0, 0.0), -t);
    }
}

float3 AdjustSaturation_PS(float3 rgb, float saturation)
{
    float luminance = dot(rgb, float3(0.299, 0.587, 0.114));
    float t = saturation / 100.0;
    
    return lerp(luminance.xxx, rgb, 1.0 + t);
}

float3 AdjustContrast_PS(float3 rgb, float contrast)
{
    float t = (contrast + 100.0) / 100.0;
    
    float3 result = (rgb - 0.5) * t + 0.5;
    return saturate(result);
}

float4 ApplyNewFilter(float4 color)
{
	// use ps style
	float3 afterHueSat = color.rgb;
	afterHueSat = AdjustLightness_PS_HueSat(afterHueSat, 1.25);
    afterHueSat = AdjustSaturation_PS(afterHueSat, 50); 

	float3 finalColor = AdjustContrast_PS(afterHueSat, 12.5); // max is 100

	// afterHueSat = AdjustSaturation_PS(afterHueSat, -25); 
	// float3 finalColor = AdjustContrast_PS(afterHueSat, 50); // max is 100
	
	return float4(finalColor, color.a);
}

// ===================================================

float4 AddBlackBorderOld(float4 color, uint3 id, float DepthThreshold, float EdgeIntensity)
{
	float centerDepth = LinearizeDepth(DepthMap[id.xy]);
    float depthLeft = LinearizeDepth(DepthMap[id.xy + int2(-1, 0)]);
    float depthRight = LinearizeDepth(DepthMap[id.xy + int2(1, 0)]);
    float depthUp = LinearizeDepth(DepthMap[id.xy + int2(0, -1)]);
    float depthDown = LinearizeDepth(DepthMap[id.xy + int2(0, 1)]);
    
    float gradX = abs(depthRight - depthLeft);
    float gradY = abs(depthDown - depthUp);
    float depthGradient = max(gradX, gradY);
    
    float edgeStrength = saturate(depthGradient / DepthThreshold);
    edgeStrength = min(1.0, edgeStrength * EdgeIntensity);
    
    float4 finalColor = lerp(color, float4(0, 0, 0, color.a), edgeStrength);
	
	return finalColor;
}

float4 AddBlackBorder(float4 color, uint3 id, float EdgeSensitivity, float EdgeStrength, float EdgeWidth)
{
    float4 originalColor = color;

	float minDepth = 1e10;
    float maxDepth = -1e10;
    
    int radius = max(1, int(EdgeWidth * 2));
    
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x == 0 && y == 0) continue;
            
            float depth = LinearizeDepth(DepthMap[id.xy + int2(x, y)]);
            minDepth = min(minDepth, depth);
            maxDepth = max(maxDepth, depth);
        }
    }
    
    float centerDepth = LinearizeDepth(DepthMap[id.xy]);
    float depthRange = maxDepth - minDepth;
    
    bool nearMin = abs(centerDepth - minDepth) < depthRange * 0.2;
    bool nearMax = abs(centerDepth - maxDepth) < depthRange * 0.2;
    
    float edgeStrength = 0;
    if ((nearMin || nearMax) && depthRange > EdgeSensitivity)
    {
        edgeStrength = min(0.6f, depthRange / (EdgeSensitivity * 3));
        edgeStrength = edgeStrength * EdgeStrength;
    }
    
    float4 finalColor = lerp(originalColor, float4(0, 0, 0, originalColor.a), edgeStrength);
	
	return finalColor;
}

// ===================================================

float4 ApplyLUT(float4 color){
	float3 outColor; 

	//color.rgb *= 0.8;
	
	float3 gradedColor = LookupTexture.SampleLevel(SamplerLinear, color.rgb, 0).rgb;
	outColor = lerp(color.rgb, gradedColor, 0.985);

	return float4(outColor, color.a);
}

// ===================================================
[numthreads(16, 16, 1)]
void CS_main(uint3 id : SV_DispatchThreadID)
{
	uint2 pixel = id.xy;
	uint2 outSize;
	OutputColor.GetDimensions(outSize.x, outSize.y);
	if (pixel.x >= outSize.x || pixel.y >= outSize.y)
		return;

	float4 originalColor = InputColor[pixel];
	
	//float4 filteredColor = ApplyAFPFilter(originalColor);
	// float4 filteredColor = ApplyNewFilter(originalColor);
	// filteredColor = AddBlackBorder(filteredColor, id, 10, 0.75, 0.5);

    float4 filteredColor = ApplyLUT(originalColor);
	
	OutputColor[pixel] = filteredColor;
	OutputDepth[pixel] = DepthMap[pixel];
}
