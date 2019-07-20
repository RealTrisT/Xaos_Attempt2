#include "MansInterfacin.h"


#include <DirectXMath.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


#include "Targa.h"

//////////////////////////////////////////////////////////   DX VARIABLES   //////////////////////////////////////////////////////////

//_____________________________________________________________TEXTURING
ID3D11SamplerState* pSamplerState;


//_____________________________________________________________STENCIL
ID3D11Texture2D* pDepthStencil;
ID3D11DepthStencilView* pDepthStencilView;

ID3D11DepthStencilState* subtractStencilState;
ID3D11DepthStencilState* paintStencilState;
ID3D11DepthStencilState* nullStencilState;

//_____________________________________________________________BLENDING
ID3D11BlendState* blendState;

//_____________________________________________________________VERTEX&COLOR BUFFERS
ID3D11Buffer* pVertexBuffer;// the vertex buffer
ID3D11Buffer* pColorsBuffer;// the colors buffer
ID3D11Buffer* pTexelsBuffer;// the texels buffer

//_____________________________________________________________SHADERS
ID3D11VertexShader* pVS_color;    // the vertex shader for color
ID3D11PixelShader* pPS_color;     // the pixel shader for color
ID3D11InputLayout* pLayout_color; // the input layout for color

ID3D11VertexShader* pVS_texture;    // the vertex shader for textures
ID3D11PixelShader* pPS_texture;     // the pixel shader for textures
ID3D11InputLayout* pLayout_texture; // the input layout for textures

ID3D11Buffer* contextstantBuffer = 0; // constant buffer ( where the projection matrix is placed )

MansInterfacin::GraphicsSystem::GraphicsSystem(NativeWindowSystem* native_window_system, void(*callback_initedgraphics)(GraphicsSystem*)) 
	: WindowSystem(native_window_system), callback_initedgraphics(callback_initedgraphics)
{}

void MansInterfacin::GraphicsSystem::InitD3D() {

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------DEVICE AND SWAPCHAIN-----------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC desc = { 0 };

	desc.BufferCount = 2;                                    // one back buffer
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	desc.OutputWindow = this->WindowSystem->hWnd;            // the window to be used
	desc.SampleDesc.Count = 1;                               // how many multisamples
	desc.SampleDesc.Quality = 0;
	desc.Windowed = TRUE;                                    // windowed/full-screen mode
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;			 // sixteh

	HRESULT success = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		0,
		0,
		0,
		0,
		D3D11_SDK_VERSION,
		&desc,
		&this->swapchain,
		&this->device,
		0,
		&this->context
	);
	printf("D3D11CreateDeviceAndSwapChain: %s\n", (success == S_OK) ? "success" : "failed");
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------BACK BUFFER-----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// get the address of the back buffer
	ID3D11Texture2D* pBackBuffer;
	this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);

	// use the back buffer address to create the render target
	success = this->device->CreateRenderTargetView(pBackBuffer, NULL, &this->backbuffer);
	pBackBuffer->Release();

	printf("CreateRenderTargetView: %s\n", (success == S_OK) ? "success" : "failed");

	// set the render target as the back buffer
	this->context->OMSetRenderTargets(1, &this->backbuffer, NULL);


	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------VIEWPORT------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// Set the viewport
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)this->WindowSystem->width;
	viewport.Height = (float)this->WindowSystem->height;
	this->context->RSSetViewports(1, &viewport);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------SHADERS-------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	HRESULT hr;

	ID3D10Blob* VS_color, * PS_color;
	hr = D3DCompile(GraphicsSystem::shader_color, strlen(GraphicsSystem::shader_color) + 1, "VS", 0, 0, "VShader", "vs_4_0", 0, 0, &VS_color, 0);
	hr = D3DCompile(GraphicsSystem::shader_color, strlen(GraphicsSystem::shader_color) + 1, "PS", 0, 0, "PShader", "ps_4_0", 0, 0, &PS_color, 0);
	device->CreateVertexShader(VS_color->GetBufferPointer(), VS_color->GetBufferSize(), NULL, &pVS_color);
	device->CreatePixelShader(PS_color->GetBufferPointer(), PS_color->GetBufferSize(), NULL, &pPS_color);


	ID3D10Blob* VS_texture, * PS_texture;
	hr = D3DCompile(GraphicsSystem::shader_texture, strlen(GraphicsSystem::shader_texture) + 1, "VS", 0, 0, "VShader", "vs_4_0", 0, 0, &VS_texture, 0);
	printf("compiling texture vertex shade: %X\n", hr);
	hr = D3DCompile(GraphicsSystem::shader_texture, strlen(GraphicsSystem::shader_texture) + 1, "PS", 0, 0, "PShader", "ps_4_0", 0, 0, &PS_texture, 0);
	printf("compiling texture pixel shade: %X\n", hr);
	device->CreateVertexShader(VS_texture->GetBufferPointer(), VS_texture->GetBufferSize(), NULL, &pVS_texture);
	device->CreatePixelShader(PS_texture->GetBufferPointer(), PS_texture->GetBufferSize(), NULL, &pPS_texture);

	context->PSSetShader(pPS_color, 0, 0);
	context->VSSetShader(pVS_color, 0, 0);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------INPUT LAYOUT----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC ied_color[] = {
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	success = this->device->CreateInputLayout(ied_color, 2, VS_color->GetBufferPointer(), VS_color->GetBufferSize(), &pLayout_color);
	printf("CreateInputLayout: %s\n", (success == S_OK) ? "success" : "failed");
	this->context->IASetInputLayout(pLayout_color);

	D3D11_INPUT_ELEMENT_DESC ied_texture[] = {
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,						D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		1, offsetof(colortexel, t),	D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	success = this->device->CreateInputLayout(ied_texture, 2, VS_texture->GetBufferPointer(), VS_texture->GetBufferSize(), &pLayout_texture);
	printf("creating input layout for texture shaders: %X\n", hr);
	//this->context->IASetInputLayout(pLayout_texture);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------TEXTURING-----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	D3D11_SAMPLER_DESC sampler_description = {};
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;//D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_description.AddressU =
		sampler_description.AddressV =
		sampler_description.AddressW =
		D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_description.MinLOD = 0;
	sampler_description.MaxLOD = D3D11_FLOAT32_MAX;
	this->device->CreateSamplerState(&sampler_description, &pSamplerState);
	this->context->PSSetSamplers(0, 1, &pSamplerState);



	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------PROJECTION MATRIX--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	DirectX::XMMATRIX orthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.f, float(this->WindowSystem->width), float(this->WindowSystem->height), 0.f, 0.f, 1.f);

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(orthoMatrix);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &orthoMatrix;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	device->CreateBuffer(&cbDesc, &InitData, &contextstantBuffer);
	context->VSSetConstantBuffers(0, 1, &contextstantBuffer);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------VERTEX BUFFER----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//this section no longer exists since I can now generate vertex buffers dynamically

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------DEPTH STENCIL----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	D3D11_TEXTURE2D_DESC depthStencilDescription = {};
	depthStencilDescription.Width = this->WindowSystem->width;
	depthStencilDescription.Height = this->WindowSystem->height;

	depthStencilDescription.MipLevels = 1;
	depthStencilDescription.ArraySize = 1;

	depthStencilDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilDescription.SampleDesc.Count = 1;
	depthStencilDescription.SampleDesc.Quality = 0;

	depthStencilDescription.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDescription.CPUAccessFlags = 0;
	depthStencilDescription.MiscFlags = 0;
	printf("depth stencil createtex2D: %X\n", this->device->CreateTexture2D(&depthStencilDescription, NULL, &pDepthStencil));

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
	DSVDesc.Format = depthStencilDescription.Format;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;

	this->device->CreateDepthStencilView(pDepthStencil, &DSVDesc, &pDepthStencilView);
	this->context->OMSetRenderTargets(1, &this->backbuffer, pDepthStencilView);
	this->context->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	depthStencilStateDesc.DepthEnable = false;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = true;
	depthStencilStateDesc.StencilReadMask = 0xFF;
	depthStencilStateDesc.StencilWriteMask = 0xFF;
	depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;

	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	printf("createDepthStencilState 1 : %x\n", this->device->CreateDepthStencilState(&depthStencilStateDesc, &subtractStencilState));

	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	printf("createDepthStencilState 2 : %x\n", this->device->CreateDepthStencilState(&depthStencilStateDesc, &paintStencilState));

	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	printf("createDepthStencilState 3 : %x\n", this->device->CreateDepthStencilState(&depthStencilStateDesc, &nullStencilState));

	this->context->OMSetDepthStencilState(nullStencilState, 0);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------ALPHA BLENDING---------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


	this->device->CreateBlendState(&blendStateDesc, &blendState);
	this->context->OMSetBlendState(blendState, NULL, 0xFFFFFF);

	/*-----------------------Initiated,-Do-Callback---------------------------*/
	/*----*/ if (callback_initedgraphics)callback_initedgraphics(this); /*------*/
	/*------------------------------------------------------------------------*/
}

void MansInterfacin::GraphicsSystem::TermD3D(void) {
	pVertexBuffer->Release();
	contextstantBuffer->Release();
	pVS_color->Release();
	pPS_color->Release();
	this->swapchain->Release();
	this->backbuffer->Release();
	this->device->Release();
	this->context->Release();
}

void MansInterfacin::GraphicsSystem::UpdateVertexBuffer(const std::vector<f3coord>& vertex_array) {
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(this->context->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }
	memcpy(ms.pData, vertex_array.data(), vertex_array.size() * sizeof(f3coord));
	this->context->Unmap(pVertexBuffer, NULL);
}

void MansInterfacin::GraphicsSystem::UpdateColorsBuffer(const std::vector<f4color>& colors_array) {
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(this->context->Map(pColorsBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }
	memcpy(ms.pData, colors_array.data(), colors_array.size() * sizeof(f4color));
	this->context->Unmap(pColorsBuffer, NULL);
}

void MansInterfacin::GraphicsSystem::UpdateTexelsBuffer(const std::vector<f2coord>& coords_array) {
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(this->context->Map(pTexelsBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }
	memcpy(ms.pData, coords_array.data(), coords_array.size() * sizeof(f2coord));
	this->context->Unmap(pTexelsBuffer, NULL);
}

void MansInterfacin::GraphicsSystem::SetTopology(Topologies topology){
	this->context->IASetPrimitiveTopology(topology == Topologies::TRIANGLE_LIST ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void MansInterfacin::GraphicsSystem::SetRenderColorState() {
	context->PSSetShader(pPS_color, 0, 0);
	context->VSSetShader(pVS_color, 0, 0);
	this->context->IASetInputLayout(pLayout_color);
}
void MansInterfacin::GraphicsSystem::SetRenderTextureState() {
	context->PSSetShader(pPS_texture, 0, 0);
	context->VSSetShader(pVS_texture, 0, 0);
	this->context->IASetInputLayout(pLayout_texture);
}

void MansInterfacin::GraphicsSystem::SetSubtractiveStencilState(uint8_t stencil_level) {
	this->context->OMSetDepthStencilState(subtractStencilState, stencil_level);
}

void MansInterfacin::GraphicsSystem::SetPaintingStencilState(uint8_t stencil_level) {
	this->context->OMSetDepthStencilState(paintStencilState, stencil_level);
}

void MansInterfacin::GraphicsSystem::SetNullStencilState() {
	this->context->OMSetDepthStencilState(nullStencilState, 0);
}

void MansInterfacin::GraphicsSystem::ClearStencilBuffer() {
	this->context->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void MansInterfacin::GraphicsSystem::Draw(UINT offset, UINT vertex_amount) {
	context->Draw(vertex_amount, offset);
}

void MansInterfacin::GraphicsSystem::ClearFrame(f4color col) {
	this->context->ClearRenderTargetView(this->backbuffer, (float*)& col);
}

void MansInterfacin::GraphicsSystem::PresentFrame() {
	this->swapchain->Present(1, 0);
}

const char* MansInterfacin::GraphicsSystem::shader_color = R"(
	cbuffer VS_CONSTANT_BUFFER : register(b0)
	{
		matrix mMat;
	};

	struct VOut
	{
		float4 position : SV_POSITION;
		float4 color : COLOR;
	};

	VOut VShader(float4 position : POSITION, float4 color : COLOR)
	{
		VOut output;
		output.position = mul(mMat, position);
		output.color = color;
		return output;
	}

	float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
	{
		return float4(color[0]*color[3], color[1]*color[3], color[2]*color[3], color[3]);
	}
)";



const char* MansInterfacin::GraphicsSystem::shader_texture = R"(
	cbuffer VS_CONSTANT_BUFFER : register(b0)
	{
		matrix mMat;
	};

	struct VOut
	{
		float4 position : SV_POSITION;
		float2 texcoord : TEXCOORD;
	};

	VOut VShader(float4 position : POSITION, float2 texcoord : TEXCOORD)
	{
		VOut output;
		output.position = mul(mMat, position);
		output.texcoord = texcoord;
		return output;
	}


	Texture2D objTexture : TEXTURE : register(t0);
	SamplerState objSamplerState : SAMPLER : register(s0);

	float4 PShader(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
	{
		float4 color = objTexture.Sample(objSamplerState, texcoord);
		return float4(color[0]*color[3], color[1]*color[3], color[2]*color[3], color[3]);
	}
)";