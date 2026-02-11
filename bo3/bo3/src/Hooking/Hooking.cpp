#include "Hooking.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT __stdcall WndProc( const HWND wnd, UINT msg, WPARAM param, LPARAM lparam )
{
	if (GetActiveWindow( ) == nullptr)
		return CallWindowProc( g_Hooking->m_OriginalWndProc, wnd, msg, param, lparam );

	if (ImGui_ImplWin32_WndProcHandler( wnd, msg, param, lparam ) && g_UserInterface->m_IsOpened)
		return 1L;

	return CallWindowProc( g_Hooking->m_OriginalWndProc, wnd, msg, param, lparam );
}

Hooking::Hooking( )
{
	uintptr_t m_PresentFuncAddr = g_Memory->FindPattern( "GameOverlayRenderer64.dll", "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B E8" );
	uintptr_t m_CreateHookAddr = g_Memory->FindPattern( "GameOverlayRenderer64.dll", "48 89 5C 24 ? 57 48 83 EC ? 33 C0" );

	using SteamCreateHook_t = __int64( __fastcall* )(unsigned __int64, __int64, unsigned __int64*, int, const char*);
	auto SteamCreateHook = reinterpret_cast<SteamCreateHook_t>(m_CreateHookAddr);

	SteamCreateHook(
		(unsigned __int64)m_PresentFuncAddr,
		(__int64)&DetourPresent,
		(unsigned __int64*)&m_oPresent,
		1,
		("DXGISwapChain_Present")
	);
}

inline std::once_flag intializeFlag;
HRESULT Hooking::DetourPresent( IDXGISwapChain* swapchain, UINT syncInterval, UINT flags )
{
	std::call_once( intializeFlag, [swapchain] { g_Hooking->IntializeRenderingEnviroment( swapchain ); } );

	g_Hooking->BeginImGuiFrame( );
	{
		g_UserInterface->Render( );
	}
	g_Hooking->EndImGuiFrame( );

	return g_Hooking->m_oPresent( swapchain, syncInterval, flags );
}

bool Hooking::IntializeRenderingEnviroment( IDXGISwapChain* swapchain )
{
	this->m_pSwapChain = swapchain;
	if (FAILED( swapchain->GetDevice( __uuidof(ID3D11Device), reinterpret_cast<void**>(&this->m_pDevice) ) ))
		return false;

	this->m_pDevice->GetImmediateContext( &this->m_pDeviceContext );

	DXGI_SWAP_CHAIN_DESC desc;
	this->m_pSwapChain->GetDesc( &desc );
	this->m_Window = desc.OutputWindow;

	this->m_pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&this->m_pBackBuffer) );
	if (!this->m_pBackBuffer) return false;

	this->m_pDevice->CreateRenderTargetView( this->m_pBackBuffer, nullptr, &m_pRenderTargetView );
	this->m_pBackBuffer->Release( );

	this->m_OriginalWndProc = (WNDPROC)SetWindowLongPtrA( this->m_Window, GWLP_WNDPROC, (LONG_PTR)WndProc );

	this->InitializeImGui( );

	g_Logger->Log( Success, "Rendering Enviroment Created!" );

	return true;
}

bool Hooking::InitializeImGui( )
{
	IMGUI_CHECKVERSION( );

	ImGui::CreateContext( );

	const auto& io = ImGui::GetIO( );

	ImGui::StyleColorsDark( );

	ImGui_ImplWin32_Init( this->m_Window );
	ImGui_ImplDX11_Init( this->m_pDevice, this->m_pDeviceContext );

	return true;
}

void Hooking::BeginImGuiFrame( )
{
	ID3D11Texture2D* renderTargetTexture = nullptr;
	if (!this->m_pRenderTargetView)
	{
		if (FAILED( this->m_pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&renderTargetTexture) ) )) return;

		this->m_pDevice->CreateRenderTargetView( renderTargetTexture, nullptr, &this->m_pRenderTargetView );
		renderTargetTexture->Release( );
	}

	ImGui_ImplDX11_NewFrame( );
	ImGui_ImplWin32_NewFrame( );
	ImGui::NewFrame( );
}

void Hooking::EndImGuiFrame( )
{
	this->m_pDeviceContext->OMSetRenderTargets( 1, &this->m_pRenderTargetView, 0 );
	ImGui::Render( );
	if (this->m_pRenderTargetView)
	{
		this->m_pRenderTargetView->Release( );
		this->m_pRenderTargetView = nullptr;
	}

	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
}

void Hooking::Uninitialize( )
{
	SetWindowLongPtrA( this->m_Window, GWLP_WNDPROC, (LONG_PTR)this->m_OriginalWndProc );
}