cbuffer cbPerFrame : register(b0) {
	matrix projectionFromWorldMtx;
};

cbuffer cbPerFrame2 : register(b1) {
	matrix projectionFromWorldMtx2;
};


float4 VS_Main(float4 pos : POSITION) : SV_POSITION {

	return pos;
    return mul(pos, projectionFromWorldMtx2);

}


float4 PS_Main(float4 pos : SV_POSITION) : SV_TARGET {

	return float4(0.0f, 1.0f, 0.0f, 1.0f);

}
