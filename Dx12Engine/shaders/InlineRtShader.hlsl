RaytracingAccelerationStructure scene : register(t0, space0);
RWTexture2D<float4> renderTarget : register(u0, space0);

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
  float2 coord = (float2)threadId.xy;
  
  RayQuery<RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;

  RayDesc ray;
  ray.Origin = float3(coord / float2(800, 600), 0);
  ray.Direction = float3(0, 0, 1);
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
    renderTarget[coord] = float4(1, 0, 0, 1);
  }
  else
  {
    renderTarget[coord] = float4(0, 0, 0, 1);
  }
}
