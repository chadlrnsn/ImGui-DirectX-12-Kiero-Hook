#include <framework/stdafx.h>
#include "d3d12hook.h"
#include <kiero.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

// Debug
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

typedef HRESULT(__stdcall *PresentFunc)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
static PresentFunc oPresent = nullptr;

typedef void(__stdcall *ExecuteCommandListsFunc)(ID3D12CommandQueue *pCommandQueue, UINT NumCommandLists, ID3D12CommandList *const *ppCommandLists);
static ExecuteCommandListsFunc oExecuteCommandLists = nullptr;

typedef HRESULT(__stdcall *ResizeBuffersFunc)(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
static ResizeBuffersFunc oResizeBuffers = nullptr;

typedef HRESULT(__stdcall *SignalFunc)(ID3D12CommandQueue *queue, ID3D12Fence *fence, UINT64 value);
static SignalFunc oSignal = nullptr;

HWND window = nullptr;
WNDPROC oWndProc = nullptr;

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator = nullptr;
    ID3D12GraphicsCommandList* CommandList = nullptr;   // once per frame
    UINT64 FenceValue = 0;
    ID3D12Resource* g_mainRenderTargetResource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor = {};
};

static FrameContext *g_frameContext = nullptr;
static int NUM_BACK_BUFFERS = 0;
static UINT64 g_fenceValue = 0;

static ID3D12Device *g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap *g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap *g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12CommandQueue* g_pendingQueue = nullptr;    // stash it until we can verify it belongs to our device else DEVICE_REMOVED ofc
static IUnknown* g_swapchainIdentity = nullptr; // so we can ignore Present from other swapchains
static ID3D12Fence* g_fence = nullptr;
static HANDLE g_fenceEvent = nullptr;
static IDXGISwapChain3 *g_pSwapChain = nullptr;

static bool g_isResizing = false;
static bool g_initDone = false;
static UINT g_presentCallCount = 0;
static DWORD g_presentThreadId = 0;

bool show_demo_window = true;
bool bShould_render = true;

// Helper: check device removed reason
static void LogDeviceState(const char* context)
{
    if (!g_pd3dDevice) return;
    HRESULT reason = g_pd3dDevice->GetDeviceRemovedReason();
    if (reason != S_OK)
    {
        LOG_ERROR("[%s] DEVICE REMOVED! Reason: 0x%08X", context, reason);
    }
}

// Helper: compare swapchains. Basically identity check with IUnknown
static bool IsSameSwapchain(IDXGISwapChain* a, IDXGISwapChain* b)
{
    IUnknown* ia = nullptr;
    IUnknown* ib = nullptr;
    if (FAILED(a->QueryInterface(IID_PPV_ARGS(&ia))) || !ia) return false;
    if (FAILED(b->QueryInterface(IID_PPV_ARGS(&ib))) || !ib) { 
        ia->Release(); 
        return false; 
    }
    const bool same = (ia == ib);   // dxgi can give different interfaces for the same object. That's why we compare it like this.
    ia->Release();
    ib->Release();
    return same;
}

void CreateRenderTarget()
{
    LOG_DEBUG("[CreateRenderTarget] start: swapchain=%p device=%p rtvHeap=%p frameCtx=%p numBuf=%d",
        g_pSwapChain, g_pd3dDevice, g_pd3dRtvDescHeap, g_frameContext, NUM_BACK_BUFFERS);

    if (!g_pSwapChain || !g_pd3dDevice || !g_pd3dRtvDescHeap || !g_frameContext || NUM_BACK_BUFFERS <= 0)
    {
        LOG_ERROR("[CreateRenderTarget] precondition failed, aborting");
        return;
    }

    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
    {
        g_frameContext[i].g_mainRenderTargetDescriptor = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }

    for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource *pBackBuffer = nullptr;
        HRESULT hr = g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        if (SUCCEEDED(hr))
        {
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_frameContext[i].g_mainRenderTargetDescriptor);
            g_frameContext[i].g_mainRenderTargetResource = pBackBuffer;
            LOG_DEBUG("[CreateRenderTarget] buffer[%u] = %p", i, pBackBuffer);
        }
        else
        {
            LOG_ERROR("[CreateRenderTarget] GetBuffer(%u) FAILED: 0x%08X", i, hr);
            if (pBackBuffer) { pBackBuffer->Release(); }
        }
    }
    LOG_DEBUG("[CreateRenderTarget] done");
}

void CleanupRenderTarget()
{
    LOG_DEBUG("[CleanupRenderTarget] numBuf=%d frameCtx=%p", NUM_BACK_BUFFERS, g_frameContext);
    if (g_frameContext && NUM_BACK_BUFFERS > 0)
    {
        for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
        {
            if (g_frameContext[i].g_mainRenderTargetResource)
            {
                g_frameContext[i].g_mainRenderTargetResource->Release();
                g_frameContext[i].g_mainRenderTargetResource = nullptr;
                LOG_DEBUG("[CleanupRenderTarget] released buffer[%u]", i);
            }
        }
    }
}

void WaitForAllFrames()
{
    if (!g_pd3dCommandQueue || !g_fence || !g_fenceEvent)
    {
        LOG_WARN("[WaitForAllFrames] skipped: queue=%p fence=%p event=%p", g_pd3dCommandQueue, g_fence, g_fenceEvent);
        return;
    }

    g_fenceValue++;
    LOG_DEBUG("[WaitForAllFrames] Signal fence=%llu", g_fenceValue);
    HRESULT hr = g_pd3dCommandQueue->Signal(g_fence, g_fenceValue);
    if (FAILED(hr))
    {
        LOG_ERROR("[WaitForAllFrames] Signal FAILED: 0x%08X", hr);
        LogDeviceState("WaitForAllFrames-Signal");
        return;
    }

    UINT64 completed = g_fence->GetCompletedValue();
    if (completed < g_fenceValue)
    {
        LOG_DEBUG("[WaitForAllFrames] waiting: completed=%llu need=%llu", completed, g_fenceValue);
        g_fence->SetEventOnCompletion(g_fenceValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
        LOG_DEBUG("[WaitForAllFrames] wait done");
    }

    if (g_frameContext && NUM_BACK_BUFFERS > 0)
    {
        for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
            g_frameContext[i].FenceValue = 0;
    }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SIZE)
    {
        LOG_DEBUG("[WndProc] WM_SIZE wParam=%u lParam=(%u x %u) isResizing=%d",
            (UINT)wParam, LOWORD(lParam), HIWORD(lParam), g_isResizing);
        fflush(stdout);
    }

    // Block WM_PAINT while we are inside hkResizeBuffers.
    // The app's render targets are already released at this point;
    // letting WM_PAINT through would make the app call OnRender() →
    // PopulateCommandList() on destroyed resources → crash.
    if (uMsg == WM_PAINT && g_isResizing)
    {
        // Validate the region to stop Windows from reposting WM_PAINT
        ValidateRect(hwnd, nullptr);
        return 0;
    }

    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

void InitImGui()
{
    LOG_INFO("[InitImGui] start");

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsLight();

    if (window)
    {
        ImGui_ImplWin32_Init(window);
        LOG_DEBUG("[InitImGui] Win32 init with HWND=%p", window);
    }

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (g_pSwapChain)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        if (SUCCEEDED(g_pSwapChain->GetDesc(&desc)) && desc.BufferDesc.Format != DXGI_FORMAT_UNKNOWN)
        {
            format = desc.BufferDesc.Format;
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC1 desc1;
            if (SUCCEEDED(g_pSwapChain->GetDesc1(&desc1)))
            {
                format = desc1.Format;
            }
        }
    }

    LOG_DEBUG("[InitImGui] format=%u numFrames=%d", format, NUM_BACK_BUFFERS);

    // Force fonts build. I thought ImGui_ImplDX12_Init does this for us, ig not.
    io.Fonts->Build();

    const int framesInFlight = (NUM_BACK_BUFFERS > 0 ? NUM_BACK_BUFFERS : 2) + 1;
    ImGui_ImplDX12_Init(g_pd3dDevice, framesInFlight, format, g_pd3dSrvDescHeap, g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
    ImGui_ImplDX12_CreateDeviceObjects();   // 6-arg init does NOT build the font texture. skip this and we crash on first draw
    LOG_INFO("[InitImGui] done");
}

HRESULT __fastcall hkPresent(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags)
{
    g_presentCallCount++;

    if (g_presentThreadId == 0) {
        g_presentThreadId = GetCurrentThreadId();
    }

    if (!g_initDone)
    {
        if (!g_pendingQueue) return oPresent(pSwapChain, SyncInterval, Flags);

        if (FAILED(pSwapChain->QueryInterface(IID_PPV_ARGS(&g_pSwapChain))))
        {
            LOG_ERROR("[hkPresent] init: QueryInterface IDXGISwapChain3 FAILED");
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        if (SUCCEEDED(g_pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&g_pd3dDevice)))
        {
            if (!g_swapchainIdentity)
            {
                // capture our swapchain identity. Then we can compare with imposter swapchains.
                pSwapChain->QueryInterface(IID_PPV_ARGS(&g_swapchainIdentity));
            }

            LOG_INFO("[hkPresent] init: device=%p", g_pd3dDevice);

            // ignore pending queue if its not from swapchain device
            if (!g_pd3dCommandQueue)
            {
                ID3D12Device* queueDevice = nullptr;
                if (SUCCEEDED(g_pendingQueue->GetDevice(IID_PPV_ARGS(&queueDevice))) && queueDevice)
                {
                    if (queueDevice == g_pd3dDevice)
                    {
                        g_pd3dCommandQueue = g_pendingQueue;
                        LOG_INFO("[hkPresent] init: commandQueue captured=%p", g_pd3dCommandQueue);
                    }
                    queueDevice->Release();
                }

                if (!g_pd3dCommandQueue) return oPresent(pSwapChain, SyncInterval, Flags);
            }

            DXGI_SWAP_CHAIN_DESC sdesc;
            if (SUCCEEDED(g_pSwapChain->GetDesc(&sdesc)))
            {
                window = sdesc.OutputWindow;
                NUM_BACK_BUFFERS = sdesc.BufferCount;
                LOG_INFO("[hkPresent] init: HWND=%p bufferCount=%d format=%u w=%u h=%u",
                    window, NUM_BACK_BUFFERS, sdesc.BufferDesc.Format,
                    sdesc.BufferDesc.Width, sdesc.BufferDesc.Height);
            }
            if (!window)
            {
                window = GetActiveWindow();
                LOG_WARN("[hkPresent] init: OutputWindow=NULL, using GetActiveWindow=%p", window);
            }

            if (NUM_BACK_BUFFERS <= 0)
                NUM_BACK_BUFFERS = 2;

            // SRV Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                HRESULT hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));
                if (FAILED(hr))
                {
                    LOG_ERROR("[hkPresent] init: CreateDescriptorHeap SRV FAILED: 0x%08X", hr);
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }
            }

            // RTV Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                HRESULT hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
                if (FAILED(hr))
                {
                    LOG_ERROR("[hkPresent] init: CreateDescriptorHeap RTV FAILED: 0x%08X", hr);
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }
            }

            // Frame Contexts
            g_frameContext = new FrameContext[NUM_BACK_BUFFERS];

            for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
            {
                g_frameContext[i].CommandAllocator = nullptr;
                g_frameContext[i].CommandList = nullptr;
                g_frameContext[i].FenceValue = 0;
                HRESULT hr = g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator));
                if (FAILED(hr))
                {
                    LOG_ERROR("[hkPresent] init: CreateCommandAllocator[%u] FAILED: 0x%08X", i, hr);
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }
            }
            LOG_DEBUG("[hkPresent] init: created %d command allocators", NUM_BACK_BUFFERS);

            // Command List
            for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
            {
                HRESULT hr = g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[i].CommandAllocator, nullptr, IID_PPV_ARGS(&g_frameContext[i].CommandList));
                if (FAILED(hr))
                {
                    LOG_ERROR("[hkPresent] init: CreateCommandList[%u] FAILED: 0x%08X", i, hr);
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }
                g_frameContext[i].CommandList->Close();
            }

            // Fence & Event
            {
                HRESULT hr = g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence));
                if (FAILED(hr))
                {
                    LOG_ERROR("[hkPresent] init: CreateFence FAILED: 0x%08X", hr);
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }
            }

            g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (g_fenceEvent == nullptr)
            {
                LOG_ERROR("[hkPresent] init: CreateEvent FAILED");
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            CreateRenderTarget();

            if (window)
            {
                oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
                LOG_DEBUG("[hkPresent] init: WndProc hooked, old=%p", oWndProc);
            }

            InitImGui();

            g_initDone = true;
            LOG_INFO("[hkPresent] === INIT COMPLETE ===");
        }
        else
        {
            LOG_ERROR("[hkPresent] init: GetDevice FAILED");
        }
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    // --- Per-frame rendering ---

    if (!g_pd3dDevice || !g_frameContext || !g_pd3dSrvDescHeap || !g_pSwapChain)
    {
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    if (g_isResizing)
    {
        LOG_DEBUG("[hkPresent] frame %u: SKIPPED (resizing)", g_presentCallCount);
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    // ignore present from any swapchain except the one we hooked
    if (g_swapchainIdentity && !IsSameSwapchain(pSwapChain, g_pSwapChain)) return oPresent(pSwapChain, SyncInterval, Flags);

    // Check device health
    {
        HRESULT reason = g_pd3dDevice->GetDeviceRemovedReason();
        if (reason != S_OK)
        {
            LOG_ERROR("[hkPresent] frame %u: DEVICE ALREADY REMOVED before render: 0x%08X", g_presentCallCount, reason);
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
        show_demo_window = !show_demo_window;

    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    if (backBufferIdx >= (UINT)NUM_BACK_BUFFERS)
    {
        LOG_ERROR("[hkPresent] frame %u: backBufferIdx=%u >= NUM_BACK_BUFFERS=%d !!",
            g_presentCallCount, backBufferIdx, NUM_BACK_BUFFERS);
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    FrameContext &frameCtx = g_frameContext[backBufferIdx];
    if (!frameCtx.CommandAllocator)
    {
        LOG_ERROR("[hkPresent] frame %u: CommandAllocator[%u] is NULL!", g_presentCallCount, backBufferIdx);
        return oPresent(pSwapChain, SyncInterval, Flags);
    }
    if (!frameCtx.g_mainRenderTargetResource)
    {
        LOG_ERROR("[hkPresent] frame %u: RenderTarget[%u] is NULL!", g_presentCallCount, backBufferIdx);
        return oPresent(pSwapChain, SyncInterval, Flags);
    }
    if (!frameCtx.CommandList)
    {
        LOG_ERROR("[hkPresent] frame %u: CommandList[%u] is NULL!", g_presentCallCount, backBufferIdx);
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    // Wait for this frame's previous GPU work to finish before reusing its allocator
    if (frameCtx.FenceValue != 0 && g_fence && g_fence->GetCompletedValue() < frameCtx.FenceValue)
    {
        g_fence->SetEventOnCompletion(frameCtx.FenceValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    // Begin ImGui frame
    ImGui_ImplWin32_NewFrame(); // platform first then renderer
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = show_demo_window;
    if (show_demo_window)
        ImGui::ShowDemoWindow();

    // Reset allocator and record commands
    HRESULT hrReset = frameCtx.CommandAllocator->Reset();
    if (FAILED(hrReset))
    {
        LOG_ERROR("[hkPresent] frame %u: CommandAllocator->Reset() FAILED: 0x%08X", g_presentCallCount, hrReset);
        LogDeviceState("Present-AllocatorReset");
        ImGui::EndFrame();
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    HRESULT hrListReset = frameCtx.CommandList->Reset(frameCtx.CommandAllocator, nullptr);
    if (FAILED(hrListReset))
    {
        LOG_ERROR("[hkPresent] frame %u: CommandList->Reset() FAILED: 0x%08X", g_presentCallCount, hrListReset);
        LogDeviceState("Present-ListReset");
        ImGui::EndFrame();
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frameCtx.g_mainRenderTargetResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    frameCtx.CommandList->ResourceBarrier(1, &barrier);
    frameCtx.CommandList->OMSetRenderTargets(1, &frameCtx.g_mainRenderTargetDescriptor, FALSE, nullptr);
    frameCtx.CommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), frameCtx.CommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    frameCtx.CommandList->ResourceBarrier(1, &barrier);

    HRESULT hrClose = frameCtx.CommandList->Close();
    if (FAILED(hrClose))
    {
        LOG_ERROR("[hkPresent] frame %u: CommandList->Close() FAILED: 0x%08X", g_presentCallCount, hrClose);
        LogDeviceState("Present-ListClose");
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    ID3D12CommandList* ppCommandLists[] = { frameCtx.CommandList };
    g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Signal fence for this frame — do NOT wait here.
    // We wait at the TOP of the next frame that reuses this same backBufferIdx.
    // Blocking here would stall the entire game render pipeline → TDR → DEVICE_REMOVED.
    g_fenceValue++;
    g_pd3dCommandQueue->Signal(g_fence, g_fenceValue);
    frameCtx.FenceValue = g_fenceValue;

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void __fastcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (!g_pd3dCommandQueue && pCommandQueue && GetCurrentThreadId() == g_presentThreadId)
    {
        D3D12_COMMAND_QUEUE_DESC desc = pCommandQueue->GetDesc();
        if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
            g_pendingQueue = pCommandQueue; // store then later verify identity in hkPresent
        }
    }

    oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT __fastcall hkResizeBuffers(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    LOG_INFO("[hkResizeBuffers] ENTER: BufferCount=%u Width=%u Height=%u Format=%u Flags=0x%X initDone=%d",
        BufferCount, Width, Height, NewFormat, SwapChainFlags, g_initDone);

    if (!g_pd3dDevice || !g_pSwapChain || !g_frameContext)
    {
        LOG_WARN("[hkResizeBuffers] not initialized, passthrough");
        return oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    LogDeviceState("ResizeBuffers-entry");

    g_isResizing = true;

    ImGui_ImplDX12_InvalidateDeviceObjects();   // drop font SRV and PSO before back buffers go away

    LOG_DEBUG("[hkResizeBuffers] WaitForAllFrames...");
    fflush(stdout);
    WaitForAllFrames();

    LOG_DEBUG("[hkResizeBuffers] CleanupRenderTarget...");
    fflush(stdout);
    CleanupRenderTarget();
    LOG_DEBUG("[hkResizeBuffers] CleanupRenderTarget done");
    fflush(stdout);

    LOG_DEBUG("[hkResizeBuffers] calling oResizeBuffers...");
    fflush(stdout);
    HRESULT result = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    LOG_INFO("[hkResizeBuffers] oResizeBuffers returned: 0x%08X", result);
    fflush(stdout);

    if (FAILED(result))
    {
        LOG_ERROR("[hkResizeBuffers] oResizeBuffers FAILED: 0x%08X", result);
        LogDeviceState("ResizeBuffers-afterCall");
        // we already invalidated. Here must be rebuilt
        ImGui_ImplDX12_CreateDeviceObjects();
        g_isResizing = false;
        return result;
    }

    // Determine actual buffer count
    UINT actualBufferCount = BufferCount;
    if (actualBufferCount == 0)
    {
        DXGI_SWAP_CHAIN_DESC sdesc = {};
        if (SUCCEEDED(g_pSwapChain->GetDesc(&sdesc)))
        {
            actualBufferCount = sdesc.BufferCount;
        }
    }
    if (actualBufferCount == 0)
        actualBufferCount = 2;

    LOG_DEBUG("[hkResizeBuffers] actualBufferCount=%u old NUM_BACK_BUFFERS=%d", actualBufferCount, NUM_BACK_BUFFERS);

    // Reallocate if buffer count changed
    if (actualBufferCount != (UINT)NUM_BACK_BUFFERS)
    {
        LOG_INFO("[hkResizeBuffers] buffer count changed %d -> %u, reallocating", NUM_BACK_BUFFERS, actualBufferCount);

        if (g_frameContext)
        {
            for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
            {
                if (g_frameContext[i].CommandList)
                {
                    g_frameContext[i].CommandList->Release();
                    g_frameContext[i].CommandList = nullptr;
                }
                if (g_frameContext[i].CommandAllocator)
                {
                    g_frameContext[i].CommandAllocator->Release();
                    g_frameContext[i].CommandAllocator = nullptr;
                }
            }
            delete[] g_frameContext;
            g_frameContext = nullptr;
        }

        if (g_pd3dRtvDescHeap)
        {
            g_pd3dRtvDescHeap->Release();
            g_pd3dRtvDescHeap = nullptr;
        }

        NUM_BACK_BUFFERS = actualBufferCount;

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        HRESULT hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
        LOG_DEBUG("[hkResizeBuffers] new RTV heap: hr=0x%08X ptr=%p", hr, g_pd3dRtvDescHeap);

        g_frameContext = new FrameContext[NUM_BACK_BUFFERS];
        for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
        {
            g_frameContext[i].CommandAllocator = nullptr;
            g_frameContext[i].CommandList = nullptr;
            g_frameContext[i].FenceValue = 0;
            g_frameContext[i].g_mainRenderTargetResource = nullptr;
            g_frameContext[i].g_mainRenderTargetDescriptor = {};

            g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator));

            g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[i].CommandAllocator, nullptr, IID_PPV_ARGS(&g_frameContext[i].CommandList));
            if (g_frameContext[i].CommandList) {
                g_frameContext[i].CommandList->Close();
            }
        }
    }

    LOG_DEBUG("[hkResizeBuffers] CreateRenderTarget...");
    CreateRenderTarget();

    // Actually we do need to call CreateDeviceObjects here. RTV formats and SRV font gpu could changed.
    ImGui_ImplDX12_CreateDeviceObjects();

    g_isResizing = false;

    LogDeviceState("ResizeBuffers-exit");
    LOG_INFO("[hkResizeBuffers] EXIT OK");

    return result;
}

HRESULT __fastcall hkSignal(ID3D12CommandQueue *queue, ID3D12Fence *fence, UINT64 value)
{
    return oSignal(queue, fence, value);
}

bool InitD3D12Hook()
{
    LOG_INFO("Waiting for process initialization...");

    HANDLE d3d12Module = nullptr;
    HANDLE dxgiModule = nullptr;

    while (true)
    {
        d3d12Module = GetModuleHandleA("d3d12.dll");
        dxgiModule = GetModuleHandleA("dxgi.dll");

        if (d3d12Module && dxgiModule)
            break;

        if (WaitForSingleObject(GetCurrentProcess(), 1000) != WAIT_TIMEOUT)
        {
            LOG_ERROR("Process terminated while waiting for DirectX");
            return false;
        }

        LOG_INFO("Waiting for DirectX modules...");
    }

    LOG_INFO("DirectX modules found, initializing hooks...");

    try
    {
        auto kieroStatus = kiero::init(kiero::RenderType::D3D12);
        if (kieroStatus != kiero::Status::Success)
        {
            LOG_ERROR("Failed to initialize kiero (status=%d)", (int)kieroStatus);
            return false;
        }

        bool hooks_success = true;

        if (kiero::bind(54, (void **)&oExecuteCommandLists, hkExecuteCommandLists) != kiero::Status::Success)
        {
            LOG_ERROR("Failed to hook ExecuteCommandLists");
            hooks_success = false;
        }

        if (kiero::bind(58, (void **)&oSignal, hkSignal) != kiero::Status::Success)
        {
            LOG_ERROR("Failed to hook Signal");
            hooks_success = false;
        }

        if (kiero::bind(140, (void **)&oPresent, hkPresent) != kiero::Status::Success)
        {
            LOG_ERROR("Failed to hook Present");
            hooks_success = false;
        }

        if (kiero::bind(145, (void **)&oResizeBuffers, hkResizeBuffers) != kiero::Status::Success)
        {
            LOG_ERROR("Failed to hook ResizeBuffers");
            hooks_success = false;
        }

        if (!hooks_success)
        {
            LOG_ERROR("Failed to create one or more hooks");
            kiero::shutdown();
            return false;
        }

        LOG_INFO("D3D12 successfully hooked using kiero");
        return true;
    }
    catch (...)
    {
        LOG_ERROR("Exception during hook initialization");
        kiero::shutdown();
        return false;
    }
}

void ReleaseD3D12Hook()
{
    LOG_INFO("[ReleaseD3D12Hook] start");
    kiero::shutdown();

    if (g_pd3dCommandQueue && g_fence && g_fenceEvent)
    {
        WaitForAllFrames();
    }

    if (g_pd3dDevice)
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    CleanupRenderTarget();

    if (g_frameContext)
    {
        for (UINT i = 0; i < (UINT)NUM_BACK_BUFFERS; i++)
        {
            if (g_frameContext[i].CommandList)
            {
                g_frameContext[i].CommandList->Release();
                g_frameContext[i].CommandList = nullptr;
            }
            if (g_frameContext[i].CommandAllocator)
            {
                g_frameContext[i].CommandAllocator->Release();
                g_frameContext[i].CommandAllocator = nullptr;
            }
        }
        delete[] g_frameContext;
        g_frameContext = nullptr;
    }

    if (g_fenceEvent)
    {
        CloseHandle(g_fenceEvent);
        g_fenceEvent = nullptr;
    }

    if (g_pd3dRtvDescHeap)
    {
        g_pd3dRtvDescHeap->Release();
        g_pd3dRtvDescHeap = nullptr;
    }

    if (g_pd3dSrvDescHeap)
    {
        g_pd3dSrvDescHeap->Release();
        g_pd3dSrvDescHeap = nullptr;
    }

    if (g_fence)
    {
        g_fence->Release();
        g_fence = nullptr;
    }

    if (oWndProc && window)
    {
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        oWndProc = nullptr;
    }

    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }

    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }

    if (g_swapchainIdentity)
    {
        g_swapchainIdentity->Release();
        g_swapchainIdentity = nullptr;
    }

    window = nullptr;
    g_pd3dCommandQueue = nullptr;
    g_pendingQueue = nullptr;
    NUM_BACK_BUFFERS = 0;
    g_fenceValue = 0;
    g_initDone = false;

    LOG_INFO("[ReleaseD3D12Hook] done");
}
