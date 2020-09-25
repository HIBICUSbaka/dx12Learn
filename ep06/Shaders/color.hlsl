//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj; 
	// 系统后续自动填充至256B
};

struct VertexIn
{
	// 需要与输入布局描述相匹配
	// 至少需要着色器所需输入在输入布局中一定要有所描述
	// 输入布局中描述过的可以不作为着色器输入签名
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;		// GPU 会自动进行裁剪、光栅化等操作，因此仍然需要齐次裁剪空间信息
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	// 顶点的齐次坐标为 1
	// mul 方法可根据输入进行自动重载
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target	// SV_Target 表示该值将存在渲染目标之中，与其相匹配
{
    return pin.Color;
}


