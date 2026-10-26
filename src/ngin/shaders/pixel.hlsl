#include "rootSignature.hlsl"

struct PixelInput {
  float4 position : SV_Position;
  float2 originalPosition : TEXCOORD0;
};

[RootSignature(ROOTSIG)]
float4 main(PixelInput input) : SV_Target {
  float2 pos = input.originalPosition;
  return float4(pos.y, pos.x, pos.x * -1.0f, 1.0f);
}
