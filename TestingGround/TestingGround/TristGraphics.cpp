#include "TristGraphics.h"


#include <stdio.h>
#include <chrono>
#include <thread>


#include <DirectXMath.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


#include "Targa.h"


HANDLE windowInitedEvent;
void UD::InitWindow(LRESULT(CALLBACK *WindowProc_a)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)) {
	if (WindowProc_a)this->WindowProc_p = WindowProc_a;

	windowInitedEvent = CreateEventA(NULL, TRUE, FALSE, NULL);	//create the event that we're gonna wait for until the window is done initializing
	puts("creating thread to create window and init dx");
	this->windowThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)UD::windowFunc, (LPVOID)this, 0, 0);
	puts("thread created");
	WaitForSingleObject(windowInitedEvent, INFINITE);			//wait for the window to finish initializing and signal the event
	puts("shit inited");
	CloseHandle(windowInitedEvent);
}

void UD::TermWindow(){
	PostMessageA(this->hWnd, WM_QUIT, 0, 0);					//let the message loop know we wanna shut this bitch down
	if (WaitForSingleObject(this->windowThread, 1000) == WAIT_TIMEOUT) { TerminateThread(this->windowThread, 0); puts("forcibly shutting down"); }
																// ^ wait for a second for the message loop to figure out it's done, if it doesn't then something fucked up, force the thread to exit
	this->windInited = false;
}

DWORD UD::windowFunc(UD* _this){
	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = _this->WindowProc_p;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = "GaymeBoy";
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNSHADOW);
	auto classerino = RegisterClassA(&wc);
	if (!classerino) {
		printf("Error registering class %d", GetLastError());
		return 0;
	}

	RECT desired_size = { 0, 0, (LONG)_this->width, (LONG)_this->height };
	AdjustWindowRect(&desired_size, WS_VISIBLE | WS_POPUP, FALSE);

	_this->hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, (LPCSTR)classerino, "GaiBoi", WS_VISIBLE | WS_POPUP, 0, 0, desired_size.right - desired_size.left, desired_size.bottom - desired_size.top, 0, 0, 0, 0);
	if (!_this->hWnd) {
		printf("Error creating window %d", GetLastError());
		return 0;
	}

	_this->windInited = true;
	_this->callback_initedWindow(_this);

	puts("inited, signaling event");
	SetEvent(windowInitedEvent);							//signal the event to let the parent thread know we're done with initializing

	MSG msg;
	while (true) {
		if (PeekMessage(&msg, 0, 0, 0, 0)) {
			if (!GetMessage(&msg, 0, 0, 0))
				break; // got a WM_QUIT
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));		//TODO: 2 headers for this?
	}

	return 0;
}

void UD::SetWindowProc(LRESULT(CALLBACK *WindowProc_a)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)){
	this->WindowProc_p = WindowProc_a;
	if(this->windInited)SetWindowLongPtr(this->hWnd, GWLP_WNDPROC, (LONG_PTR)&WindowProc_a);
}

void UD::SetWindowInitCallback (void(*callback_initedWindow_a )(UD*)){ callback_initedWindow  = callback_initedWindow_a ; }
void UD::SetDirectxInitCallback(void(*callback_initedDirectx_a)(UD*)){ callback_initedDirectx = callback_initedDirectx_a; }

LRESULT UD::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	} return DefWindowProc(hWnd, message, wParam, lParam);
}



//////////////////////////////////////////////////////////   DX VARIABLES   //////////////////////////////////////////////////////////

//_____________________________________________________________TEXTURING
ID3D11SamplerState* pSamplerState;


//_____________________________________________________________STENCIL
ID3D11Texture2D* pDepthStencil;
ID3D11DepthStencilView* pDepthStencilView;

ID3D11DepthStencilState* subtractStencilState;
ID3D11DepthStencilState* paintStencilState;

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

void UD::InitD3D(){

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------DEVICE AND SWAPCHAIN-----------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC desc = { 0 };

	desc.BufferCount = 2;                                    // one back buffer
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	desc.OutputWindow = this->hWnd;                          // the window to be used
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
	printf("D3D11CreateDeviceAndSwapChain: %s\n", (success == S_OK)?"success":"failed");
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------BACK BUFFER-----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

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
	viewport.Width  = (float)this->width;
	viewport.Height = (float)this->height;
	this->context->RSSetViewports(1, &viewport);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------SHADERS-------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	HRESULT hr;

	ID3D10Blob *VS_color, *PS_color;
	hr = D3DCompile(UD::shader_color, strlen(UD::shader_color) + 1, "VS", 0, 0, "VShader", "vs_4_0", 0, 0, &VS_color, 0);
	hr = D3DCompile(UD::shader_color, strlen(UD::shader_color) + 1, "PS", 0, 0, "PShader", "ps_4_0", 0, 0, &PS_color, 0);
	device->CreateVertexShader(VS_color->GetBufferPointer(), VS_color->GetBufferSize(), NULL, &pVS_color);
	device->CreatePixelShader(PS_color->GetBufferPointer(), PS_color->GetBufferSize(), NULL, &pPS_color);


	ID3D10Blob* VS_texture, * PS_texture;
	hr = D3DCompile(UD::shader_texture, strlen(UD::shader_texture) + 1, "VS", 0, 0, "VShader", "vs_4_0", 0, 0, &VS_texture, 0);
	printf("compiling texture vertex shade: %X\n", hr);
	hr = D3DCompile(UD::shader_texture, strlen(UD::shader_texture) + 1, "PS", 0, 0, "PShader", "ps_4_0", 0, 0, &PS_texture, 0);
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
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		2, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	success = this->device->CreateInputLayout(ied_texture, 2, VS_texture->GetBufferPointer(), VS_texture->GetBufferSize(), &pLayout_texture);
	printf("creating input layout for texture shaders: %X\n", hr);
	//this->context->IASetInputLayout(pLayout_texture);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------TEXTURING-----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	D3D11_SAMPLER_DESC sampler_description = {};
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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

	DirectX::XMMATRIX orthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.f, float(width), float(height), 0.f, 0.f, 1.f);

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

	unsigned int vstride = sizeof(f3coord);
	unsigned int voffset = 0;
	D3D11_BUFFER_DESC vbd = { 0 };
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = 200 * sizeof(f3coord);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	unsigned int cstride = sizeof(f4color);
	unsigned int coffset = 0;
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = 200 * sizeof(f4color);
	cbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	unsigned int tstride = sizeof(f2coord);
	unsigned int toffset = 0;
	D3D11_BUFFER_DESC tbd = { 0 };
	tbd.Usage = D3D11_USAGE_DYNAMIC;
	tbd.ByteWidth = 200 * sizeof(f2coord);
	tbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&vbd, NULL, &pVertexBuffer);
	device->CreateBuffer(&cbd, NULL, &pColorsBuffer);
	device->CreateBuffer(&tbd, NULL, &pTexelsBuffer);

	ID3D11Buffer* buffers[3] = { pVertexBuffer, pColorsBuffer, pTexelsBuffer };
	unsigned int strides[3] = { vstride, cstride, tstride };
	unsigned int offsets[3] = { voffset, coffset, toffset };

	context->IASetVertexBuffers(0, 3, buffers, strides, offsets);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------DEPTH STENCIL----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	D3D11_TEXTURE2D_DESC depthStencilDescription = {};
	depthStencilDescription.Width = this->width;
	depthStencilDescription.Height = this->height;

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
	depthStencilStateDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFunc			= D3D11_COMPARISON_NEVER;

	depthStencilStateDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_REPLACE;
	depthStencilStateDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_REPLACE;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_REPLACE;
	depthStencilStateDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	printf("createDepthStencilState 1 : %x\n", this->device->CreateDepthStencilState(&depthStencilStateDesc, &subtractStencilState));

	depthStencilStateDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_NOT_EQUAL;
	printf("createDepthStencilState 2 : %x\n", this->device->CreateDepthStencilState(&depthStencilStateDesc, &paintStencilState));

	
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
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL & (~D3D11_COLOR_WRITE_ENABLE_ALPHA);
	

	this->device->CreateBlendState(&blendStateDesc, &blendState);
	this->context->OMSetBlendState(blendState, NULL, 0xFFFFFF);


	//--/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\----/\-- EXPERIMENTAL START

	TargaFile file = {};
	if (!TargaFile::open("../test_images/walkcat.tga", &file)) {
		printf("NIGAAAAAAAAAAAAAAAA\n");
		return;
	}
	
	size_t pic_area = file.header.image_specification.width * file.header.image_specification.height;									//make a temp variable for size cuz that shit's big

	std::vector<uint8_t> buffer(pic_area * 4);																							//buffer to save the R8G8B8A8_UNORM to

	if (file.header.image_specification.bpp != 32) {																					//if it's 32 bits (with alpha and everything)
		if (file.header.image_specification.bpp == 24) {																				//if it's 24 bits,
			std::vector<uint8_t> temp_buffer(file.header.image_specification.width* file.header.image_specification.height * 3);		//allocate space for the whole image data section
			file.readImageData(temp_buffer.data());																						//read it whole
			for (size_t i = 0, t = 0; i < temp_buffer.size(); i += 3, t += 4) {															//and for each pixel
				buffer[t + 0] = temp_buffer[i + 2];																						//copy it over to the 32-bit
				buffer[t + 1] = temp_buffer[i + 1];																						//allocation, and beware of
				buffer[t + 2] = temp_buffer[i + 0];																						//the endianness (not sure about this yet)
				buffer[t + 3] = 0xFF;																									//set alpha to 1, since it's got no alpha channel that's all it can be
			}
		} else {																														//if it's not 24 bit, I haven't done it yet
			printf("FUUUUUUUUUUUUUUCK ITS NOT 32 OR 24 REEEEEEEEEEEE %d (alpha is %d)\n",												//so just scream off the top of your lungs
				file.header.image_specification.bpp,																					//that < 24 bit machine broke, and the bit 
				file.header.image_specification.image_descriptor & 0xF																	//per pixel amount, as well as the amount
			);																															//of bits dedicated to the alpha channel
			return;
		}
	} else {
		file.readImageData(buffer.data());																								//otherwise just read it into the buffer, which prolly doesn't
	}																																	//work cuz endianness, but I haven't tried it yet so fuck it lul


	ID3D11Texture2D* the_texture;
	ID3D11ShaderResourceView* the_texture_resourceview;


	D3D11_TEXTURE2D_DESC texture_description = {};
	texture_description.Width = file.header.image_specification.width;
	texture_description.Height = file.header.image_specification.height;
	texture_description.MipLevels = 0;
	texture_description.ArraySize = 1;
	texture_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_description.SampleDesc.Count = 1;
	texture_description.SampleDesc.Quality = 0;
	texture_description.Usage = D3D11_USAGE_DEFAULT;
	texture_description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture_description.CPUAccessFlags = 0;
	texture_description.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	if (FAILED(device->CreateTexture2D(&texture_description, NULL, &the_texture))) {
		printf("FUUUUUUUUUUUUUUCK ITS NOT CREATE TEXTURE\n");
		return;
	}
	context->UpdateSubresource(the_texture, 0, NULL, buffer.data(), texture_description.Width * 4, 0);


	D3D11_SHADER_RESOURCE_VIEW_DESC texture_resourceview_description = {};

	texture_resourceview_description.Format = texture_description.Format;
	texture_resourceview_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	texture_resourceview_description.Texture2D.MostDetailedMip = 0;
	texture_resourceview_description.Texture2D.MipLevels = -1;

	if (FAILED(device->CreateShaderResourceView(the_texture, &texture_resourceview_description, &the_texture_resourceview))){
		printf("FUUUUUUUUUUUUUUCK ITS NOT CREATE TEXTURE RESOURCE VIEW\n");
		return;
	}

	context->GenerateMips(the_texture_resourceview);

	context->PSSetShaderResources(0, 1, &the_texture_resourceview);

	//--\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/----\/-- EXPERIMENTAL END

	/*-----------------------Initiated,-Do-Callback---------------------------*/
	/*----*/ if (callback_initedDirectx)callback_initedDirectx(this); /*------*/
	/*------------------------------------------------------------------------*/
}

void UD::TermD3D(void){
	pVertexBuffer->Release();
	contextstantBuffer->Release();
	pVS_color->Release();
	pPS_color->Release();
	this->swapchain->Release();
	this->backbuffer->Release();
	this->device->Release();
	this->context->Release();
}

void UD::UpdateVertexBuffer(const std::vector<f3coord>& vertex_array) {
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(this->context->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }
	memcpy(ms.pData, vertex_array.data(), vertex_array.size() * sizeof(f3coord));
	this->context->Unmap(pVertexBuffer, NULL);
}

void UD::UpdateColorsBuffer(const std::vector<f4color>& colors_array) {
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(this->context->Map(pColorsBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }
	memcpy(ms.pData, colors_array.data(), colors_array.size() * sizeof(f4color));
	this->context->Unmap(pColorsBuffer, NULL);
}

void UD::UpdateTexelsBuffer(const std::vector<f2coord>& coords_array) {
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(this->context->Map(pTexelsBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }
	memcpy(ms.pData, coords_array.data(), coords_array.size() * sizeof(f2coord));
	this->context->Unmap(pTexelsBuffer, NULL);
}

void UD::SetRenderColorState() {
	context->PSSetShader(pPS_color, 0, 0);
	context->VSSetShader(pVS_color, 0, 0);
	this->context->IASetInputLayout(pLayout_color);
}
void UD::SetRenderTextureState() {
	context->PSSetShader(pPS_texture, 0, 0);
	context->VSSetShader(pVS_texture, 0, 0);
	this->context->IASetInputLayout(pLayout_texture);
}

void UD::SetSubtractiveStencilState(uint8_t stencil_level) {
	this->context->OMSetDepthStencilState(subtractStencilState, stencil_level);
}

void UD::SetPaintingStencilState(uint8_t stencil_level) {
	this->context->OMSetDepthStencilState(paintStencilState, stencil_level);
}

void UD::SetNullStencilState() {
	this->context->OMSetDepthStencilState(nullptr, 0);
}

void UD::ClearStencilBuffer() {
	this->context->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void UD::Draw(UINT offset, UINT vertex_amount) {
	context->Draw(vertex_amount, offset);
}

void UD::ClearFrame(f4color col) {
	this->context->ClearRenderTargetView(this->backbuffer, (float*)&col);
}

void UD::PresentFrame(){
	this->swapchain->Present(1, 0);
}


void UD::DefaultCallbackAfterWindowInit(UD * param){
	((UD*)param)->InitD3D();
}


UD::UD(unsigned int width, unsigned int height, void(*callback_initedWindow)(UD *)) : width(width), height(height) {
	if (callback_initedWindow)this->callback_initedWindow = callback_initedWindow;
}

const char* UD::shader_color = R"(
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
		return float4(color[0]*color[3], color[1]*color[3], color[2]*color[3], color[3]);   //TODO: actually implement alpha blending https://docs.microsoft.com/en-us/windows/desktop/direct3d11/d3d10-graphics-programming-guide-blend-state
	}
)";



const char* UD::shader_texture = R"(
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