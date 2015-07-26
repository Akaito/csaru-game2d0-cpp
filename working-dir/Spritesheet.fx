cbuffer cbPerFrame : register(b0) {
    matrix projectionFromWorldMtx;
};

cbuffer cbPerObject : register(b1) {
    matrix worldFromModelMtx;
    matrix projectionFromModelMtx;
    float2 textureDims;
    float2 frameTexPos;
    float2 frameTexDims;
};

Texture2D    colorMap_     : register(t0);
SamplerState colorSampler_ : register(s0);

struct VS_Input {
    float4 pos  : POSITION;
    float2 tex0 : TEXCOORD0;
};

struct PS_Input {
    float4 pos  : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};


PS_Input VS_Main(VS_Input vertex) {

    PS_Input vs_out = (PS_Input)0;
    
    // Position
    float4 outPos = vertex.pos;
    outPos.xy *= frameTexDims;
    
    /*
    outPos = mul(outPos, worldFromModelMtx);
    outPos = mul(outPos, projectionFromWorldMtx);
    /*/
    outPos = mul(outPos, projectionFromModelMtx);
    /*/
    outPos = mul(
        outPos,
        transpose(mul(
            transpose(worldFromModelMtx),
            transpose(projectionFromWorldMtx)
        ))
    );
    //*/
    
    outPos.xy *= 1.0;
    vs_out.pos = outPos;
    
    // Texture coords
    vs_out.tex0 = (frameTexPos + vertex.tex0 * frameTexDims) / textureDims;
    
    return vs_out;
}


float4 PS_Main(PS_Input frag) : SV_TARGET {

    float4 result = colorMap_.Sample(colorSampler_, frag.tex0);
    return result;
}
