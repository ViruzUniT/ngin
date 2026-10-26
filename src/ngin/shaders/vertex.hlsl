#include "rootSignature.hlsl"

struct VertexOutput {
  float4 position : SV_Position;
  float2 originalPosition : TEXCOORD0;
};

[RootSignature(ROOTSIG)]
VertexOutput main(float2 pos : Position) {
  VertexOutput output;
  output.position = float4(pos.xy, 0.0f, 1.0f);
  output.originalPosition = pos;
  return output;
}
