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
    if (!g_pSwapChain || !g_pd3dDevice || !g_pd3dRtvDescHeap || !g_frameContext || NUM_BACK_BUFFERS <= 0)
        return;

    // Получаем размер дескриптора RTV и начальный дескриптор
    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

    // Сначала инициализируем дескрипторы
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        g_frameContext[i].g_mainRenderTargetDescriptor = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }

    // Затем создаем render target views
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource *pBackBuffer = nullptr;
        if (SUCCEEDED(g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer))))
        {
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_frameContext[i].g_mainRenderTargetDescriptor);
            g_frameContext[i].g_mainRenderTargetResource = pBackBuffer;
        }
        else
        {
            LOG_ERROR("Failed to get back buffer %d", i);
            if (pBackBuffer)
            {
                pBackBuffer->Release();
                pBackBuffer = nullptr;
            }
        }
    }
}

void CleanupRenderTarget()
{
    if (g_frameContext && NUM_BACK_BUFFERS > 0)
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;  // prevent system cursor flicker on exit
    ImGui::StyleColorsLight();

    ImGui_ImplWin32_Init(window);

    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                        DXGI_FORMAT_R8G8B8A8_UNORM,
                        g_pd3dSrvDescHeap,
                        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
}

HRESULT __fastcall hkPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool init = false;

    if (!init)
    {
        // Ждем пока не получим CommandQueue через ExecuteCommandLists
        if (!g_pd3dCommandQueue)
            return oPresent(pSwapChain, SyncInterval, Flags);

        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void **)&g_pd3dDevice)))
        {
            DXGI_SWAP_CHAIN_DESC sdesc;
            pSwapChain->GetDesc(&sdesc);
            window = sdesc.OutputWindow;
            NUM_BACK_BUFFERS = sdesc.BufferCount;

            // SRV Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap))))
                    return oPresent(pSwapChain, SyncInterval, Flags);
            }

            // RTV Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap))))
                    return oPresent(pSwapChain, SyncInterval, Flags);
            }

            // Command Allocator
            ID3D12CommandAllocator *allocator;
            if (FAILED(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
                return oPresent(pSwapChain, SyncInterval, Flags);

            // Command List
            if (FAILED(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList))))
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }
            g_pd3dCommandList->Close();

            // Frame Contexts
            g_frameContext = new FrameContext[NUM_BACK_BUFFERS];
            if (!g_frameContext)
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                g_frameContext[i].CommandAllocator = allocator;
                g_frameContext[i].FenceValue = 0;
            }

            // Fence & Events
            if (FAILED(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence))))
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (g_fenceEvent == nullptr)
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            g_hSwapChainWaitableObject = pSwapChain->GetFrameLatencyWaitableObject();
            g_pSwapChain = pSwapChain;

            // Create RenderTarget
            CreateRenderTarget();

            // Hook window procedure
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);

            // Initialize ImGui last, after all DirectX objects are created
            InitImGui();

            init = true;
        }
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    // Проверяем все необходимые объекты
    if (!g_pd3dCommandQueue || !g_pd3dDevice || !g_frameContext || !g_pd3dSrvDescHeap)
        return oPresent(pSwapChain, SyncInterval, Flags);

    // Обработка изменения размера
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        WaitForLastSubmittedFrame();
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }

    // Обработка переключения окна
    if (GetAsyncKeyState(VK_INSERT) & 1)
        show_demo_window = !show_demo_window;

    // Начало нового кадра
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Отрисовка ImGui
    ImGui::GetIO().MouseDrawCursor = show_demo_window;
    if (show_demo_window)
        ImGui::ShowDemoWindow();

    // Получаем текущий back buffer
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    FrameContext &frameCtx = g_frameContext[backBufferIdx];

    // Сброс command allocator
    frameCtx.CommandAllocator->Reset();

    // Подготовка к рендерингу
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_frameContext[backBufferIdx].g_mainRenderTargetResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Выполнение команд рендеринга
    g_pd3dCommandList->Reset(frameCtx.CommandAllocator, nullptr);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->OMSetRenderTargets(1, &g_frameContext[backBufferIdx].g_mainRenderTargetDescriptor, FALSE, nullptr);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    // Рендеринг ImGui
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    // Возврат ресурса в состояние present
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    // Выполнение command list
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
    // Проверяем готовность к изменению размера
    if (!g_pd3dDevice || !g_pSwapChain)
    {
        LOG_ERROR("Cannot resize - DirectX objects not initialized");
        return oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    if (g_pd3dDevice)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();
    }

    CleanupRenderTarget();

    // Сохраняем новое количество буферов
    NUM_BACK_BUFFERS = BufferCount;

    // Вызываем оригинальную функцию
    HRESULT result = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (SUCCEEDED(result))
    {
        // Пересоздаем наши ресурсы только если успешно изменили размер
        CreateRenderTarget();
        if (g_pd3dDevice)
        {
            ImGui_ImplDX12_CreateDeviceObjects();
        }
    }
    else
    {
        LOG_ERROR("ResizeBuffers failed with error code: 0x%X", result);
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
            LOG_ERROR("Failed to initialize kiero");
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
    kiero::shutdown();

    if (g_pd3dDevice)
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    if (g_pd3dCommandQueue && g_fence && g_fenceEvent)
    {
        WaitForLastSubmittedFrame();
    }

    // Clean up render targets
    CleanupRenderTarget();

    // Release command allocators
    if (g_frameContext)
    {
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            if (g_frameContext[i].CommandAllocator)
            {
                g_frameContext[i].CommandAllocator->Release();
                g_frameContext[i].CommandAllocator = nullptr;
            }
        }
        delete[] g_frameContext;
        g_frameContext = nullptr;
    }

    if (g_pd3dCommandList)
    {
        g_pd3dCommandList->Release();
        g_pd3dCommandList = nullptr;
    }

    if (g_pd3dCommandQueue)
    {
        g_pd3dCommandQueue->Release();
        g_pd3dCommandQueue = nullptr;
    }

    // Close handles before releasing resources
    if (g_fenceEvent)
    {
        CloseHandle(g_fenceEvent);
        g_fenceEvent = nullptr;
    }

    if (g_hSwapChainWaitableObject)
    {
        CloseHandle(g_hSwapChainWaitableObject);
        g_hSwapChainWaitableObject = nullptr;
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

    g_pd3dDevice = nullptr;
    g_pSwapChain = nullptr;
    window = nullptr;

    NUM_BACK_BUFFERS = -1;
    g_frameIndex = 0;
    g_fenceValue = 0;
}
