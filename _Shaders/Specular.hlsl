cbuffer World_CBuffer : register(b0)
{
    matrix World;
}

cbuffer VP_CBuffer : register(b1)
{
    matrix View;
    matrix Projection;
};

cbuffer PerFrame_CBuffer : register(b2)
{
    float3 Direction;
    float3 Camera;
    float padding;
    
    float roughness;
    float ambient;
    float diffuse;
    float specular;
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
SamplerState Sampler;
DepthStencilState DSS;

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    output.Position = input.Position;
    
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
    float3 normal = normalize(output.Normal);
    float3 l = -normalize(Direction);
    float NdotL = saturate(dot(normal, l));
    
    float3 v = normalize(Camera - output.oPosition);
    //float3 reflection = reflect(Direction, normal);
    //float cosine = saturate(dot(v, reflection));
    
    //float fresnel = saturate(pow(cosine, roughness) * (roughness + 8)/8);
    
    float fresnel = max(dot(normal, l), 0) * (roughness + 8.f) / 8.f *
        pow(max(dot(normal, (l + v) / 2.f), 0), roughness);
    
    fresnel = saturate(fresnel);
    
    return float4(DiffuseMap.Sample(Sampler, output.Uv).rgb * 
        (ambient + NdotL * diffuse + fresnel * specular), 1.0f);
}

float4 PS1(VertexOutput output) : SV_Target
{
    return float4(output.oPosition,1.0f);
}
