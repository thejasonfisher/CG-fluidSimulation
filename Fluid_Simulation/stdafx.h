// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

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
