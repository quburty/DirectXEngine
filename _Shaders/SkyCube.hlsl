cbuffer World_CBuffer : register(b0)
{
    matrix World;
}

cbuffer VP_CBuffer : register(b1)
{
    matrix View;
    matrix Projection;
};

struct VertexInput
{
    float4 Position : Position;
    float2 Uv : Uv;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float3 oPosition : Position1;
    float2 Uv : Uv;
};

TextureCube Texture0;
SamplerState Sampler;

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    output.Position = input.Position;
    
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.Uv = input.Uv;
    output.oPosition = input.Position.xyz;
    
    return output;
}

float4 PS(VertexOutput output) :SV_Target
{
    return Texture0.Sample(Sampler, output.oPosition.rgb);
}
