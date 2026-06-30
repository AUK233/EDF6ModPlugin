// input
Texture2D<float> m_z_buffer : register(t0);
RWTexture2D<float> m_out_depth : register(u0);

cbuffer CalLinearDepthParam : register(b0) {
	float2 m_near_far_plane;
	float2 pad;
};
// ===================================================
float LinearizeDepth(float depth) {
	float near = m_near_far_plane.x;
	float far = m_near_far_plane.y;
	
	float denominator = depth * (far - near) - far;
	float numerator = far * near;
	float linearDepth = numerator / denominator;
	
	return -linearDepth;
}
// ===================================================
[numthreads(16, 16, 1)]
void CS_main(uint3 id : SV_DispatchThreadID) {
	m_out_depth[id.xy] = LinearizeDepth(m_z_buffer[id.xy]);
}
