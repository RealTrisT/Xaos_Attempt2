#include "TristGraphics.h"


#include <stdio.h>
#include <chrono>
#include <thread>


#include <DirectXMath.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")




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

ID3D11Buffer* pVertexBuffer;// the vertex buffer
ID3D11VertexShader* pVS;    // the vertex shader
ID3D11PixelShader* pPS;     // the pixel shader
ID3D11InputLayout* pLayout; // the input layout
ID3D11Buffer* pConstantBuffer = 0; // constant buffer ( where the projection matrix is placed )
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
		&this->pSwapchain, 
		&this->pDevice, 
		0, 
		&this->pCon
	);
	printf("D3D11CreateDeviceAndSwapChain: %s\n", (success == S_OK)?"success":"failed");
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------BACK BUFFER-----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	this->pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	success = this->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &this->backbuffer);
	pBackBuffer->Release();

	printf("CreateRenderTargetView: %s\n", (success == S_OK) ? "success" : "failed");

	// set the render target as the back buffer
	this->pCon->OMSetRenderTargets(1, &this->backbuffer, NULL);


	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------VIEWPORT------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// Set the viewport
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width  = (float)this->width;
	viewport.Height = (float)this->height;
	this->pCon->RSSetViewports(1, &viewport);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------SHADERS-------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	ID3D10Blob *VS, *PS;
	HRESULT hr;

	hr = D3DCompile(UD::shader, strlen(UD::shader) + 1, "VS", 0, 0, "VShader", "vs_4_0", 0, 0, &VS, 0);
	hr = D3DCompile(UD::shader, strlen(UD::shader) + 1, "PS", 0, 0, "PShader", "ps_4_0", 0, 0, &PS, 0);

	pDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	pDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

	pCon->PSSetShader(pPS, 0, 0);
	pCon->VSSetShader(pVS, 0, 0);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------INPUT LAYOUT----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	success = this->pDevice->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
	printf("CreateInputLayout: %s\n", (success == S_OK) ? "success" : "failed");
	this->pCon->IASetInputLayout(pLayout);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------PROJECTION MATRIX--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	DirectX::XMMATRIX mat = DirectX::XMMatrixOrthographicOffCenterLH(0.f, float(width), float(height), 0.f, 0.f, 1.f);

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(mat);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &mat;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	pDevice->CreateBuffer(&cbDesc, &InitData, &pConstantBuffer);
	pCon->VSSetConstantBuffers(0, 1, &pConstantBuffer);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------VERTEX BUFFER----------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	unsigned int stride = sizeof(PIXEL_VERTEX);
	unsigned int offset = 0;
	D3D11_BUFFER_DESC bd = { 0 };

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = this->width * this->height * sizeof(PIXEL_VERTEX);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	pDevice->CreateBuffer(&bd, NULL, &pVertexBuffer);
	pCon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	/*-----------------------Initiated,-Do-Callback---------------------------*/
	/*----*/ if (callback_initedDirectx)callback_initedDirectx(this); /*------*/
	/*------------------------------------------------------------------------*/
}

void UD::TermD3D(void){
	pVertexBuffer->Release();
	pConstantBuffer->Release();
	pVS->Release();
	pPS->Release();
	this->pSwapchain->Release();
	this->backbuffer->Release();
	this->pDevice->Release();
	this->pCon->Release();
}

void UD::ShitImage(b3color * buffer, float alpha) {
	unsigned int size = this->width * this->height;
	PIXEL_VERTEX * pPix = new PIXEL_VERTEX[size];

	unsigned int i = 0;
	for (size_t y = 0; y < this->height; y++) {
		for (size_t x = 0; x < this->width; x++) {
			pPix[i] = {
				((float)x + 1),
				((float)y + 1),
				0,
				f4color(f3color(buffer[i]), alpha)
			};
			i++;
		}
	}

	this->ShitImage(pPix);

	delete[] pPix;
}

void UD::ShitImage(f3color * buffer, float alpha) {
	unsigned int size = this->width * this->height;
	PIXEL_VERTEX * pPix = new PIXEL_VERTEX[size];

	unsigned int i = 0;
	for (size_t y = 0; y < this->height; y++) {
		for (size_t x = 0; x < this->width; x++) {
			pPix[i] = {
				((float)x + 1),
				((float)y + 1),
				0,
				f4color(buffer[i], alpha)
			};
			i++;
		}
	}

	this->ShitImage(pPix);

	delete[] pPix;
}

void UD::ShitImage(f4color * buffer){
	unsigned int size = this->width * this->height;
	PIXEL_VERTEX * pPix = new PIXEL_VERTEX[size];

	unsigned int i = 0;
	for (size_t y = 0; y < this->height; y++) {
		for (size_t x = 0; x < this->width; x++){
			pPix[i] = {
				((float)x+1),
				((float)y+1),
				0,
				buffer[i]
			};
			i++;
		}
	}

	this->ShitImage(pPix);

	delete[] pPix;
}

void UD::ShitImage(PIXEL_VERTEX* pPix) {

	unsigned int size = this->width * this->height;
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(pCon->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) { puts("failed to map"); return; }	// map the buffer
	memcpy(ms.pData, pPix, size * sizeof(PIXEL_VERTEX));																// copy the data
	pCon->Unmap(pVertexBuffer, NULL);

	pCon->Draw(size, 0);
}

void UD::ClearFrame(f4color col) {
	this->pCon->ClearRenderTargetView(this->backbuffer, col);
}

void UD::PresentFrame(){
	this->pSwapchain->Present(1, 0);
}


void UD::DefaultCallbackAfterWindowInit(UD * param){
	((UD*)param)->InitD3D();
}


UD::UD(unsigned int Width, unsigned int Height, void(*Callback_initedWindow)(UD *)) : width(Width), height(Height) {
	if (Callback_initedWindow)callback_initedWindow = Callback_initedWindow;
}

const char* UD::shader = R"(
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