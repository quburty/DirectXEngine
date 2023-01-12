//나중에 vertex 구조 다른거 쓸 수 있을까..?
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
    float height;
    float offset;
};

Texture2D<float4> Input;
RWStructuredBuffer<VertexModel> Output;

[numthreads(500, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    uint x = id.x % width;
    uint y = id.x / width;
    
    float r = Input.Load(uint3(x, y, 0)).r * 256.f;
    
    VertexModel result;
    result.position = float3(x * offset, r, y * offset);
    result.normal = float3(0, 0, 0);
    result.tangent = float3(1, 0, 0);
    result.uv = float2((float) x / width, (float) y / height);
    
    result.indices = uint4(0,0,0,0);
    result.weights = float4(0,0,0,0);
    
    Output[y * width + x] = result;
}