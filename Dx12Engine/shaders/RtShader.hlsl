RaytracingAccelerationStructure scene : register(t0, space0);
RWTexture2D<float4> renderTarget : register(u0, space0);

typedef BuiltInTriangleIntersectionAttributes Attributes;
struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void RaygenShader()
{
  RayDesc ray;
  ray.Origin = float3((float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions(), 0);
  ray.Direction = float3(0, 0, 1);
  ray.TMin = 0.001;
  ray.TMax = 1000.0;
  RayPayload payload = { float4(0, 0, 0, 0) };
  TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
  
  renderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void ClosestHitShader(inout RayPayload payload, in Attributes attr)
{
  float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
  payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void MissShader(inout RayPayload payload)
{
  payload.color = float4(0, 0, 0, 1);
}
