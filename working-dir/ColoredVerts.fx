cbuffer cbPerFrame : register(b0) {
	matrix projectionFromWorldMtx;
};

cbuffer cbPerObject : register(b1) {
	matrix worldFromModelMtx;
};

struct VS_Output {
    float4 pos   : SV_POSITION;
    float4 color : COLOR0;
};


VS_Output VS_Main(float4 inPos : POSITION, float4 inColor : COLOR) {

    VS_Output output;
    output.pos   = mul(inPos, worldFromModelMtx);
    output.pos   = mul(output.pos, projectionFromWorldMtx);
    
    output.color = inColor;
    
    return output;

}


float4 PS_Main(VS_Output input) : SV_TARGET {

	//return float4(0.0f, 1.0f, 0.0f, 1.0f);
    return input.color;

}
