//--------------------------------------------------------------------------------------
// File: FluidDemo.cpp
//
// SPH Fluid Model.
// Fluid Surface Generated with Marching Cubes.
//--------------------------------------------------------------------------------------

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define DXUT_AUTOLIB

// DirectX headers
#include <d3d9.h>
#include <D3Dx9.h>
#include <D3D10_1.h>
#include <D3D10.h>
#include <D3Dx10.h>

// DirectX libraries
#pragma comment( lib, "D3D9.lib" )
#pragma comment( lib, "D3D10.lib" )
#pragma comment( lib, "D3D10_1.lib" )
#pragma comment( lib, "D3Dx10.lib" )
#pragma comment( lib, "D3Dx9.lib" )

// DXUT headers
#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "DXUTShapes.h"
#include "SDKmisc.h"
#include "SDKmesh.h"

// DXUT libraries
#pragma comment( lib, "DXUT.lib" )
#pragma comment( lib, "DXUTOpt.lib" )

#include "ParticleSystem.h"
#include "MarchingCubes.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CModelViewerCamera                  g_Camera;               // A model viewing camera

CDXUTDialogResourceManager          g_DialogResourceManager;// manager for shared resources of dialogs
CD3DSettingsDlg                     g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                         g_HUD;                  // manages the 3D UI
CDXUTDialog                         g_SampleUI;             // dialog for sample specific controls
ID3DX10Font*                        g_pFont = NULL;         // Font for drawing text
ID3DX10Sprite*                      g_pSprite = NULL;       // Sprite for batching text drawing
CDXUTTextHelper*                    g_pTxtHelper = NULL;

ID3D10Effect*                       g_pEffect = NULL;       // D3DX effect interface
ID3D10InputLayout*                  g_pVertexLayout = NULL; // Vertex Layout
ID3D10InputLayout*                  g_pBatchedVertexLayout = NULL; 
ID3D10EffectTechnique*              g_pPhongTechnique = NULL;
ID3D10EffectTechnique*              g_pSolidTechnique = NULL;
ID3D10EffectTechnique*              g_pBatchedTechnique = NULL;
ID3D10EffectTechnique*              g_pBatchedSquareTechnique = NULL;
ID3D10Buffer*						g_pSphereInstanceBuffer = NULL;
ID3D10Buffer*						g_pSquareInstanceBuffer = NULL;
ID3D10Buffer*						g_pMCVertexBuffer = NULL;

ID3D10RasterizerState*				g_pSolid;
ID3D10RasterizerState*				g_pWire;

ID3DX10Mesh*						g_pSphere = NULL;
ID3DX10Mesh*						g_pSquare = NULL;

ID3D10EffectMatrixVariable*         g_pWorldVariable = NULL; // transformations
ID3D10EffectMatrixVariable*         g_pViewVariable = NULL;
ID3D10EffectMatrixVariable*         g_pProjectionVariable = NULL;

ID3D10EffectVectorVariable*			g_pLightDirVariable = NULL; // lights
ID3D10EffectVectorVariable*			g_pLightColorVariable = NULL;

ID3D10EffectVectorVariable*			g_pAmbientColor		= NULL; 
ID3D10EffectVectorVariable*			g_pSphereColor		= NULL; 
ID3D10EffectScalarVariable*			g_pSpecPower		= NULL; 
ID3D10EffectVectorVariable*			g_pCameraPosition	= NULL; 

D3DXMATRIX                          g_World;

ID3D10Texture2D*				g_pDepthStencil = NULL;
ID3D10DepthStencilState*		g_pDSState = NULL;
ID3D10DepthStencilView*			g_pDSView = NULL;

ParticleSystem					g_ParticleSystem;
MarchingCubes					g_MarchingCubes;

//----------------
// Rendering Mode - true = SPH, false = MC
//----------------
bool							g_bRenderingMode = true;
bool							g_bIsogrid = false;
bool							g_bWire = false;
bool							g_bPause = true;
unsigned						g_uTargetIso = 50;
unsigned						g_uVertexCount = 0;

//-------------------------
// Rendering Structures
//-------------------------
struct SphereInstance
{
	D3DXVECTOR4		color;
	D3DXMATRIX		world;
};
// memory map for instance data
const unsigned	g_uMaxParticles = 500;
const unsigned	g_uTriangleLimit = 50000; // is 50000 enough?

//SphereInstance* g_pInstances = new SphereInstance[ g_uMaxParticles ]; // todo fix.  creates heap corruption
//MarchingCubesVertex* g_pMCVerts = new MarchingCubesVertex[ g_uTriangleLimit ];
#include "boost\shared_array.hpp"
boost::shared_array<MarchingCubesVertex> g_pMCVerts( new MarchingCubesVertex[ g_uTriangleLimit ] );
boost::shared_array<SphereInstance> g_pInstances( new SphereInstance[g_uMaxParticles] );
boost::shared_array<SphereInstance> g_pSquareInstances;

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
                                       DXGI_FORMAT BufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D10DestroyDevice( void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

void RenderText();
void InitApp();
void InitParticles();
std::string FloatToString( const float &f );

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackD3D10DeviceAcceptable( IsD3D10DeviceAcceptable );
    DXUTSetCallbackD3D10DeviceCreated( OnD3D10CreateDevice );
    DXUTSetCallbackD3D10SwapChainResized( OnD3D10ResizedSwapChain );
    DXUTSetCallbackD3D10SwapChainReleasing( OnD3D10ReleasingSwapChain );
    DXUTSetCallbackD3D10DeviceDestroyed( OnD3D10DestroyDevice );
    DXUTSetCallbackD3D10FrameRender( OnD3D10FrameRender );

    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen

    InitApp();

    DXUTCreateWindow( L"Fluid Demo" );
    DXUTCreateDevice( true, 800, 600 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
#define IDC_RENDER		1
#define IDC_ISOTEXT		2
#define IDC_ISO			3
#define IDC_RSSTATE		4
#define	IDC_ISOGRID		5

void InitApp()
{
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

	g_SampleUI.AddComboBox( IDC_RENDER, 15, -275, 100, 20);
	g_SampleUI.GetComboBox( IDC_RENDER )->AddItem( L"Spheres", NULL );
	g_SampleUI.GetComboBox( IDC_RENDER )->AddItem( L"Iso-Surface", NULL );

	g_SampleUI.AddStatic( IDC_ISOTEXT, L"Iso-Level", 40, -240, 50, 12 );
	g_SampleUI.AddSlider( IDC_ISO, 15, -218, 100, 18, 0, 300 );
	g_SampleUI.GetSlider( IDC_ISO )->SetValue( 50 );

	g_SampleUI.AddComboBox( IDC_RSSTATE, 15, -180, 100, 20);
	g_SampleUI.GetComboBox( IDC_RSSTATE )->AddItem( L"Solid", NULL );
	g_SampleUI.GetComboBox( IDC_RSSTATE )->AddItem( L"Wire", NULL );

	g_SampleUI.AddCheckBox( IDC_ISOGRID, L"Iso-Grid", 25, -140, 100, 20 );

    g_HUD.SetCallback( OnGUIEvent ); 

    g_SampleUI.SetCallback( OnGUIEvent );

	InitParticles();

	// set up the marching cubes class
	g_MarchingCubes.Initialization( -100, 100, 100, 100, -100, -100, 10, -10, -10 );

	g_pSquareInstances = boost::shared_array<SphereInstance>( new SphereInstance[ g_MarchingCubes.size_x * g_MarchingCubes.size_y * g_MarchingCubes.size_z ] );
}

void InitParticles()
{
	g_ParticleSystem.Reset();
}


//--------------------------------------------------------------------------------------
// Reject any D3D10 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
                                       DXGI_FORMAT BufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D10CreateDevice( pd3dDevice ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D10CreateDevice( pd3dDevice ) );
    V_RETURN( D3DX10CreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                                L"Arial", &g_pFont ) );
    V_RETURN( D3DX10CreateSprite( pd3dDevice, 512, &g_pSprite ) );
    g_pTxtHelper = new CDXUTTextHelper( NULL, NULL, g_pFont, g_pSprite, 15 );

    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif

    // Read the D3DX effect file
    V_RETURN( D3DX10CreateEffectFromFile( L"FluidDemo.hlsl", NULL, NULL, "fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL,
                                          NULL, &g_pEffect, NULL, NULL ) );

    // Obtain the technique
    g_pPhongTechnique = g_pEffect->GetTechniqueByName( "RenderPhong" );
    g_pBatchedTechnique = g_pEffect->GetTechniqueByName( "RenderPhongBatched" );
	g_pBatchedSquareTechnique  = g_pEffect->GetTechniqueByName( "RenderSolidSquareBatched" );
	g_pSolidTechnique = g_pEffect->GetTechniqueByName( "RenderSolid" );

    // Obtain the variables
    g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();
	g_pLightDirVariable = g_pEffect->GetVariableByName( "vLightDir" )->AsVector();
	g_pLightColorVariable = g_pEffect->GetVariableByName( "vLightColor" )->AsVector();
	g_pAmbientColor = g_pEffect->GetVariableByName( "vAmbientColor" )->AsVector();
	g_pSphereColor = g_pEffect->GetVariableByName( "vSphereColor" )->AsVector();
	g_pSpecPower = g_pEffect->GetVariableByName( "fSpecPower" )->AsScalar();	
	g_pCameraPosition = g_pEffect->GetVariableByName( "vCameraPos" )->AsVector();

    // Define the input layout
	{
		const D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = sizeof( layout ) / sizeof( layout[0] );

		// Create the input layout
		D3D10_PASS_DESC PassDesc;
		g_pPhongTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
		V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
												 PassDesc.IAInputSignatureSize, &g_pVertexLayout ) );
	}

	// Define input layout for instanced geometry
	{
		const D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	 12,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,		D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,	D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32,	D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48,	D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64,	D3D10_INPUT_PER_INSTANCE_DATA, 1 },	  
		};
		UINT numElements = sizeof( layout ) / sizeof( layout[0] );

		D3D10_PASS_DESC PassDesc;
		g_pBatchedTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
		V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature, 
												PassDesc.IAInputSignatureSize, &g_pBatchedVertexLayout ) );
	}

    // Create Instanced Buffers
	D3D10_BUFFER_DESC bufferDesc =
	{
		( g_uMaxParticles ) * sizeof( SphereInstance ),
		D3D10_USAGE_DYNAMIC,
		D3D10_BIND_VERTEX_BUFFER,
		D3D10_CPU_ACCESS_WRITE,
		0
	};

	V_RETURN( pd3dDevice->CreateBuffer( &bufferDesc, NULL, &g_pSphereInstanceBuffer ) );

	bufferDesc.ByteWidth = ( g_MarchingCubes.size_x * g_MarchingCubes.size_y *  g_MarchingCubes.size_z ) *
								sizeof( SphereInstance );

	V_RETURN( pd3dDevice->CreateBuffer( &bufferDesc, NULL, &g_pSquareInstanceBuffer ) );

    // Initialize the world matrix
    D3DXMatrixIdentity( &g_World );

    // Initialize the camera
    D3DXVECTOR3 Eye( 0.0f, 0.0f, -400.0f );
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    g_Camera.SetViewParams( &Eye, &At );

	// create the sphere mesh
	V_RETURN( DXUTCreateSphere( pd3dDevice, 10, 20, 20, &g_pSphere ) );
	V_RETURN( g_pSphere->CommitToDevice() );

	// create the box mesh
	V_RETURN( DXUTCreateBox( pd3dDevice, 1.0f, 1.0f, 1.0f, &g_pSquare ) );
	V_RETURN( g_pSquare->CommitToDevice() );

	// create the vertex buffer for marching cubes.
	{
		D3D10_BUFFER_DESC bufferDesc;
		ZeroMemory( &bufferDesc, sizeof( D3D10_BUFFER_DESC ) );
		bufferDesc.Usage             = D3D10_USAGE_DYNAMIC;
		bufferDesc.ByteWidth         = sizeof( MarchingCubesVertex ) * g_uTriangleLimit;
		bufferDesc.BindFlags         = D3D10_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags    = D3D10_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags         = 0; 

		V_RETURN( pd3dDevice->CreateBuffer( &bufferDesc, NULL, &g_pMCVertexBuffer ) );
	}

	// Set up rasterizer states
	D3D10_RASTERIZER_DESC rastDesc;
	rastDesc.AntialiasedLineEnable = 0;
	rastDesc.CullMode = D3D10_CULL_BACK;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0;
	rastDesc.DepthClipEnable = 1;
	rastDesc.FillMode = D3D10_FILL_SOLID;
	rastDesc.FrontCounterClockwise = 0;
	rastDesc.MultisampleEnable = 0;
	rastDesc.ScissorEnable = 0;
	rastDesc.SlopeScaledDepthBias = 0;

	pd3dDevice->CreateRasterizerState( &rastDesc, &g_pSolid );

	rastDesc.FillMode = D3D10_FILL_WIREFRAME;
	rastDesc.CullMode = D3D10_CULL_NONE;

	pd3dDevice->CreateRasterizerState( &rastDesc, &g_pWire );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D10ResizedSwapChain( pd3dDevice, pBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D10ResizedSwapChain( pd3dDevice, pBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = static_cast<float>( pBufferSurfaceDesc->Width ) /
        static_cast<float>( pBufferSurfaceDesc->Height );
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 5000.0f );
    g_Camera.SetWindow( pBufferSurfaceDesc->Width, pBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

    g_HUD.SetLocation( pBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBufferSurfaceDesc->Width - 170, pBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );

	// create a depth stencilbuffer
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width = pBufferSurfaceDesc->Width;
	descDepth.Height = pBufferSurfaceDesc->Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	V_RETURN( pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil ) );

	D3D10_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory( &dsDesc, sizeof( D3D10_DEPTH_STENCIL_DESC ) );
	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D10_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

	V_RETURN( pd3dDevice->CreateDepthStencilState(&dsDesc, &g_pDSState) );

	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	descDSV.Texture2D.MipSlice = 0;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;

	// Create the depth stencil view
	V_RETURN( pd3dDevice->CreateDepthStencilView( g_pDepthStencil, 
											 &descDSV,
											 &g_pDSView ) ); 
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D10 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();
    pd3dDevice->ClearRenderTargetView( pRTV, ClearColor );

    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    //
    // Clear the depth stencil
    //
    ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
    pd3dDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH || D3D10_CLEAR_STENCIL, 1.0, 0 );
	pd3dDevice->OMSetDepthStencilState( g_pDSState, 1 );

	//
	// Set Primitive Topology
	//

	pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//
	// Rendering Modes for the project
	//

	if( g_bRenderingMode == true )
	{
		//
		// Render the spheres
		//

		if( g_bWire == true )
		{
			pd3dDevice->RSSetState( g_pWire );
		}

		// Set vertex layout
		pd3dDevice->IASetInputLayout( g_pBatchedVertexLayout );

		// get vertex buffer from model
		ID3D10Buffer* pVertexBuffer;
		g_pSphere->GetDeviceVertexBuffer( 0, &pVertexBuffer );

		D3D10_BUFFER_DESC vertexDesc;
		ZeroMemory( &vertexDesc, sizeof( D3D10_BUFFER_DESC ) );
		pVertexBuffer->GetDesc( &vertexDesc );

		// get index buffer from model
		ID3D10Buffer* pIndexBuffer;
		g_pSphere->GetDeviceIndexBuffer(  &pIndexBuffer );

		D3D10_BUFFER_DESC indexDesc;
		ZeroMemory( &indexDesc, sizeof( D3D10_BUFFER_DESC ) );
		pIndexBuffer->GetDesc( &indexDesc );

		UINT strides[2] = { vertexDesc.ByteWidth / g_pSphere->GetVertexCount(), sizeof( SphereInstance ) };
		UINT offsets[2] = { 0, 0 };
		ID3D10Buffer* pVB[2] = { pVertexBuffer, g_pSphereInstanceBuffer };
		pd3dDevice->IASetVertexBuffers( 0, 2, pVB, strides, offsets );
		pd3dDevice->IASetIndexBuffer( pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

		// batched rendering
		D3D10_TECHNIQUE_DESC techDesc;
		g_pBatchedTechnique->GetDesc( &techDesc );
		for( UINT p = 0; p < techDesc.Passes; ++p )
		{
			g_pBatchedTechnique->GetPassByIndex( p )->Apply( 0 );
			g_pSphere->DrawSubsetInstanced( 0, g_ParticleSystem.GetParticles().size(), 0 );
		}

		SAFE_RELEASE( pVertexBuffer );
		SAFE_RELEASE( pIndexBuffer );

		pd3dDevice->RSSetState( g_pSolid );
	}
	else
	{
		//
		// Render the marching cubes isosurface
		//

		if( g_bWire == true )
		{
			pd3dDevice->RSSetState( g_pWire );
		}

		// Set Vertex Layout
		pd3dDevice->IASetInputLayout( g_pVertexLayout );

		UINT strides[1] = { sizeof( MarchingCubesVertex ) };
		UINT offsets[1] = { 0 };
		ID3D10Buffer* pVB[1] = { g_pMCVertexBuffer };
		pd3dDevice->IASetVertexBuffers( 0, 1, pVB, strides, offsets );
		
		D3D10_TECHNIQUE_DESC techDesc;
		g_pPhongTechnique->GetDesc( &techDesc );
		for( UINT p = 0; p < techDesc.Passes; ++p )
		{
			g_pPhongTechnique->GetPassByIndex( p )->Apply( 0 );
			if( g_uVertexCount > g_uTriangleLimit )
			{
				pd3dDevice->Draw( g_uTriangleLimit , 0 );
			}
			else
			{
				pd3dDevice->Draw( g_uVertexCount , 0 );
			}
		}

		pd3dDevice->RSSetState( g_pSolid );
	}

	// view iso - grid
	if( g_bIsogrid == true )
	{
		/*
		std::vector<vertex>::const_iterator i;
		for( i = g_MarchingCubes.m_Vertices.begin(); i != g_MarchingCubes.m_Vertices.end(); ++i )
		{
			D3DXMATRIX world;
			D3DXMatrixTranslation( &world, i->pos.x, i->pos.y, i->pos.z );

			g_pWorldVariable->SetMatrix( ( float* )&world);
		
			D3DXVECTOR4 vSphereColor = D3DXVECTOR4( i->flux / 100.0f, i->flux / 100.0f, i->flux / 100.0f, 0.25f );
			g_pSphereColor->SetFloatVector( (float *) vSphereColor );

			D3D10_TECHNIQUE_DESC techDesc;
			g_pSolidTechnique->GetDesc( &techDesc );
			for( UINT p = 0; p < techDesc.Passes; ++p )
			{
				g_pSolidTechnique->GetPassByIndex( p )->Apply( 0 );
				g_pSquare->DrawSubset( 0 );
			}
		}
		*/

		// Set vertex layout
		pd3dDevice->IASetInputLayout( g_pBatchedVertexLayout );

		// get vertex buffer from model
		ID3D10Buffer* pVertexBuffer;
		g_pSquare->GetDeviceVertexBuffer( 0, &pVertexBuffer );

		D3D10_BUFFER_DESC vertexDesc;
		ZeroMemory( &vertexDesc, sizeof( D3D10_BUFFER_DESC ) );
		pVertexBuffer->GetDesc( &vertexDesc );

		// get index buffer from model
		ID3D10Buffer* pIndexBuffer;
		g_pSquare->GetDeviceIndexBuffer(  &pIndexBuffer );

		D3D10_BUFFER_DESC indexDesc;
		ZeroMemory( &indexDesc, sizeof( D3D10_BUFFER_DESC ) );
		pIndexBuffer->GetDesc( &indexDesc );

		UINT strides[2] = { vertexDesc.ByteWidth / g_pSquare->GetVertexCount(), sizeof( SphereInstance ) };
		UINT offsets[2] = { 0, 0 };
		ID3D10Buffer* pVB[2] = { pVertexBuffer, g_pSquareInstanceBuffer };
		pd3dDevice->IASetVertexBuffers( 0, 2, pVB, strides, offsets );
		pd3dDevice->IASetIndexBuffer( pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

		// batched rendering
		D3D10_TECHNIQUE_DESC techDesc;
		g_pBatchedSquareTechnique->GetDesc( &techDesc );
		for( UINT p = 0; p < techDesc.Passes; ++p )
		{
			g_pBatchedSquareTechnique->GetPassByIndex( p )->Apply( 0 );
			g_pSquare->DrawSubsetInstanced( 0, g_MarchingCubes.m_Vertices.size(), 0 );
		}

		SAFE_RELEASE( pVertexBuffer );
		SAFE_RELEASE( pIndexBuffer );
	}

    //
    // Render the UI
    //
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );

    RenderText();
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D10ReleasingSwapChain();

	SAFE_RELEASE( g_pDepthStencil );
	SAFE_RELEASE( g_pDSState );
	SAFE_RELEASE( g_pDSView );
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D10DestroyDevice();
    g_D3DSettingsDlg.OnD3D10DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pSprite );
    SAFE_DELETE(  g_pTxtHelper );
    SAFE_RELEASE( g_pVertexLayout );
    SAFE_RELEASE( g_pEffect );
	SAFE_RELEASE( g_pSphere );
	SAFE_RELEASE( g_pSquare );
	SAFE_RELEASE( g_pSphereInstanceBuffer );
	SAFE_RELEASE( g_pSquareInstanceBuffer );
	SAFE_RELEASE( g_pBatchedVertexLayout );
	SAFE_RELEASE( g_pMCVertexBuffer );
	SAFE_RELEASE( g_pSolid );
	SAFE_RELEASE( g_pWire );
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	// disable vsync
	pDeviceSettings->d3d10.SyncInterval = 0;
    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );

    //
    // Update variables that change once per frame
    //
    g_pProjectionVariable->SetMatrix( ( float* )g_Camera.GetProjMatrix() );
    g_pViewVariable->SetMatrix( ( float* )g_Camera.GetViewMatrix() );
    g_pWorldVariable->SetMatrix( ( float* )&g_World );
	g_pCameraPosition->SetFloatVector( ( float* ) g_Camera.GetEyePt() );

    // Lighting
    D3DXVECTOR4 vLightDirs[3] =
    {
        D3DXVECTOR4( 0.0f, -1.0f, 0.0f, 1.0f ),
        D3DXVECTOR4( -1.0f, -1.0f, 0.0f, 1.0f ),
		D3DXVECTOR4( 0.0f, -1.0f, -1.0f, 1.0f )
    };
    D3DXVECTOR4 vLightColors[3] =
    {
        D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ),
        D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ),
		D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f )
    };

    g_pLightDirVariable->SetFloatVectorArray( ( float* )vLightDirs, 0, 3 );
    g_pLightColorVariable->SetFloatVectorArray( ( float* )vLightColors, 0, 3 );

	D3DXVECTOR4 vAmbientColor = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_pAmbientColor->SetFloatVector( (float *) vAmbientColor );
	g_pSpecPower->SetFloat( 4.0f );	

	// Update the SPH simulation
	if( g_bPause == false )
		g_ParticleSystem.Step( fElapsedTime );
	
	std::vector<Particle> particles;
	particles = g_ParticleSystem.GetParticles();

	// if renderingmode == spheres
	if( g_bRenderingMode == true )
	{
		// Get Instanced data from particle system per each particle
		g_pSphereInstanceBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&g_pInstances );

		std::vector<Particle>::iterator i;
		unsigned j = 0;
		for( i = particles.begin(); i != particles.end(); ++i, ++j )
		{
			g_pInstances[j].color = i->m_vColor;
			g_pInstances[j].world = i->m_mWorld;
		}

		g_pSphereInstanceBuffer->Unmap();
	}
	else //if rendering mode == marching cubes
	{
		// build a marching cubes isosurface
		std::vector<MarchingCubesVertex> mcResult;
		mcResult = g_MarchingCubes.RunMarchingCubes( particles, (float) g_uTargetIso );
		
		// put the isosurface in the directX 10 vertex buffer
		g_pMCVertexBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void ** )&g_pMCVerts );
		
		std::vector<MarchingCubesVertex>::iterator i;
		unsigned j = 0;
		for( i = mcResult.begin(); i != mcResult.end(); ++i, ++j )
		{
			g_pMCVerts[j].pos = i->pos;
			g_pMCVerts[j].norm = i->norm;
		}

		g_pMCVertexBuffer->Unmap();

		g_uVertexCount = mcResult.size();
	}	


	if( g_bIsogrid == true )
	{
		g_pSquareInstanceBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&g_pSquareInstances );

		D3DXMATRIX	world;
		std::vector<vertex>::iterator i;
		unsigned j = 0;
		for( i = g_MarchingCubes.m_Vertices.begin(); i != g_MarchingCubes.m_Vertices.end(); ++i, ++j )
		{
			D3DXMatrixTranslation( &world, i->pos.x, i->pos.y, i->pos.z );

			g_pSquareInstances[j].world = world;
			g_pSquareInstances[j].color.x = ( i->flux / 300.0f ); 
		}

		g_pSquareInstanceBuffer->Unmap();
	}
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
	
	// Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
		// pause sim
		case VK_SPACE:
		case VK_RETURN:
			{
				if( g_bPause == true )
					g_bPause = false;
				else
					g_bPause = true;

				break;
			}
		case 'R':
			{
				// pause the sim
				g_bPause = true;
				InitParticles();
				break;
			}
        default:
			{
				break;
			}
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
	case	IDC_RENDER:
		{
			switch (g_SampleUI.GetComboBox(IDC_RENDER)->GetSelectedIndex())
			{
			// sphere
			case 0:
				{
					g_bRenderingMode = true;
					break;
				}
			// marching cubes
			case 1:
				{
					g_bRenderingMode = false;
					break;
				}
			};
			break;
		}
	case IDC_ISO:
		{
			g_uTargetIso = (unsigned) g_SampleUI.GetSlider(IDC_ISO)->GetValue();
			break;
		}
	case IDC_RSSTATE:
		{
			switch (g_SampleUI.GetComboBox(IDC_RSSTATE)->GetSelectedIndex())
			{
			// solid
			case 0:
				{
					g_bWire = false;
					break;
				}
			// wire
			case 1:
				{
					g_bWire = true;
					break;
				}
			};
			break;
		}
	case IDC_ISOGRID:
		{
			if( g_SampleUI.GetCheckBox( IDC_ISOGRID )->GetChecked() == true )
				g_bIsogrid = true;
			else
				g_bIsogrid = false;

			break;
		}
	default:
		{
			break;
		}
    }
}
