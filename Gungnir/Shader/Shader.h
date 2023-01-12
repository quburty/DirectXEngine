#pragma once

class Shader
{
public:
	Shader();
	Shader(std::wstring file);
	virtual ~Shader();

public:
	void SetShader(std::wstring file);
	void SetVertexShader(std::wstring file);
	void SetPixelShader(std::wstring file);

private:
	bool LoadShader(const std::wstring& file, ID3D10Blob** shaderBlob_ptr);
	bool CompileShader(const std::wstring& file, ID3D10Blob** shaderBlob_ptr);

public:
	//column-major
	void CreateVertexBuffer(void* vertices, uint stride, uint count,
		bool bGpuWrite = false, bool bUseUav = false);
	void CreateIndexBuffer(uint* indices, uint count);

	ID3D11Buffer* GetVertexBuffer() { return VertexBuffer; }

	bool IsVertexBufferValid() { return VertexBuffer != nullptr; }
	bool IsIndexBufferValid() { return IndexBuffer != nullptr; }

	void SetDiffuseMap(std::wstring file);
	void SetNormalMap(std::wstring file);
	void SetSpecularMap(std::wstring file);

	void SetDiffuseMap(void* data, uint length);
	void SetNormalMap(void* data, uint length);
	void SetSpecularMap(void* data, uint length);

	void SetDiffuseMap(ID3D11ShaderResourceView* srv);

	ID3D11RasterizerState* GetRasterizer() { return Rasterizer; }
	ID3D11DepthStencilState* GetDepthStencil() { return DepthStencil; }

	void CreateRasterizer(D3D11_RASTERIZER_DESC* desc);
	void CreateDepthStencil(D3D11_DEPTH_STENCIL_DESC* desc);

private:
	virtual void SetWorld(const D3DXMATRIX* world, class Mesh* mesh) {}

public:
	void RegisterMesh(class Mesh* mesh);
	void DelistMesh(class Mesh* mesh);
	uint GetMeshRefNum() { return meshes.size(); }

	virtual void Render(class Mesh* mesh);

protected:
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;

	std::set<class Mesh*> meshes;

private:
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D10Blob* VsBlob;
	ID3D10Blob* PsBlob;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11InputLayout* InputLayout;
	uint stride = 0;

	ID3D11SamplerState* SamplerState;

	ID3D11ShaderResourceView* SRV_diffuse;
	ID3D11ShaderResourceView* SRV_normal;
	ID3D11ShaderResourceView* SRV_specular;

	ID3D11Texture2D* DepthTexture;
	ID3D11DepthStencilView* DSV;

	ID3D11RasterizerState* Rasterizer;

	ID3D11DepthStencilState* DepthStencil;

protected:
	uint indexCount = 0;

private:
	bool bDiffuseMap = false;
	bool bNormalMap = false;
	bool bSpecularMap = false;

};