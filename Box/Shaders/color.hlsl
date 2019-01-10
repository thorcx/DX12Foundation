//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************
//constant buffer binding to b0-bn register
//Shader resource views binding to t0-tn
//unordered access views binding to u0-un
//texture samplers Binding to s0-sn
cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4 gPulseColor;
	float gTime;
	
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vin.PosL.xy += 0.5f*sin(vin.PosL.x)*sin(3.0f*gTime);
	vin.PosL.z *= 0.6f + 0.4f*sin(2.0f*gTime);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//clip(pin.Color.r - 0.5f);
	const float pi = 3.14159;
	
	float s = 0.5f*sin(2 * gTime - 0.25f*pi) + 0.5f;
	float4 c = lerp(pin.Color, gPulseColor, s);
    return c;
}


