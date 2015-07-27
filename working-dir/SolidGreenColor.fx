cbuffer cbPerFrame : register(b0) {
	matrix projectionFromWorldMtx;
};

cbuffer cbPerObject : register(b1) {
	matrix worldFromModelMtx;
};


float4 VS_Main(float4 pos : POSITION) : SV_POSITION {

	//return pos;
    float4 result = mul(pos, worldFromModelMtx);
    result = mul(result, projectionFromWorldMtx);
    return result;

}


float4 PS_Main(float4 pos : SV_POSITION) : SV_TARGET {

	return float4(0.0f, 1.0f, 0.0f, 1.0f);

}
