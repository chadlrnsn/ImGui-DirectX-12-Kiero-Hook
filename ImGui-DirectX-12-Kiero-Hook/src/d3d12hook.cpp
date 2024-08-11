#include "d3d12hook.h"
#include <kiero/kiero.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

#define DX12_ENABLE_DEBUG_LAYER     0
typedef HRESULT(__stdcall* PresentFunc)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
PresentFunc oPresent = nullptr;

typedef void(__stdcall* ExecuteCommandListsFunc)(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
ExecuteCommandListsFunc oExecuteCommandLists = nullptr;

typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
ResizeBuffers oResizeBuffers;

typedef HRESULT(__stdcall* SignalFunc)(ID3D12CommandQueue* queue, ID3D12Fence* fence, UINT64 value);
SignalFunc oSignal = nullptr;

HWND window;
WNDPROC oWndProc;

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue; // In imgui original code // i didn't use it

};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
//static FrameContext*                g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
// Modified
FrameContext*                       g_frameContext;
static UINT                         g_frameIndex = 0;

//static int const                    NUM_BACK_BUFFERS = 3; // original
static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12Fence* g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static bool                         g_SwapChainOccluded = false;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

UINT BufferCount = -1;
bool show_demo_window = true;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    //WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_pSwapChain->GetCurrentBackBufferIndex() % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    //g_fence->SetEventOnCompletion(g_fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

//FrameContext* WaitForNextFrameResources()
//{
//    UINT nextFrameIndex = g_frameIndex + 1;
//    g_frameIndex = nextFrameIndex;
//
//    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
//    DWORD numWaitableObjects = 1;
//
//    FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
//    UINT64 fenceValue = frameCtx->FenceValue;
//    if (fenceValue != 0) // means no fence was signaled
//    {
//        frameCtx->FenceValue = 0;
//        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
//        waitableObjects[1] = g_fenceEvent;
//        numWaitableObjects = 2;
//    }
//
//    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);
//
//    return frameCtx;
//}

void ResizeSwapChain(HWND hWnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC1 sd;
    g_pSwapChain->GetDesc1(&sd);
    sd.Width = width;
    sd.Height = height;

    IDXGIFactory4* dxgiFactory = nullptr;
    g_pSwapChain->GetParent(IID_PPV_ARGS(&dxgiFactory));

    g_pSwapChain->Release();
    CloseHandle(g_hSwapChainWaitableObject);

    IDXGISwapChain1* swapChain1 = NULL;
    dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1);
    swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
    swapChain1->Release();
    dxgiFactory->Release();

    g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

    g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    assert(g_hSwapChainWaitableObject != NULL);
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
    ImGui_ImplDX12_CreateDeviceObjects();
}

HRESULT __stdcall hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
    static bool init = false;

    if (!init) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&g_pd3dDevice))) {

            // sDesc

            DXGI_SWAP_CHAIN_DESC sdesc;
            pSwapChain->GetDesc(&sdesc);
            sdesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            window = sdesc.OutputWindow;
            sdesc.Windowed = ((GetWindowLongPtr(window, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

            DXGI_SWAP_CHAIN_DESC1 sdesc1;
            pSwapChain->GetDesc1(&sdesc1);

            //NUM_BACK_BUFFERS = sdesc1.BufferCount;
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
                //for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                //{
                //    g_mainRenderTargetDescriptor[i] = rtvHandle;
                //    rtvHandle.ptr += rtvDescriptorSize;
                //}

                for (size_t i = 0; i < NUM_BACK_BUFFERS; i++) {
                    ID3D12Resource* pBackBuffer = nullptr;

                    g_mainRenderTargetDescriptor[i] = rtvHandle;
                    pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
                    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
                    g_mainRenderTargetResource[i] = pBackBuffer;
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


            ID3D12CommandAllocator* allocator;
            if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)) != S_OK)
                return E_FAIL;

            for (size_t i = 0; i < NUM_BACK_BUFFERS; i++) {
                g_frameContext[i].CommandAllocator = allocator;
            }

            if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
                g_pd3dCommandList->Close() != S_OK)
                return E_FAIL;

            // Create Command Allocators for each frame
            //for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
            //    if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            //        return false;

            //if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
            //    g_pd3dCommandList->Close() != S_OK)
            //    return false;

            g_fenceEvent = CreateEvent(nullptr, false, false, nullptr);

            g_hSwapChainWaitableObject = pSwapChain->GetFrameLatencyWaitableObject();
            g_pSwapChain = pSwapChain;

            CreateRenderTarget();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
            InitImGui();
        }
        init = true;
    }


    if (!g_pd3dCommandQueue) {
        printf("Failed to create command queue\n");
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    if (GetAsyncKeyState(VK_INSERT) & 1) show_demo_window = !show_demo_window;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = show_demo_window;
    if (show_demo_window)
        ImGui::ShowDemoWindow();


    //FrameContext currentFrameContext = g_frameContext[pSwapChain->GetCurrentBackBufferIndex()];
    //currentFrameContext.CommandAllocator->Reset();

    FrameContext& frameCtx = g_frameContext[pSwapChain->GetCurrentBackBufferIndex()];
    frameCtx.CommandAllocator->Reset();

    //FrameContext* frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    //frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    g_pd3dCommandList->Reset(frameCtx.CommandAllocator, nullptr);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    //const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    //g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);

    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    g_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&g_pd3dCommandList));

    g_fenceLastSignaledValue++;
    g_pd3dCommandQueue->Signal(g_fence, g_fenceLastSignaledValue);
    g_fence->SetEventOnCompletion(g_fenceLastSignaledValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (!g_pd3dCommandQueue) {
        g_pd3dCommandQueue = pCommandQueue;
    }

    oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT __stdcall hkResizeBuffers(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{

    if (g_pd3dCommandQueue)
    {
        g_fenceLastSignaledValue++;
        g_pd3dCommandQueue->Signal(g_fence, g_fenceLastSignaledValue);
        g_fence->SetEventOnCompletion(g_fenceLastSignaledValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    CleanupRenderTarget();

    // Вызываем оригинальный ResizeBuffers
    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (FAILED(hr))
    {
        std::cerr << "ResizeBuffers failed with HRESULT: " << std::hex << hr << std::endl;
        return hr;
    }

    // Пересоздаем рендер-таргеты
    CreateRenderTarget();


    return hr;
}

HRESULT __stdcall hkSignal(ID3D12CommandQueue* queue, ID3D12Fence* fence, UINT64 value)
{
    if (g_pd3dCommandQueue != nullptr && queue == g_pd3dCommandQueue) {
        g_fence = fence;
        //g_fenceValue = value;
    }
    return oSignal(queue, fence, value);;
}




void InitD3D12Hook()
{
    static bool init = false;
    do {
        if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
            kiero::bind(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
            kiero::bind(58, (void**)&oSignal, hkSignal);
            kiero::bind(140, (void**)&oPresent, hkPresent);
            kiero::bind(145, (void**)&oResizeBuffers, hkResizeBuffers);
            init = true;
            std::cout << "KIERO!!" << std::endl;
        }
    } while (!init);
}
