#include "pch.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace DX;
using Microsoft::WRL::ComPtr;

#if defined(_DEBUG)

/**
To use this D3D11_CREATE_DEVICE_DEBUG flag, you must have D3D11*SDKLayers.dll installed; 
Otherwise, device creation fails. 
*/
inline bool SdkLayersAvailable()
{
	HRESULT Hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL,		// we don't want to create real hardware device
		0,
		D3D11_CREATE_DEVICE_DEBUG,  //to check if we have the sdk layer
		nullptr,					// any feature level will do
		0,
		D3D11_SDK_VERSION,	
		nullptr,					// no need to keep D3D11 Device reference
		nullptr,					// no need to know the feature level
		nullptr);					// no need to keep D3D11 Device Resource Context

	return SUCCEEDED(Hr);
}

#endif //_DEBUG

// Constants used to calculate screen rotations
namespace ScreenRotation
{
	// 0-degree Z-rotation
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 90-degree Z-rotation
	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 180-degree Z-rotation
	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 270-degree Z-rotation
	static const XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
};

DeviceResource::DeviceResource(
	DXGI_FORMAT BackBufferFormat,
	DXGI_FORMAT DepthBufferFormat,
	UINT BackBufferCount,
	D3D_FEATURE_LEVEL MinFeatureLevel,
	unsigned int flags) noexcept :
	m_ScreenViewport{},
	m_BackBufferFormat(BackBufferFormat),
	m_DepthBufferFormat(DepthBufferFormat),
	m_BackBufferCount(BackBufferCount),
	m_D3DMinFeatureLevel(MinFeatureLevel),
	m_Window(nullptr),
	m_D3DFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	m_Rotation(DXGI_MODE_ROTATION_IDENTITY),
	m_DXGIFactoryFlags(0),
	m_Size{0, 0, 1, 1},
	m_OrientationTransform3D(ScreenRotation::Rotation0),
	m_ColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
	m_DeviceNotify(nullptr)
{}


void DeviceResource::CreateDeviceResources()
{
	//All 10level9 and higher hardware with WDDM 1.1+ drivers support BGRA formats.
	UINT CreationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	if (SdkLayersAvailable())
	{
		CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	else
	{
		OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
	}
#endif

#if defined(_DEBUG)
	{
		ComPtr<IDXGIInfoQueue> DXGIInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(DXGIInfoQueue.GetAddressOf()))))
		{
			m_DXGIFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			DXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			DXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(m_DXGIFactoryFlags, IID_PPV_ARGS(m_DXGIFactory.ReleaseAndGetAddressOf())));

	if (m_options & c_AllowTearing)
	{
		BOOL AllowTearing = FALSE;

		ComPtr<IDXGIFactory5> Factory5;
		HRESULT Handle = m_DXGIFactory.As(&Factory5);
		if (SUCCEEDED(Handle))
		{
			Handle = Factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &AllowTearing, sizeof(AllowTearing));
		}

		//take out allow tearing flag
		if (FAILED(Handle) || !AllowTearing)
		{
			m_options &= ~c_AllowTearing;
#ifdef _DEBUG
			OutputDebugStringA("WARNING : Variable refresh rate displays not supported");
#endif
		}
	}

	//Determine DirectX hardware feature levels this app will support

	static const D3D_FEATURE_LEVEL s_FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	UINT FeatureLevelCount = 0;
	for (; FeatureLevelCount < _countof(s_FeatureLevels); ++FeatureLevelCount)
	{
		if (s_FeatureLevels[FeatureLevelCount] < m_D3DMinFeatureLevel)
		{
			break;
		}
	}

	if (!FeatureLevelCount)
	{
		throw std::out_of_range("MinFeautureLevel is too high");
	}

	ComPtr<IDXGIAdapter1> Adapter;
	GetHardwareAdapter(Adapter.GetAddressOf());

	//Create the Direct3D11 API Device object and a corresponding context
	ComPtr<ID3D11Device> Device;
	ComPtr<ID3D11DeviceContext> Context;

	HRESULT Result = E_FAIL;
	if (Adapter)
	{
		Result = D3D11CreateDevice(
			Adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			0,
			CreationFlags,			// Set debug and Direc2D compatibility flags
			s_FeatureLevels,
			FeatureLevelCount,
			D3D11_SDK_VERSION,
			Device.GetAddressOf(), // Returns the Direct3D device created
			&m_D3DFeatureLevel,    // Returns feature level of device create
			Context.GetAddressOf() // Returns the device imediate context
		);
	}
	//wtf is NDEBUG?
#if defined(NDEBUG)
	else
	{
		throw std::exception("No Direct3D hardware device found ")
	}
#else
	if (FAILED(Result))
	{
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		Result = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			0,
			CreationFlags,			// Set debug and Direc2D compatibility flags
			s_FeatureLevels,
			FeatureLevelCount,
			D3D11_SDK_VERSION,
			Device.GetAddressOf(), // Returns the Direct3D device created
			&m_D3DFeatureLevel,    // Returns feature level of device create
			Context.GetAddressOf() // Returns the device imediate context
		);
		if (SUCCEEDED(Result))
		{
			OutputDebugStringA("Direct3D Adapter - WARP\n");
		}
	}
#endif
	ThrowIfFailed(Result);

#ifndef NDEBUG
	ComPtr<ID3D11Debug> D3DDebug;
	if(SUCCEEDED(Device.As(&D3DDebug)))
	{ 
		ComPtr<ID3D11InfoQueue> D3DInfoQueue;
		if (SUCCEEDED(D3DDebug.As(&D3DInfoQueue)))
		{
#ifdef _DEBUG
			D3DInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			D3DInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID Hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(Hide);
			filter.DenyList.pIDList = Hide;
			D3DInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif

	ThrowIfFailed(Device.As(&m_D3DDevice));
	ThrowIfFailed(Context.As(&m_D3DDeviceContext));
}


void DeviceResource::CreateWindowSizeDependentResources()
{
}

void DeviceResource::SetWindow(IUnknown* Window, int Height, int Width, DXGI_MODE_ROTATION Rotation)
{
}

bool DeviceResource::OnWindowSizeChanged(int Height, int Width, DXGI_MODE_ROTATION Rotation)
{
	return true;
}

void DeviceResource::ValidateDevice()
{
}

void DeviceResource::HandleDeviceLost()
{
}

void DeviceResource::Trim()
{
}

void DeviceResource::Present()
{
}