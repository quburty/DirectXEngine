Texture2D<float4> Input;
RWTexture2D<float4> Output;

[numthreads(500, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    float4 color = Input.Load(id);
    
    Output[id.xy] = color;
}