// D3D11tut1.cpp : Defines the entry point for the application.
//


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3d10.lib")//pragma comment indicates to the linker that the library in quotes is to be listed as a dependency on the project
#pragma comment(lib,"d3dcompiler.lib")

#include "stdafx.h"

#include <windows.h>
#include "D3D11tut1.h"
#include <d3d11.h>
#include <d3d11_2.h>
#include <d3d10.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <WICTextureLoader.h>


using namespace DirectX;

#define MAX_LOADSTRING 100

// Global Variables:
LPCWSTR WndClassName = L"first window";
HWND hWnd = NULL;
const int SCREENWIDTH = 800;
const int SCREENHEIGHT = 600;
IDXGISwapChain* SwapChain; //allows for swapping between front and back buffers, which in turn allows smooth rendering
ID3D11Device* d3d11Device; //handles loading of objects into memory
ID3D11DeviceContext* d3d11DevCon; //handles actual rendering
ID3D11RenderTargetView* renderTargetView; //essentially the back buffer. Data gets written to this object, and subsequently rendered
ID3D11Buffer* VertexBuffer;
ID3D11Buffer* IndexBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* PS_Buffer;
ID3D10Blob* VS_Buffer;
ID3D11InputLayout* vertLayout;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;
ID3D11Buffer* cbPerObjectBuffer;
ID3D11RasterizerState* FULL;

ID3D11ShaderResourceView*CubeTexture;
ID3D11SamplerState*CubesTexSamplerState;

ID3D11BlendState* Transparency;
ID3D11RasterizerState*CCWcullMode;//CCW means counterclockwise; CW means clockwise
ID3D11RasterizerState*CWcullMode;
//defining matrices for world space, view space, and projection space
XMMATRIX WVP;
XMMATRIX World;
XMMATRIX camView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camTarget;
XMVECTOR camUp;

struct cbPerObject {
	XMMATRIX WVP;
};

cbPerObject cbPerObj;

//matrices for world transformations
XMMATRIX cube1World;
XMMATRIX cube2World;

XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;
float rot = 0.01f;

//declaring vertex struct and vertice input layout
struct Vertex {
	XMFLOAT3 pos; 
	XMFLOAT2 texCoord; 
};

D3D11_INPUT_ELEMENT_DESC layout[] = { //specifies input layout when vertex data is written to vertex buffer
	{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 }//Note:fifth parameter specifies offset in bytes from beginning of struct
};

UINT NUMELEMENTS = ARRAYSIZE(layout);

bool InitializeDirect3dApp(HINSTANCE hInstance);
void ReleaseObjects();
bool InitScene();
void UpdateScene();
void DrawScene();
bool InitializeWindow(HINSTANCE,
	int,
	int, int,
	bool);

int messageloop();
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
{

    // Perform application initialization:
    if (!InitializeWindow (hInstance,nCmdShow,SCREENWIDTH,SCREENHEIGHT,true))
    {
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
    }
	if (!InitializeDirect3dApp(hInstance)) {
		MessageBox(0, L"Direct3D Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}
	if (!InitScene()) {
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
	}
	

    // Main message loop:
	messageloop();

	return 0;
	ReleaseObjects();
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
bool InitializeWindow(HINSTANCE hInstance,int nCmdShow,int SCREENWIDTH,int SCREENHEIGHT, bool windowed)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+2);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = WndClassName;
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hWnd = CreateWindowEx(NULL, WndClassName, L"Window Title", WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,SCREENWIDTH,SCREENHEIGHT,NULL,NULL,hInstance, NULL);
	if (!hWnd) {
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return true;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) { 
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hWnd);
		}
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler
int messageloop() {
	MSG msg;

	ZeroMemory(&msg, sizeof(MSG));
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {//takes one message from queue and inserts it into msg
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg); //sends message to WndProc for processinh
		}
		else {
			UpdateScene();
			DrawScene();

		}

	}
	return msg.wParam;
}

bool InitializeDirect3dApp(HINSTANCE hInstance) {
	HRESULT hr;

	DXGI_MODE_DESC bufferDesc; //buffer description object. Allows specification of buffer characteristics

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = SCREENWIDTH;
	bufferDesc.Height = SCREENHEIGHT;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1; //back buffer refresh rate is represented by the numerator over denominator
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //sets pixel format. Currently set to eight bits for R,G,B, and alpha
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //specifies when each scanline is rendered. Currently unspecified becaused scanline order doesn't matter if you don't see it.
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //determines how image is scaled to a monitor. We are windowed so specifications are unneeded

	DXGI_SWAP_CHAIN_DESC swapChainDesc; //allows description of swapchain characteristics

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0; //SampleDesc subobject describes amount of anti-aliasing that occurs.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //sets buffer as the render output
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //allows display driver to determine best use for used buffers

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer); 

	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView); //back buffer creation
	BackBuffer->Release();

	//Depth Stencil Buffer description
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = SCREENWIDTH;
	depthStencilDesc.Height = SCREENHEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //this format allocates 24 bits for the depth and 8 for the stencil
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Creation of Depth Stencil Buffer
	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	return true;
}


void ReleaseObjects() {
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	renderTargetView->Release();
	VertexBuffer->Release();
	IndexBuffer->Release();
	SwapChain->Release();
	depthStencilBuffer->Release();
	depthStencilView->Release();
	cbPerObjectBuffer->Release();
	FULL->Release();

	CCWcullMode->Release();
	Transparency->Release();
	CWcullMode->Release();
}

bool InitScene() {
	HRESULT hr;
	//Compiling Shaders
	hr = D3DCompileFromFile(L"Effects.fx", 0, 0, "VS", "vs_5_0", 0, 0, &VS_Buffer, 0);
	hr = D3DCompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_5_0", 0, 0, &PS_Buffer, 0);
	//Creating Shaders
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	//Setting Shaders
	d3d11DevCon->VSSetShader(VS, NULL, NULL);
	d3d11DevCon->PSSetShader(PS, NULL, NULL);

	//Creating and populating Vertex Buffers
	Vertex v[] = { //remember that structs do not have constructors unless defined!
		// Front Face
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
		{{1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
		{{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},

		// Back Face
		{ { -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
		{ { 1.0f, -1.0f, 1.0f},{ 0.0f, 1.0f}},
		{ { 1.0f,  1.0f, 1.0f}, {0.0f, 0.0f}},
		{ { -1.0f,  1.0f, 1.0f},{ 1.0f, 0.0f}},

		// Top Face
		{ { -1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
		{ { -1.0f, 1.0f,  1.0f}, {0.0f, 1.0f}},
		{ { 1.0f, 1.0f,  1.0f}, {0.0f, 0.0f}},
		{ { 1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}},

		// Bottom Face
		{ { -1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
		{ { 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{ { 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},
		{ { -1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},

		// Left Face
		{ { -1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
		{ { -1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},
		{ { -1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
		{ { -1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},

		// Right Face
		{ { 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{ { 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
		{ { 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
		{ { 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}}

	};
	//Buffer description
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT; //describes how buffer is used
	vertexBufferDesc.ByteWidth = sizeof(Vertex)*24; // specifies the size of buffer; dependent on amount of vertices passed and size of vertices
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;//Specifies that this is a vertex buffer
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	//Specifies what kind of data is placed in buffer
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &VertexBuffer);
	//set buffer data
	UINT stride = sizeof(Vertex);//size of each Vertex
	UINT offset = 0;// how far from the buffer beginning we start
	d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	//create indices to be put into index buffer
	DWORD indices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};
	//Buffer description is mostly the same as vertex buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12*3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &IndexBuffer);
	d3d11DevCon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//set input layout
	hr = d3d11Device->CreateInputLayout(layout, NUMELEMENTS, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
	d3d11DevCon->IASetInputLayout(vertLayout);

	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//Create and set viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREENWIDTH;
	viewport.Height = SCREENHEIGHT;
	viewport.MinDepth = 0.0;
	viewport.MaxDepth = 1.0;

	d3d11DevCon->RSSetViewports(1, &viewport);

	
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(cbPerObject);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA constantBufferData;
	constantBufferData.pSysMem = &cbPerObj;

	hr = d3d11Device->CreateBuffer(&constantBufferDesc, &constantBufferData, &cbPerObjectBuffer);


	camPosition = XMVectorSet(0.0f, -5.0f, -10.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); 
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float)SCREENWIDTH / SCREENHEIGHT, 1.0f, 1000.0f);

	//Describe and create rasterizer state
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID; //specifies drawn primitives will resemble wireframes
	wfdesc.CullMode = D3D11_CULL_FRONT; // specifies that both sides of primitives will be drawn
	hr = d3d11Device->CreateRasterizerState(&wfdesc, &FULL);

	//load textures into memory
	hr = CreateWICTextureFromFile(d3d11Device, L"nuclear-symbol.jpg", NULL, &CubeTexture, 0);
	//describe how the texture is rendered
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; 
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = d3d11Device->CreateSamplerState(&sampDesc, &CubesTexSamplerState);

	//describe and create blend state
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	d3d11Device->CreateBlendState(&blendDesc, &Transparency);

	//define rasterizer states for blending
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.CullMode = D3D11_CULL_BACK;
	cmdesc.FillMode = D3D11_FILL_SOLID;

	cmdesc.FrontCounterClockwise = true;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CWcullMode);


	return true;
}

void UpdateScene()  { //implements any changes from previous frame
	rot += .002f;
	static float inc = 0.001f;
	static float trans = 0.0f;
	static float scale = 1.0f;
	static float sinc = 0.0005f;
	trans += inc;
	if (trans >= 3.0 || trans <= -3.0) {
		inc *= -1;
	}
	if (rot > 6.28f)
		rot = 0;
	cube1World = XMMatrixIdentity();
	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Rotation = XMMatrixRotationAxis(rotaxis, rot);
	Translation = XMMatrixTranslation(0.0f, trans, 5.0f);

	cube1World = Translation* Rotation;//Note that matrix effects are applied in the order that they are multiplied. 
	//For example, the operation performed above applies the translation effect, then the rotation

	cube2World = XMMatrixIdentity();
	Rotation = XMMatrixRotationAxis(rotaxis, -rot);
	if (scale >= 3.0 || scale < 1.0)
		sinc *= -1;
	scale += sinc;
	Scale = XMMatrixScaling(scale, scale, 1.3f);

	cube2World = Rotation*Scale;

	return;
}

void DrawScene() { // performs actual rendering
	//clear backbuffer
	float bgColor[4] = { 0.0, 0.0, 0.0, 1.0f };
	
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	//clear depth stencil
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0.0);
	World = XMMatrixIdentity();
	//first cube operations
	float blendFactor[] = { 0.75f,0.75f,0.75f,1.0f };
	//set default blend state(no blending)
	d3d11DevCon->OMSetBlendState(0, 0, 0xffffff);

	d3d11DevCon->OMSetBlendState(Transparency, blendFactor, 0xffffff);
	//find object closer to camera for rendering purposes
	XMVECTOR cubePos = XMVectorZero();
	cubePos = XMVector3TransformCoord(cubePos, cube1World);

	float distX = XMVectorGetX(cubePos) - XMVectorGetX(camPosition);
	float distY = XMVectorGetY(cubePos) - XMVectorGetY(camPosition);
	float distZ = XMVectorGetZ(cubePos) - XMVectorGetZ(camPosition);

	float cube1Dist = distX * 2 + distY * 2 + distZ * 2;

	cubePos = XMVectorZero();
	cubePos = XMVector3TransformCoord(cubePos, cube2World);

	distX = XMVectorGetX(cubePos) - XMVectorGetX(camPosition);
	distY = XMVectorGetY(cubePos) - XMVectorGetY(camPosition);
	distZ = XMVectorGetZ(cubePos) - XMVectorGetZ(camPosition);

	float cube2Dist = distX * 2 + distY * 2 + distZ * 2;

	if (cube1Dist < cube2Dist) {
		XMMATRIX tempMatrix = cube1World;
		cube1World = cube2World;
		cube2World = tempMatrix;
	}

	WVP = cube1World*camView*camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);

	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);

	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	d3d11DevCon->PSSetShaderResources(0, 1, &CubeTexture);
	d3d11DevCon->PSSetSamplers(0, 1, &CubesTexSamplerState);
	d3d11DevCon->RSSetState(CCWcullMode);
	d3d11DevCon->DrawIndexed(36, 0,0);
	//d3d11DevCon->Draw(24, 0);

	d3d11DevCon->RSSetState(CWcullMode);
	d3d11DevCon->DrawIndexed(36, 0, 0);
	//second cube operations

	WVP = cube2World*camView*camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);

	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);

	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	d3d11DevCon->PSSetShaderResources(0, 1, &CubeTexture);
	d3d11DevCon->PSSetSamplers(0, 1, &CubesTexSamplerState);
	d3d11DevCon->RSSetState(CCWcullMode);

	d3d11DevCon->DrawIndexed(36, 0, 0);
	//d3d11DevCon->Draw(24, 0);
	d3d11DevCon->RSSetState(CWcullMode);

	d3d11DevCon->DrawIndexed(36, 0, 0);

	SwapChain->Present(0,0);
}