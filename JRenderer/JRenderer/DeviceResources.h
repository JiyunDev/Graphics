//**********************************************************************
//
// DeviceResources.h - a wrapper for DX3d11 Device and the swapchain 
//
//**********************************************************************/

#pragma once

namespace DX
{
	struct IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};

	//Controll all the DX11 Device resources

	class DeviceResource
	{
	public:

		static const unsigned int c_AllowTearing = 0x1;
		static const unsigned int c_EnableHDR = 0x2;

		DeviceResource(
			DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
			//Depth 24 bit (float), stencil 8 bit(uint)
			DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
			UINT BackBufferCount = 2,
			D3D_FEATURE_LEVEL MinFeatureLevel = D3D_FEATURE_LEVEL_9_3,
			unsigned int flags = 0
		) noexcept;

		void CreateDeviceResources();
		void CreateWindowSizeDependentResources();

		void SetWindow(IUnknown* Window, int Height, int Width, DXGI_MODE_ROTATION Rotation);
		bool OnWindowSizeChanged(int Height, int Width, DXGI_MODE_ROTATION Rotation);
		
		void ValidateDevice();
		void HandleDeviceLost();

		void Trim();
		void Present();

		void RegisterDeviceNotify(IDeviceNotify* DeviceNotify) { m_DeviceNotify = DeviceNotify; }
		

		// Device Accessors
		RECT GetOutPutSize() const { return m_Size; }
		DXGI_MODE_ROTATION GetRotation() const { return m_Rotation; }


		// Direct3D Accessors
		ID3D11Device3*          GetD3DDevice() const { return m_D3DDevice.Get(); }
		ID3D11DeviceContext2*   GetD3DDeviceContext() const { return m_D3DDeviceContext.Get(); }
		IDXGISwapChain3*        GetSwapChain() const { return m_SwapChain.Get(); }
		D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return m_D3DFeatureLevel; }
		ID3D11Texture2D*        GetRenderTarget() const { return m_RenderTarget.Get(); }
		ID3D11Texture2D*        GetDepthStencil() const { return m_DepthStencil.Get(); }
		ID3D11RenderTargetView* GetRenderTargetView() const { return m_D3DRenderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_D3DDepthStencilView.Get(); }
		DXGI_FORMAT             GetBackBufferFormat() const { return m_BackBufferFormat; }
		DXGI_FORMAT             GetDepthBufferFormat() const { return m_DepthBufferFormat; }
		D3D11_VIEWPORT          GetScreenViewport() const { return m_ScreenViewport; }
		UINT                    GetBackBufferCount() const { return m_BackBufferCount; }
		DirectX::XMFLOAT4X4     GetOrientationTransform3D() const { return m_OrientationTransform3D; }
		DXGI_COLOR_SPACE_TYPE   GetColorSpace() const { return m_ColorSpace; }
		//unsigned int            GetDeviceOptions() const { return m_options; }


	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void UpdateColorSpace();

		// Direct3D objects
		Microsoft::WRL::ComPtr<IDXGIFactory3> m_DXGIFactory;
		Microsoft::WRL::ComPtr<ID3D11Device3> m_D3DDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext2> m_D3DDeviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;

		// Direct3D rendering objects. Required for 3D.
		Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_RenderTarget;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_DepthStencil;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_D3DRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_D3DDepthStencilView;
		D3D11_VIEWPORT                                  m_ScreenViewport;

		// Direct3D Properties
		DXGI_FORMAT			m_BackBufferFormat;
		DXGI_FORMAT			m_DepthBufferFormat;
		UINT				m_BackBufferCount;
		D3D_FEATURE_LEVEL	m_D3DMinFeatureLevel;

		// Cached device properties.
		IUnknown*			m_Window;
		D3D_FEATURE_LEVEL	m_D3DFeatureLevel;
		DXGI_MODE_ROTATION	m_Rotation;
		DWORD				m_DXGIFactoryFlags;
		RECT				m_Size;

		// Transforms used for display orientation.
		DirectX::XMFLOAT4X4	m_OrientationTransform3D;
		
		// HDR Support
		DXGI_COLOR_SPACE_TYPE m_ColorSpace;

		// DeviceResources options (see flags above)
		unsigned int		m_options;

		// this can be held by Device directly since it owns the DeviceResource
		IDeviceNotify*		m_DeviceNotify;
	};
}