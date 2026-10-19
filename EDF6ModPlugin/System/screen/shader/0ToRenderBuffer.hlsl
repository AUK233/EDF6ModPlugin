// ===================================================
// in c++
cbuffer xgl_user_param : register(b2) {
	int m_offset;
	float3 pp_pad;
}

Texture2D<float4> InputColorP1 : register(t0);
Texture2D<float4> InputColorP2 : register(t1);

RWTexture2D<float4> OutputHUDless : register(u0);
RWTexture2D<float4> OutputColor : register(u1);

// ===================================================
[numthreads(16, 16, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 pixel = threadID.xy;
	uint2 outSize;
	OutputColor.GetDimensions(outSize.x, outSize.y);
	if (pixel.x >= outSize.x || pixel.y >= outSize.y)
		return;

	float4 inputColor = InputColorP1[pixel];

	if (m_offset > 0){
		uint2 pixelP2;
		pixelP2.x = pixel.x + m_offset;
		pixelP2.y = pixel.y;

		if (pixelP2.x < outSize.x)
		{
			float4 colorP2 = InputColorP2[pixel];
			OutputHUDless[pixelP2] = colorP2;
			OutputColor[pixelP2] = colorP2;
		}
	}

	OutputHUDless[pixel] = inputColor;
	OutputColor[pixel] = inputColor;

}
