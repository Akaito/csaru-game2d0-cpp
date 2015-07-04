cbuffer cbChangesPerFrame : register(b0)
{
	matrix mvp_;
};

Texture2D colorMap_ : register(t0);
SamplerState colorSampler_ : register(s0);

struct VS_Input
{
	float4 pos : POSITION;
	float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
	float4 pos : SV_POSITION;
	float2 tex0 : TEXCOORD0;
};


PS_Input VS_Main(VS_Input vertex)
{
	PS_Input vs_out = (PS_Input)0;
	vs_out.pos = mul(vertex.pos, mvp_);
	vs_out.tex0 = vertex.tex0;
	
	return vs_out;
}


float4 PS_Main(PS_Input frag) : SV_TARGET
{
	return colorMap_.Sample(colorSampler_, frag.tex0);
}
