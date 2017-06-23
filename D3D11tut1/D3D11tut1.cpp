// D3D11tut1.cpp : Defines the entry point for the application.
//

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11.lib")
#pragma comment(lib,"d3dx10.lib")

#include<windows.h>
#include "stdafx.h"
#include "D3D11tut1.h"
#include<d3d11.h>
#include <d3d11_2.h>
#include <d3d10.h>

#define MAX_LOADSTRING 100

// Global Variables:
LPCWSTR WndClassName = L"first window";
HWND hWnd = NULL;
const int SCREENWIDTH = 800;
const int SCREENHEIGHT = 600;
IDXGISwapChain* SwapChain; //allows for swapping between front and back buffers, which in turn allows smooth rendering
ID3D11Device* d3d11Device; //handles loading of objects into memory
ID3D11DeviceContext* d3d11DevCon; //handles actual rendering
ID3D11RenderTargetView* renderTargetView; //essentially the back buffer. Data gets written to this object, and subsequ

float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 1;
int colormodg = 1;
int colormodb = 1;


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

	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);

	return true;
}

void ReleaseObjects() {
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
}

bool InitScene() {
	return true;
}

void UpdateScene()  { //implements any changes from previous frame
	red = colormodr*0.00005f;
	green += colormodg*0.00002f;
	blue += colormodb*0.00001f;

	if (red >= 1.0f || red <= 0.0f)
		colormodr *= -1;
	if (green >= 1.0f || green <= 0.0f)
		colormodg *= -1;
	if (blue >= 1.0f || blue < 0.0f) 
		colormodb *= -1;
}

void DrawScene() { // performs actual rendering
	float bgColor[4] = { red, green, blue, 1.0f };

	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	SwapChain->Present(0,0);
}