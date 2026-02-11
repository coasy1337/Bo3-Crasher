#pragma once
#include <Include.h>

class Hooking {
public:
	Hooking( );
	~Hooking( ) = default;

	static HRESULT DetourPresent( IDXGISwapChain* swapchain, UINT syncInterval, UINT flags );

	bool IntializeRenderingEnviroment( IDXGISwapChain* swapchain );
	void Uninitialize( );
	void BeginImGuiFrame( );
	void EndImGuiFrame( );

	WNDPROC m_OriginalWndProc = nullptr;
private:
	typedef HRESULT( __fastcall* tPresent )(IDXGISwapChain*, UINT, UINT);
	tPresent m_oPresent;

	auto InitializeImGui( ) -> bool;

	HWND m_Window = 0;
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11Texture2D* m_pBackBuffer = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
};

inline std::unique_ptr<Hooking> g_Hooking;