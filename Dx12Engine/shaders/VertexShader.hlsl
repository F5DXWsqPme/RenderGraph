#include "vsout.hlsli"

struct VsIn
{
  float3 pos : POSITION;
  float3 color : COLOR;
};

VsOut main(VsIn input)
{
  VsOut output;
  output.pos = float4(input.pos, 1.0);
  output.color = float4(input.color, 1.0);
  return output;
}
