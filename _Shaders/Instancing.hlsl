cbuffer World_CBuffer : register(b0)
{
    matrix World;
}

cbuffer VP_CBuffer : register(b1)
{
    matrix View;
    matrix Projection;
};

cbuffer Light_CBuffer : register(b2)
{
    float3 Direction;
}

cbuffer Inst_CBuffer : register(b3)
{
    matrix Worlds[1000];
}

struct VertexInput
{
    float4 Position : Position;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float2 Uv : Uv;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float3 oPosition : Position1;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float2 Uv : Uv;
};

Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);
SamplerState Sampler;
DepthStencilState DSS;

VertexOutput VS(VertexInput input, uint instID: SV_InstanceID)
{
    VertexOutput output;
    output.Position = input.Position;
    
    output.Position = mul(output.Position, Worlds[instID]);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.Uv = input.Uv;
    output.oPosition = input.Position.xyz;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    
    return output;
}

float4 PS(VertexOutput output) :SV_Target
{
    float NdotL = dot(output.Normal, -Direction);
    return float4(DiffuseMap.Sample(Sampler, output.Uv).rgb * NdotL, 1.0f);
}
