#include "rootSignature.hlsl"

struct PixelInput {
  float4 position : SV_Position;
  float2 originalPosition : TEXCOORD0;
};

[RootSignature(ROOTSIG)]
float4 main(PixelInput input) : SV_Target {
  float2 pos = input.originalPosition;
  pos.x = pos.x * 0.5f + 0.5f;
  pos.y = pos.y * 0.5f + 0.5f;
  return float4(pos.y , pos.x - pos.y * 0.5f, (pos.x - 1.0f) * -1.0f - pos.y * 0.5f, 1.0f);
}
