﻿//------------------------------------------------------------
// Scroll_3_1.cpp
// 맵의 스크롤 
// 
//------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#define VIEW_WIDTH			640					// 화면 너비 
#define VIEW_HEIGHT			480					// 화면 높이 
#define PICTURE_WIDTH			1600					// 배경 너비 
#define CAMERA_VEL			10.0f				// 카메라 속도 
#define MAPSIZE_X				21					// 맵 너비 
#define MAPSIZE_Y				12					// 맵 높이 
#define CHIPSIZE				64					// 칩 크기

	float		fCamera_x, fCamera_y;				// 카메라 좌표 
	int			nMapData[MAPSIZE_Y][MAPSIZE_X];		// 맵 데이터 

int DrawMapChip( float x, float y, int nNumber );
												// 맵칩 그리기 

int InitMap( void )								// 처음에 한 번만 호출된다 
{
	fCamera_x = VIEW_WIDTH  / 2.0f;				// 카메라 초기 위치 
	fCamera_y = VIEW_HEIGHT / 2.0f;					// 카메라 초기 위치 

	return 0;
}


int MoveMap( void )								// 매 프레임 호출된다 
{
	// 왼쪽 방향키가 눌렸으면 왼쪽으로
	if ( GetAsyncKeyState( VK_LEFT ) ) {
		fCamera_x -= CAMERA_VEL;
		if ( fCamera_x < VIEW_WIDTH / 2.0f ) {
			fCamera_x = VIEW_WIDTH / 2.0f;
		}
	}
	// 오른쪽 방향키가 눌렸으면 오른쪽으로
	if ( GetAsyncKeyState( VK_RIGHT ) ) {
		fCamera_x += CAMERA_VEL;
		if ( fCamera_x > ( float )( MAPSIZE_X * CHIPSIZE - VIEW_WIDTH / 2.0f ) ) {
			fCamera_x = ( float )( MAPSIZE_X * CHIPSIZE - VIEW_WIDTH / 2.0f );
		}
	}

	// 위 방향키가 눌렸으면 위로 
	if ( GetAsyncKeyState( VK_UP ) ) {
		fCamera_y -= CAMERA_VEL;
		if ( fCamera_y < VIEW_HEIGHT / 2.0f ) {
			fCamera_y = VIEW_HEIGHT / 2.0f;
		}
	}
	// 아래 방향키가 눌렸으면 아래로 
	if ( GetAsyncKeyState( VK_DOWN ) ) {
		fCamera_y += CAMERA_VEL;
		if ( fCamera_y > ( float )( MAPSIZE_Y * CHIPSIZE - VIEW_HEIGHT / 2.0f ) ) {
			fCamera_y = ( float )( MAPSIZE_Y * CHIPSIZE - VIEW_HEIGHT / 2.0f );
		}
	}

//	fBack_x = VIEW_WIDTH / 2.0f - fCamera_x;

	return 0;
}


int DrawMap( void )							// 맵 그리기 
{
	int					i, j;
	float				fMap_x, fMap_y;				// 맵 표시 좌표 
	int					nBaseChip_x, nBaseChip_y;		// 그릴 좌측 상단 칩 번호
	float				fBasePos_x, fBasePos_y;		// 그릴 좌측 상단 칩 좌표
	float				fChipPos_x, fChipPos_y;		// 그릴 현재 칩 좌표 
	int					nChipNum_x, nChipNum_y;		// 그릴 칩 수(가로, 세로)

	fMap_x = VIEW_WIDTH  / 2.0f - fCamera_x;		// 맵 표시 좌표
	fMap_y = VIEW_HEIGHT / 2.0f - fCamera_y;
	nBaseChip_x = ( int )-fMap_x / CHIPSIZE;		// 그릴 좌측 상단 칩 번호 
	nBaseChip_y = ( int )-fMap_y / CHIPSIZE;
	fBasePos_x = fMap_x + nBaseChip_x * CHIPSIZE;	// 그릴 좌측 상단 칩 좌표
	fBasePos_y = fMap_y + nBaseChip_y * CHIPSIZE;
	nChipNum_x = VIEW_WIDTH  / CHIPSIZE + 1 + 1;		// 그릴 칩 수_가로 
	nChipNum_y = VIEW_HEIGHT / CHIPSIZE + 1 + 1;		// 그릴 칩 수_세로 
	fChipPos_y = fBasePos_y;
	for ( i = 0; i < nChipNum_y; i++ ) {
		fChipPos_x = fBasePos_x;
		for ( j = 0; j < nChipNum_x; j++ ) {
			DrawMapChip( fChipPos_x, fChipPos_y,
						 nMapData[nBaseChip_y + i][nBaseChip_x + j] );
			fChipPos_x += CHIPSIZE;
		}
		fChipPos_y += CHIPSIZE;
	}

	return 0;
}


//------------------------------------------------------------
// 이하 DirectX에 의한 표시 프로그램 

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>								//  Unicode 멀티 바이트 문자 관계

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>


#define MAX_BUFFER_VERTEX				20000	// 최대 버퍼 정점 수


// 링크 라이브러리 
#pragma comment( lib, "d3d11.lib" )    // D3D11라이브러리 
#pragma comment( lib, "d3dx11.lib" )


// 세이프 릴리스 매크로
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release(); ( p )=NULL; } }
#endif


// 정점 구조체 
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
    XMFLOAT4	v4Color;
	XMFLOAT2	v2UV;
};

// 셰이더 구조체 
struct CBNeverChanges
{
    XMMATRIX mView;
};

// 텍스처 그림 구조체 
struct TEX_PICTURE {
	ID3D11ShaderResourceView	*pSRViewTexture;
	D3D11_TEXTURE2D_DESC		tdDesc;
	int							nWidth, nHeight;
};


// 글로벌 변수 
UINT  g_nClientWidth;							// 그릴 영역 너비 
UINT  g_nClientHeight;							// 그릴 영역 높이

HWND        g_hWnd;								// 윈도우 핸들 


ID3D11Device				*g_pd3dDevice;		// 디바이스 
IDXGISwapChain			*g_pSwapChain;		// DXGI스왑체인 
ID3D11DeviceContext		*g_pImmediateContext;	// 디바이스 컨텍스트 
ID3D11RasterizerState		*g_pRS;				// 래스터라이저 
ID3D11RenderTargetView		*g_pRTV;				// 렌더링 타깃 
D3D_FEATURE_LEVEL       	g_FeatureLevel;		// 피처 레벨 

ID3D11Buffer			*g_pD3D11VertexBuffer;
ID3D11BlendState		*g_pbsAlphaBlend;
ID3D11VertexShader		*g_pVertexShader;
ID3D11PixelShader		*g_pPixelShader;
ID3D11InputLayout		*g_pInputLayout;
ID3D11SamplerState		*g_pSamplerState;

ID3D11Buffer			*g_pCBNeverChanges = NULL;

TEX_PICTURE				g_tBack;

// 그릴 정점 버퍼 
CUSTOMVERTEX g_cvVertices[MAX_BUFFER_VERTEX];
int							g_nVertexNum = 0;
ID3D11ShaderResourceView	*g_pNowTexture = NULL;


// Direct3D 초기화 
HRESULT InitD3D( void )
{
    HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL  FeatureLevelsRequested[6] = { D3D_FEATURE_LEVEL_11_0,
													 D3D_FEATURE_LEVEL_10_1,
													 D3D_FEATURE_LEVEL_10_0,
													 D3D_FEATURE_LEVEL_9_3,
													 D3D_FEATURE_LEVEL_9_2,
													 D3D_FEATURE_LEVEL_9_1 };
	UINT               numLevelsRequested = 6;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	// 디바이스 작성 
	hr = D3D11CreateDevice( NULL,
					D3D_DRIVER_TYPE_HARDWARE, 
					NULL, 
					0,
					FeatureLevelsRequested, 
					numLevelsRequested,
					D3D11_SDK_VERSION, 
					&g_pd3dDevice,
					&FeatureLevelsSupported,
					&g_pImmediateContext );
	if( FAILED ( hr ) ) {
		return hr;
	}

	// 팩토리 취득 
	IDXGIDevice * pDXGIDevice;
	hr = g_pd3dDevice->QueryInterface( __uuidof( IDXGIDevice ), ( void ** )&pDXGIDevice );
	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), ( void ** )&pDXGIAdapter );
	IDXGIFactory * pIDXGIFactory;
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), ( void ** )&pIDXGIFactory);

	// 스왑체인 생성 
    DXGI_SWAP_CHAIN_DESC	sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_nClientWidth;
	sd.BufferDesc.Height = g_nClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	hr = pIDXGIFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );

	pDXGIDevice->Release();
	pDXGIAdapter->Release();
	pIDXGIFactory->Release();

	if( FAILED ( hr ) ) {
		return hr;
	}

    // 렌더링 타깃 생성 
    ID3D11Texture2D			*pBackBuffer = NULL;
    D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't get backbuffer." ), _T( "Error" ), MB_OK );
        return hr;
    }
    pBackBuffer->GetDesc( &BackBufferSurfaceDesc );
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRTV );
    SAFE_RELEASE( pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create render target view." ), _T( "Error" ), MB_OK );
        return hr;
    }

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRTV, NULL );

    // 래스터라이저 설정 
    D3D11_RASTERIZER_DESC drd;
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_NONE;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
    g_pImmediateContext->RSSetState( g_pRS );

    // 뷰포트 설정
    D3D11_VIEWPORT vp;
    vp.Width    = ( FLOAT )g_nClientWidth;
    vp.Height   = ( FLOAT )g_nClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    return S_OK;
}


// 프로그래머블 셰이더 작성 
HRESULT MakeShaders( void )
{
    HRESULT hr;
    ID3DBlob* pVertexShaderBuffer = NULL;
    ID3DBlob* pPixelShaderBuffer = NULL;
    ID3DBlob* pError = NULL;

    DWORD dwShaderFlags = 0;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    // 컴파일 
    hr = D3DX11CompileFromFile( _T( "Basic_2D.fx" ), NULL, NULL, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pVertexShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_2D.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DX11CompileFromFile( _T( "Basic_2D.fx" ), NULL, NULL, "PS", "ps_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pPixelShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pError );
        return hr;
    }
    SAFE_RELEASE( pError );
    
    // VertexShader 작성 
    hr = g_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
										   pVertexShaderBuffer->GetBufferSize(),
										   NULL, &g_pVertexShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }
    // PixelShader 작성 
    hr = g_pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
										  pPixelShaderBuffer->GetBufferSize(),
										  NULL, &g_pPixelShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }

    // 입력 버퍼 입력 형식 
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );
	// 입력 버퍼 입력 형식 작성 
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements,
										  pVertexShaderBuffer->GetBufferPointer(),
										  pVertexShaderBuffer->GetBufferSize(),
										  &g_pInputLayout );
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    //  셰이더 상수 버퍼 작성
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( CBNeverChanges );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;

	// 변환행렬 
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mScreen;
    mScreen = XMMatrixIdentity();
	mScreen._11 =  2.0f / g_nClientWidth;
	mScreen._22 = -2.0f / g_nClientHeight;
	mScreen._41 = -1.0f;
	mScreen._42 =  1.0f;
	cbNeverChanges.mView = XMMatrixTranspose( mScreen );
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    return S_OK;
}


// 텍스처 로드
int LoadTexture( TCHAR *szFileName, TEX_PICTURE *pTexPic )
{
    HRESULT						hr;
	D3DX11_IMAGE_INFO			ImageInfo;
	D3DX11_IMAGE_LOAD_INFO		liLoadInfo;
	ID3D11Texture2D				*pTexture;
	int							nTexWidth, nTexHeight;

	// 이미지 정보 취득 
	hr = D3DX11GetImageInfoFromFile( szFileName, NULL, &ImageInfo, NULL );
    if ( FAILED( hr ) ) {
        return hr;
    }
	nTexWidth  = ( int )pow( 2.0, floor( log( ( double )ImageInfo.Width )  / log( 2.0 ) + 1.01 ) );
	if ( ( nTexWidth  / 2 ) == ImageInfo.Width  ) nTexWidth  /= 2;
	nTexHeight = ( int )pow( 2.0, floor( log( ( double )ImageInfo.Height ) / log( 2.0 ) + 1.01 ) );
	if ( ( nTexHeight / 2 ) == ImageInfo.Height ) nTexHeight /= 2;

	// 로드 
	ZeroMemory( &liLoadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
	liLoadInfo.Width = nTexWidth;
	liLoadInfo.Height = nTexHeight;
	liLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	liLoadInfo.Format = ImageInfo.Format;

	hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, szFileName, &liLoadInfo, NULL, &( pTexPic->pSRViewTexture ), NULL );
    if ( FAILED( hr ) ) {
        return hr;
    }
	pTexPic->pSRViewTexture->GetResource( ( ID3D11Resource ** )&( pTexture ) );
	pTexture->GetDesc( &( pTexPic->tdDesc ) );
	pTexture->Release();

	pTexPic->nWidth = ImageInfo.Width;
	pTexPic->nHeight = ImageInfo.Height;

	return S_OK;
}


// 그리기 모드 오브젝트 초기화 
int InitDrawModes( void )
{
    HRESULT				hr;

	// 블렌드 스테이터스 
    D3D11_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
    BlendDesc.RenderTarget[0].BlendEnable           = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState( &BlendDesc, &g_pbsAlphaBlend );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // 샘플러 
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof( samDesc ) );
    samDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &samDesc, &g_pSamplerState );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}


// 지오메트리 초기화
HRESULT InitGeometry( void )
{
    HRESULT hr = S_OK;

    //정점 버퍼 작성 
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( CUSTOMVERTEX ) * MAX_BUFFER_VERTEX;
    BufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA SubResourceData;
    SubResourceData.pSysMem             = g_cvVertices;
    SubResourceData.SysMemPitch         = 0;
    SubResourceData.SysMemSlicePitch    = 0;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pD3D11VertexBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

	// 텍스처 작성 
	g_tBack.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "9.bmp" ), &g_tBack );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 9.bmp" ), _T( "Error" ), MB_OK );
		return hr;
    }

	return S_OK;
}


// 맵 데이터 로드 
int LoadMapData( void )
{
	int				i, j;
	FILE			*fp;
	char			szLine[MAPSIZE_X * 3 + 3];
	char			*pReadPoint;

	if ( ( fp = fopen( "MapData02.txt", "r" ) ) == NULL ) {
	 	MessageBox( NULL, _T( "Can't open MapData02.txt" ), _T( "Error" ), MB_OK );
		return -1;
	}
	
	for ( i = 0; i < MAPSIZE_Y; i++ ) {
		fgets( szLine, MAPSIZE_X * 3 + 2, fp );
		pReadPoint = szLine;
		for ( j = 0; j < MAPSIZE_X; j++ ) {
			nMapData[i][j] = atoi( pReadPoint );
			pReadPoint = strchr( pReadPoint + 1, ' ' );
		}
	}

	fclose( fp );

	return 0;
}


// 종료처리 
int Cleanup( void )
{
    SAFE_RELEASE( g_tBack.pSRViewTexture );
    SAFE_RELEASE( g_pD3D11VertexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAlphaBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// 래스터라이저

	// 스테이터스  클리어 
	if ( g_pImmediateContext ) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Flush();
	}

    SAFE_RELEASE( g_pRTV );									// 렌더링 타깃

    // 스왑체인
    if ( g_pSwapChain != NULL ) {
        g_pSwapChain->SetFullscreenState( FALSE, 0 );
    }
    SAFE_RELEASE( g_pSwapChain );

    SAFE_RELEASE( g_pImmediateContext );					// 디바이스 컨텍스트
    SAFE_RELEASE( g_pd3dDevice );							// 디바이스

	return 0;
}


// 윈도 프로시저 
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


// 그림 그리기 대기 행렬 플러시 
int FlushDrawingPictures( void )
{
	HRESULT			hr;

	if ( ( g_nVertexNum > 0 ) && g_pNowTexture ) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hr = g_pImmediateContext->Map( g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		if ( SUCCEEDED( hr ) ) {
			CopyMemory( mappedResource.pData, &( g_cvVertices[0] ), sizeof( CUSTOMVERTEX ) * g_nVertexNum );
			g_pImmediateContext->Unmap( g_pD3D11VertexBuffer, 0 );
		}
		g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pNowTexture );
		g_pImmediateContext->Draw( g_nVertexNum, 0 );
	}
	g_nVertexNum = 0;
	g_pNowTexture = NULL;

	return 0;
}


// 그림 그리기
int DrawPicture( float x, float y, TEX_PICTURE *pTexPic )
{
	if ( g_nVertexNum > ( MAX_BUFFER_VERTEX - 6 ) ) return -1;	// 정점 버퍼에서 넘치면 그리지 않고 

	// 텍스처가 바뀌면 대기 행렬 플러시 
	if ( ( pTexPic->pSRViewTexture != g_pNowTexture ) && g_pNowTexture ) {
		FlushDrawingPictures();
	}

	// 정점 세트 
	g_cvVertices[g_nVertexNum + 0].v4Pos   = XMFLOAT4( x,                   y,                    0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v2UV    = XMFLOAT2( 0.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 1].v4Pos   = XMFLOAT4( x,                   y + pTexPic->nHeight, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV    = XMFLOAT2( 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Pos   = XMFLOAT4( x + pTexPic->nWidth, y,                    0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV    = XMFLOAT2( 1.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 3] = g_cvVertices[g_nVertexNum + 1];
	g_cvVertices[g_nVertexNum + 4].v4Pos   = XMFLOAT4( x + pTexPic->nWidth, y + pTexPic->nHeight, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v2UV    = XMFLOAT2( 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 5] = g_cvVertices[g_nVertexNum + 2];
	g_nVertexNum += 6;
	g_pNowTexture = pTexPic->pSRViewTexture;

	return 0;
}


// 맵칩 그리기 
int DrawMapChip( float x, float y, int nNumber )
{
	float			tu1, tv1;
	float			tu2, tv2;

	if ( g_nVertexNum > ( MAX_BUFFER_VERTEX - 6 ) ) return -1;	// 정점 버퍼에서 넘치면 그리지 않고

	// 텍스처가 바뀌면 대기 행렬 플러시 
	if ( ( g_tBack.pSRViewTexture != g_pNowTexture ) && g_pNowTexture ) {
		FlushDrawingPictures();
	}

	// 정점 세트 
	tu1 = ( nNumber & 3  ) * 0.25f;
	tv1 = ( nNumber >> 2 ) * 0.25f;
	tu2 = tu1 + 0.25f;
	tv2 = tv1 + 0.25f;
	g_cvVertices[g_nVertexNum + 0].v4Pos   = XMFLOAT4( x,                         y,                          0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v2UV    = XMFLOAT2( tu1, tv1 );
	g_cvVertices[g_nVertexNum + 1].v4Pos   = XMFLOAT4( x,                         y + g_tBack.nHeight / 4.0f, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV    = XMFLOAT2( tu1, tv2 );
	g_cvVertices[g_nVertexNum + 2].v4Pos   = XMFLOAT4( x + g_tBack.nWidth / 4.0f, y,                          0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV    = XMFLOAT2( tu2, tv1 );
	g_cvVertices[g_nVertexNum + 3] = g_cvVertices[g_nVertexNum + 1];
	g_cvVertices[g_nVertexNum + 4].v4Pos   = XMFLOAT4( x + g_tBack.nWidth / 4.0f, y + g_tBack.nHeight / 4.0f, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v2UV    = XMFLOAT2( tu2, tv2 );
	g_cvVertices[g_nVertexNum + 5] = g_cvVertices[g_nVertexNum + 2];
	g_nVertexNum += 6;
	g_pNowTexture = g_tBack.pSRViewTexture;

	return 0;
}



// 렌더링
HRESULT Render( void )
{
    // 화면 클리어 
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f );
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, ( float * )&v4Color );

    // 샘플러 래스터라이저 세트
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    g_pImmediateContext->RSSetState( g_pRS );
    
    //그리기 설정 
    UINT nStrides = sizeof( CUSTOMVERTEX );
    UINT nOffsets = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pD3D11VertexBuffer, &nStrides, &nOffsets );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetInputLayout( g_pInputLayout );

    // 셰이더 설정
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

    // 그리기 
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	DrawMap();
    g_pImmediateContext->OMSetBlendState( g_pbsAlphaBlend, NULL, 0xFFFFFFFF );

    // 표시
	FlushDrawingPictures();

    return S_OK;
}


// 엔트리 포인트 
int WINAPI _tWinMain( HINSTANCE hInst, HINSTANCE, LPTSTR, int )
{
	LARGE_INTEGER			nNowTime, nLastTime;		// 현재와 직전의 시각 
	LARGE_INTEGER			nTimeFreq;					// 시간단위

    // 화면 크기 
    g_nClientWidth  = VIEW_WIDTH;						// 너비 
    g_nClientHeight = VIEW_HEIGHT;						// 높이

	// Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
                      _T( "D3D Sample" ), NULL };
    RegisterClassEx( &wc );

	RECT rcRect;
	SetRect( &rcRect, 0, 0, g_nClientWidth, g_nClientHeight );
	AdjustWindowRect( &rcRect, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "Scroll_3_1" ),
						   WS_OVERLAPPEDWINDOW, 100, 20, rcRect.right - rcRect.left, rcRect.bottom - rcRect.top,
						   GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D() ) && SUCCEEDED( MakeShaders() ) )
    {
        // Create the shaders
        if( SUCCEEDED( InitDrawModes() ) )
        {
			if ( SUCCEEDED( InitGeometry() ) && ( LoadMapData() >= 0 ) ) {	// 지오메트리 작성

				// Show the window
				ShowWindow( g_hWnd, SW_SHOWDEFAULT );
				UpdateWindow( g_hWnd );

				InitMap();									// 캐릭터 초기화
				
				QueryPerformanceFrequency( &nTimeFreq );			//시간단위 
				QueryPerformanceCounter( &nLastTime );			// 1프레임 전 시각 초기화

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while( msg.message != WM_QUIT )
				{
					Render();
					MoveMap();
					do {
						if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
						{
							TranslateMessage( &msg );
							DispatchMessage( &msg );
						}
						QueryPerformanceCounter( &nNowTime );
					} while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 90 ) ) &&
							 ( msg.message != WM_QUIT ) );
					while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 60 ) ) &&
						   ( msg.message != WM_QUIT ) )
					{
						QueryPerformanceCounter( &nNowTime );
					}
					nLastTime = nNowTime;
					g_pSwapChain->Present( 0, 0 );					// 표시 
				}
			}
        }
    }

    // Clean up everything and exit the app
    Cleanup();
    UnregisterClass( _T( "D3D Sample" ), wc.hInstance );
    return 0;
}
