#include "vsout.hlsli"

float4 main(VsOut input) : SV_TARGET
{
  return input.color;
}
