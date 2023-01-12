#include "Framework.h"
#include "ComputeShader.h"

ComputeShader::ComputeShader(std::wstring wfile)
{
	Device = D3D::Get()->GetDevice();
	DeviceContext = D3D::Get()->GetDeviceContext();

	HRESULT hr;

	if (Path::GetExtension(wfile) == L"cso")
	{
		HANDLE fileHandle = CreateFile(wfile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		bool bChecked = fileHandle != INVALID_HANDLE_VALUE;
		assert(bChecked);

		DWORD dataSize = GetFileSize(fileHandle, NULL);
		assert(dataSize != 0xFFFFFFFF);

		void* data = malloc(dataSize);
		DWORD readSize;
		Check(ReadFile(fileHandle, data, dataSize, &readSize, NULL));

		CloseHandle(fileHandle);
		fileHandle = NULL;

		hr = Device->CreateComputeShader(data, dataSize, nullptr, &shader);
		Check(hr);
	}
	else
	{
		ID3D10Blob* errorBlob = nullptr;

		hr = D3DX11CompileFromFileW
		(
			wfile.c_str(),
			nullptr,
			nullptr,
			"CS",
			"cs_5_0",
			NULL,
			NULL,
			nullptr,
			&csBlob,
			&errorBlob,
			nullptr
		);

		if (errorBlob != nullptr)
			MessageBoxA(Window::Get()->GetHWND(), (char*)errorBlob->GetBufferPointer(), "", 0);

		assert(SUCCEEDED(hr));

		hr = Device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &shader);
		assert(SUCCEEDED(hr));

		SAFE_RELEASE(csBlob);
	}
}

ComputeShader::~ComputeShader()
{
	SAFE_RELEASE(shader);

}

void ComputeShader::SetCBuffer(ID3D11Buffer * cbuffer, uint slot)
{
	DeviceContext->CSSetConstantBuffers(slot, 1, &cbuffer);
}

void ComputeShader::Compute()
{
	DeviceContext->CSSetShader(shader, nullptr, 0);
	DeviceContext->CSSetShaderResources(0, srvCount, SRVs);
	DeviceContext->CSSetUnorderedAccessViews(0, uavCount, UAVs, nullptr);

	DeviceContext->Dispatch(computeX, computeY, computeZ);
}
