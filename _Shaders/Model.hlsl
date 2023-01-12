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

cbuffer Model_CBuffer : register(b4)
{
    matrix Bones[1000];
}

struct VertexInput
{
    float4 Position : Position;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float2 Uv : Uv;
    
    uint4 Indices : Indices;
    float4 Weights : Weights;
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

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    output.Position = input.Position;
    
    matrix boneWorld = matrix(float4(0, 0, 0, 0), float4(0, 0, 0, 0), float4(0, 0, 0, 0), float4(0, 0, 0, 0));
    boneWorld += Bones[input.Indices[0]] * input.Weights[0];
    boneWorld += Bones[input.Indices[1]] * input.Weights[1];
    boneWorld += Bones[input.Indices[2]] * input.Weights[2];
    boneWorld += Bones[input.Indices[3]] * input.Weights[3];
    
    output.Position = mul(output.Position, boneWorld);
    output.Position = mul(output.Position, World);
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
    float4 normal = float4(output.Normal, 0.0f);
    float4 tangent = float4(output.Tangent, 0.0f);
    float4 bitangent = float4(cross(output.Normal, output.Tangent), 0.0f);
    matrix space = matrix(normal, tangent, bitangent, float4(0, 0, 0, 1));
    
    float3 n = mul(NormalMap.Sample(Sampler, output.Uv), space).rgb;
    
    float NdotL = dot(output.Normal, -Direction);
    return float4(DiffuseMap.Sample(Sampler, output.Uv).rgb * NdotL, 1.0f);
}
