RaytracingAccelerationStructure scene : register(t0, space0);
RWTexture2D<float4> renderTarget : register(u0, space0);

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
  float2 coord = (float2)threadId.xy;
  
  RayQuery<RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;

  RayDesc ray;
  float3 at = float3(1.5, 1.5, 1.5);
  float3 look = float3(0, -0.2, 0);
  float3 dir = normalize(look - at);
  float3 right = normalize(cross(float3(0, 1, 0), dir));
  float3 up = normalize(cross(right, dir));
  ray.Origin = at;
  ray.Direction = dir + right * (coord.x / 800 - 0.5f) + up * (coord.y / 600 - 0.5f);
  ray.TMin = 0.001;
  ray.TMax = 1000.0;
  
  q.TraceRayInline(scene,
    RAY_FLAG_CULL_NON_OPAQUE |
    RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
    1,
    ray);
  
  q.Proceed();
  
  if (q.CommittedStatus() & COMMITTED_TRIANGLE_HIT)
  {
    float3 shadePoint = ray.Origin + ray.Direction * q.CommittedRayT();
    renderTarget[coord] = float4(lerp(float3(0.1, 0.2, 0.1), float3(1, 1, 1), shadePoint.y), 1);
  }
  else
  {
    renderTarget[coord] = float4(0, 0, 0, 1);
  }
}
