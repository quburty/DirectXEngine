#pragma once

class ComputeShader final
{
public:
	ComputeShader(std::wstring wfile);
	~ComputeShader();

	void SetComputeSize(uint x, uint y, uint z) { computeX = x; computeY = y; computeZ = z; }
	void SetSRVs(ID3D11ShaderResourceView* const* srvs, uint count) { SRVs = srvs; srvCount = count; }
	void SetUAVs(ID3D11UnorderedAccessView* const* uavs, uint count) { UAVs = uavs; uavCount = count; }
	void SetCBuffer(ID3D11Buffer* cbuffer, uint slot);

	void Compute();

private:
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;

	ID3D10Blob* csBlob;
	ID3D11ComputeShader* shader;

	ID3D11ShaderResourceView* const * SRVs;
	uint srvCount = 0;

	ID3D11UnorderedAccessView* const * UAVs;
	uint uavCount = 0;

	uint computeX;
	uint computeY;
	uint computeZ;
};