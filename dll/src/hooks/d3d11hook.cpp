#include <framework/stdafx.h>
#include "d3d11hook.h"
#include <kiero.h>
#include <dxgi.h>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
// Embedded fonts
#include "../dev/imgui/tahoma.h"
#include "../dev/imgui/fonts/fa_solid_900.h"
#include "../dev/imgui/IconsFontAwesome5.h"

#include <includes.h>
#include "../gui/CheatMenu.h"

#pragma comment(lib, "d3d11.lib")

// D3D11 method indices (kiero) for Present and ResizeBuffers on IDXGISwapChain
// See kiero docs: D3D11 Present = 8, ResizeBuffers = 13
using PresentFunc = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
static PresentFunc oPresent11 = nullptr;
using ResizeBuffersFunc = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
static ResizeBuffersFunc oResizeBuffers11 = nullptr;

static ID3D11Device*           g_d3dDevice = nullptr;
static ID3D11DeviceContext*    g_d3dContext = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static IDXGISwapChain*         g_swapChain11 = nullptr;
static HWND                    g_hWnd11 = nullptr;
static WNDPROC                 g_oWndProc11 = nullptr;

static void CreateRenderTarget11()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(g_swapChain11->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
    {
        g_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget11()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT APIENTRY WndProc11(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    if (CheatMenu::IsVisible())
    {
        switch (uMsg)
        {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP:
        case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
        case WM_MOUSEMOVE:
            return true;
        default: break;
        }
    }
    return CallWindowProc(g_oWndProc11, hwnd, uMsg, wParam, lParam);
}

static void InitImGui11()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(g_hWnd11);
    ImGui_ImplDX11_Init(g_d3dDevice, g_d3dContext);

    // Fonts (embedded)
    ImFontConfig baseCfg; baseCfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)tahoma, sizeof(tahoma), 16.0f, &baseCfg);
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig iconCfg; iconCfg.MergeMode = true; iconCfg.PixelSnapH = true; iconCfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)fa_solid_900, sizeof(fa_solid_900), 16.0f, &iconCfg, icon_ranges);
    io.Fonts->Build();
}

static HRESULT __stdcall hkPresent11(IDXGISwapChain* pSwap, UINT SyncInterval, UINT Flags)
{
    static bool init = false;
    if (!init)
    {
        if (SUCCEEDED(pSwap->GetDevice(__uuidof(ID3D11Device), (void**)&g_d3dDevice)))
        {
            g_d3dDevice->GetImmediateContext(&g_d3dContext);
            g_swapChain11 = pSwap;

            DXGI_SWAP_CHAIN_DESC sd{};
            pSwap->GetDesc(&sd);
            g_hWnd11 = sd.OutputWindow;

            CreateRenderTarget11();

            g_oWndProc11 = (WNDPROC)SetWindowLongPtr(g_hWnd11, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc11);

            InitImGui11();

            Cheat::Core::CheatMain::Initialize();
            init = true;
        }
        return oPresent11(pSwap, SyncInterval, Flags);
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
        CheatMenu::Toggle();

    // New ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = CheatMenu::IsVisible();

    CheatMenu::Render();

    // Cheat update once per frame
    Cheat::Core::CheatMain::Update(GetTickCount());

    // Render
    ImGui::Render();
    g_d3dContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent11(pSwap, SyncInterval, Flags);
}

static HRESULT __stdcall hkResizeBuffers11(IDXGISwapChain* pSwap, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (!g_d3dDevice)
        return oResizeBuffers11(pSwap, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    ImGui_ImplDX11_InvalidateDeviceObjects();
    CleanupRenderTarget11();

    HRESULT hr = oResizeBuffers11(pSwap, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (SUCCEEDED(hr))
    {
        CreateRenderTarget11();
        ImGui_ImplDX11_CreateDeviceObjects();
    }
    return hr;
}

bool InitD3D11Hook()
{
    LOG_INFO("Initializing D3D11 hook...");
    try
    {
        auto st = kiero::init(kiero::RenderType::D3D11);
        if (st != kiero::Status::Success)
        {
            LOG_ERROR("kiero D3D11 init failed");
            return false;
        }

        bool ok = true;
        if (kiero::bind(8, (void**)&oPresent11, hkPresent11) != kiero::Status::Success) { LOG_ERROR("Failed to hook Present (D3D11)"); ok = false; }
        if (kiero::bind(13, (void**)&oResizeBuffers11, hkResizeBuffers11) != kiero::Status::Success) { LOG_ERROR("Failed to hook ResizeBuffers (D3D11)"); ok = false; }

        if (!ok)
        {
            kiero::shutdown();
            return false;
        }
        LOG_INFO("D3D11 successfully hooked using kiero");
        return true;
    }
    catch (...)
    {
        LOG_ERROR("Exception during D3D11 hook init");
        kiero::shutdown();
        return false;
    }
}

void ReleaseD3D11Hook()
{
    kiero::shutdown();

    if (g_d3dDevice)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    CleanupRenderTarget11();

    if (g_d3dContext) { g_d3dContext->Release(); g_d3dContext = nullptr; }
    if (g_d3dDevice)  { g_d3dDevice->Release();  g_d3dDevice  = nullptr; }

    if (g_oWndProc11 && g_hWnd11)
    {
        SetWindowLongPtr(g_hWnd11, GWLP_WNDPROC, (LONG_PTR)g_oWndProc11);
        g_oWndProc11 = nullptr;
    }

    g_swapChain11 = nullptr;
    g_hWnd11 = nullptr;
}

