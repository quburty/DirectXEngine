struct VertexModel
{
    float3 position;
    float2 uv;
    float3 normal;
    float3 tangent;
    
    uint4 indices;
    float4 weights;
};

cbuffer Width_CBuffer : register(b6)
{
    float width;
    float3 center;
};

RWStructuredBuffer<VertexModel> Output : register(u0);
RWByteAddressBuffer VertexOutput : register(u1);

[numthreads(500, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    uint x = id.x % width;
    uint y = id.x / width;
    
    VertexModel result = Output[y * width + x];
    
    if(distance(result.position.xz,center.xz) < 100.f )
    {
        float offset = (100.f - distance(result.position.xz, center.xz))/100.f;
        result.position = Output[y * width + x].position - float3(0, offset, 0);
    }
    
    float3 v0 = result.position;
    float3 v1 = Output[id.x - 1].position;
    float3 v2 = Output[id.x - width].position;
    float3 v3 = Output[id.x - width + 1].position;
    float3 v4 = Output[id.x + 1].position;
    float3 v5 = Output[id.x + width - 1].position;
    float3 v6 = Output[id.x + width].position;
    
    result.normal = float3(0, 0, 0);
    result.normal += cross(v1 - v0, v2 - v0);
    result.normal += cross(v2 - v0, v3 - v0);
    result.normal += cross(v3 - v0, v4 - v0);
    result.normal += cross(v4 - v0, v5 - v0);
    result.normal += cross(v5 - v0, v6 - v0);
    result.normal += cross(v6 - v0, v1 - v0);
    //끝에는 어떻게함?
    
    result.weights = float4(cross(float3(1, 0, 0), float3(0, 1, 0)), 1);
    
    uint address = (y * width + x) * 76;
    
    VertexOutput.GetDimensions(result.indices[1]);
    
    Output[y * width + x] = result;

    VertexOutput.Store3(address + 0, asuint(result.position));
    VertexOutput.Store2(address + 12, asuint(result.uv));
    VertexOutput.Store3(address + 20, asuint(result.normal));
    VertexOutput.Store3(address + 32, asuint(result.tangent));
    VertexOutput.Store4(address + 44, asuint(result.indices));
    VertexOutput.Store4(address + 60, asuint(result.weights));

}