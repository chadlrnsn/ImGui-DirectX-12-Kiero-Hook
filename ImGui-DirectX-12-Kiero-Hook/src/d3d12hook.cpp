#include "d3d12hook.h"
#include <kiero/kiero.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <thread>

// Debug
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

typedef HRESULT(__stdcall *PresentFunc)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
PresentFunc oPresent = nullptr;

typedef void(__stdcall *ExecuteCommandListsFunc)(ID3D12CommandQueue *pCommandQueue, UINT NumCommandLists, ID3D12CommandList *const *ppCommandLists);
ExecuteCommandListsFunc oExecuteCommandLists = nullptr;

typedef HRESULT(__stdcall *ResizeBuffers)(IDXGISwapChain3 *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
ResizeBuffers oResizeBuffers;

typedef HRESULT(__stdcall *SignalFunc)(ID3D12CommandQueue *queue, ID3D12Fence *fence, UINT64 value);
SignalFunc oSignal = nullptr;

HWND window;
WNDPROC oWndProc;

struct FrameContext
{
    ID3D12CommandAllocator *CommandAllocator;
    UINT64 FenceValue; // In imgui original code // i didn't use it
    ID3D12Resource *g_mainRenderTargetResource = {};
    D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor = {};
};

// Data
static int const NUM_FRAMES_IN_FLIGHT = 3;
// static FrameContext*                g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
//  Modified
FrameContext *g_frameContext;
static UINT g_frameIndex = 0;
static UINT g_fenceValue = 0;

// static int const                    NUM_BACK_BUFFERS = 3; // original
static int NUM_BACK_BUFFERS = -1;
static ID3D12Device *g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap *g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap *g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue *g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList *g_pd3dCommandList = nullptr;
static ID3D12Fence *g_fence = nullptr;
static HANDLE g_fenceEvent = nullptr;
static UINT64 g_fenceLastSignaledValue = 0;
static IDXGISwapChain3 *g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static HANDLE g_hSwapChainWaitableObject = nullptr;
// static ID3D12Resource*              g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {}; // Original
// static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {}; // Original
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;

bool show_demo_window = true;
bool bShould_render = true;

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource *pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_frameContext[i].g_mainRenderTargetDescriptor);
        g_frameContext[i].g_mainRenderTargetResource = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        if (g_frameContext[i].g_mainRenderTargetResource)
        {
            g_frameContext[i].g_mainRenderTargetResource->Release();
            g_frameContext[i].g_mainRenderTargetResource = nullptr;
        }
    }
}

void WaitForLastSubmittedFrame()
{
    FrameContext *frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

void InitImGui()
{
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
    ImGui_ImplDX12_CreateDeviceObjects();
}

HRESULT __fastcall hkPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool init = false;

    if (!init)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void **)&g_pd3dDevice)))
        {

            // sDesc
            DXGI_SWAP_CHAIN_DESC sdesc;
            pSwapChain->GetDesc(&sdesc);
            sdesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            sdesc.Windowed = ((GetWindowLongPtr(window, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
            window = sdesc.OutputWindow;

            // sDesc1
            DXGI_SWAP_CHAIN_DESC1 sdesc1;
            pSwapChain->GetDesc1(&sdesc1);

            NUM_BACK_BUFFERS = sdesc.BufferCount;
            g_frameContext = new FrameContext[NUM_BACK_BUFFERS];

            // RTV Descriptor Heap

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
                    return E_FAIL;

                SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

                // Create RenderTargetView

                for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                {
                    g_frameContext[i].g_mainRenderTargetDescriptor = rtvHandle;
                    rtvHandle.ptr += rtvDescriptorSize;
                }
            }

            // SRV Descriptor Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                    return E_FAIL;
            }

            {
                ID3D12CommandAllocator *allocator;
                if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)) != S_OK)
                    return E_FAIL;

                for (size_t i = 0; i < NUM_BACK_BUFFERS; i++)
                {
                    g_frameContext[i].CommandAllocator = allocator;
                }

                if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
                    g_pd3dCommandList->Close() != S_OK)
                    return E_FAIL;
            }

            g_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
            if (g_fenceEvent == nullptr)
                assert(g_fenceEvent == nullptr);

            g_hSwapChainWaitableObject = pSwapChain->GetFrameLatencyWaitableObject();
            g_pSwapChain = pSwapChain;

            CreateRenderTarget();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
            InitImGui();

            // Добавляем поддержку переключения полноэкранного режима
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
            pSwapChain->GetDesc1(&swapChainDesc);

            // Включаем поддержку Alt+Enter и изменения размера
            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
            fullscreenDesc.Windowed = TRUE;
            fullscreenDesc.RefreshRate.Numerator = 60;
            fullscreenDesc.RefreshRate.Denominator = 1;
            fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        }
        init = true;
    }

    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        WaitForLastSubmittedFrame();
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }

    if (!g_pd3dCommandQueue)
    {
        printf("Failed to create command queue\n");
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
        show_demo_window = !show_demo_window;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = show_demo_window;
    if (show_demo_window)
        ImGui::ShowDemoWindow();

    FrameContext &frameCtx = g_frameContext[pSwapChain->GetCurrentBackBufferIndex()];
    frameCtx.CommandAllocator->Reset();

    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_frameContext[backBufferIdx].g_mainRenderTargetResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    g_pd3dCommandList->Reset(frameCtx.CommandAllocator, nullptr);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    g_pd3dCommandList->OMSetRenderTargets(1, &g_frameContext[backBufferIdx].g_mainRenderTargetDescriptor, FALSE, nullptr);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    g_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList *const *>(&g_pd3dCommandList));

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void __fastcall hkExecuteCommandLists(ID3D12CommandQueue *pCommandQueue, UINT NumCommandLists, ID3D12CommandList *const *ppCommandLists)
{
    if (!g_pd3dCommandQueue)
    {
        g_pd3dCommandQueue = pCommandQueue;
    }

    oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT __fastcall hkResizeBuffers(IDXGISwapChain3 *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    ImGui_ImplDX12_InvalidateDeviceObjects();
    CleanupRenderTarget();

    HRESULT result = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (SUCCEEDED(result))
    {
        CreateRenderTarget();
        ImGui_ImplDX12_CreateDeviceObjects();

        // Обновляем размеры при любом изменении разрешения
        DXGI_MODE_DESC modeDesc = {};
        modeDesc.Width = Width;
        modeDesc.Height = Height;
        modeDesc.Format = NewFormat;
        g_pSwapChain->ResizeTarget(&modeDesc);
    }

    return result;
}

HRESULT __fastcall hkSignal(ID3D12CommandQueue *queue, ID3D12Fence *fence, UINT64 value)
{
    if (g_pd3dCommandQueue != nullptr && queue == g_pd3dCommandQueue)
    {
        g_fence = fence;
        g_fenceValue = value;
    }
    return oSignal(queue, fence, value);
    ;
}

void InitD3D12Hook()
{
    static bool init = false;
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success)
        {
            kiero::bind(54, (void **)&oExecuteCommandLists, hkExecuteCommandLists);
            kiero::bind(58, (void **)&oSignal, hkSignal);
            kiero::bind(140, (void **)&oPresent, hkPresent);
            kiero::bind(145, (void **)&oResizeBuffers, hkResizeBuffers);
            std::cout << "window hooked using kiero" << std::endl;
            init = true;
        }

    } while (!init);
}

void ReleaseD3D12Hook()
{
    CleanupRenderTarget();

    if (oWndProc)
    {
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        oWndProc = nullptr;
    }

    if (window)
        window = nullptr;
}