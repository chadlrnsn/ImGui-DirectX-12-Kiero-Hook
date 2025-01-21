#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>

// Глобальные переменные
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int FRAME_COUNT = 2;

// D3D12 объекты
ID3D12Device* device = nullptr;
ID3D12CommandQueue* commandQueue = nullptr;
ID3D12CommandAllocator* commandAllocator = nullptr;
ID3D12GraphicsCommandList* commandList = nullptr;
IDXGISwapChain3* swapChain = nullptr;
ID3D12DescriptorHeap* rtvHeap = nullptr;
ID3D12Resource* renderTargets[FRAME_COUNT] = {};
ID3D12Fence* fence = nullptr;
HANDLE fenceEvent = nullptr;
UINT64 fenceValue = 0;
UINT frameIndex = 0;
UINT rtvDescriptorSize = 0;

// Прототипы функций
bool InitWindow(HINSTANCE hInstance, int nCmdShow, HWND& hwnd);
bool InitD3D12(HWND hwnd);
void Update();
void Render();
void WaitForPreviousFrame();
void CleanupD3D12();
void OnResize(HWND hwnd, UINT width, UINT height);

// Оконная процедура
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        if (device != nullptr) {
            OnResize(hwnd, LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char* argv[]) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    HWND hwnd = nullptr;

    if (!InitWindow(hInstance, SW_SHOW, hwnd)) {
        return -1;
    }

    if (!InitD3D12(hwnd)) {
        CleanupD3D12();
        return -1;
    }

    LoadLibraryW(L"DirectX12-Example-ImGui.dll");
    // LoadLibraryW(L"C:\\Users\\Litch\\Downloads\\imgui_dx12_kiero_hook.dll");

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Update();
            Render();
        }
    }

    WaitForPreviousFrame();
    CleanupD3D12();
    return 0;
}

bool InitWindow(HINSTANCE hInstance, int nCmdShow, HWND& hwnd) {
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = L"DX12WindowClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"DirectX 12 Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    ShowWindow(hwnd, nCmdShow);
    return true;
}

bool InitD3D12(HWND hwnd) {
    // Создание устройства
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;
    D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&device));

    // Создание очереди команд
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    // Создание swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = WINDOW_WIDTH;
    swapChainDesc.Height = WINDOW_HEIGHT;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    IDXGIFactory4* factory = nullptr;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    IDXGISwapChain1* swapChain1 = nullptr;
    factory->CreateSwapChainForHwnd(
        commandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );

    swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
    swapChain1->Release();
    factory->Release();

    // Создание дескрипторного кучи для RTV
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Создание RTV для каждого буфера
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < FRAME_COUNT; n++) {
        swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
        device->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    // Создание командного аллокатора
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

    // Создание командного листа
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();

    // Создание fence
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    fenceValue = 1;
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    return true;
}

void Update() {
    // Здесь будет логика обновления
}

void Render() {
    // Сброс командного аллокатора и список
    commandAllocator->Reset();
    commandList->Reset(commandAllocator, nullptr);

    // Устанавливаем render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = renderTargets[frameIndex];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += frameIndex * rtvDescriptorSize;
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Очистка render target
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Переход к состоянию present
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList->ResourceBarrier(1, &barrier);

    // Закрытие и выполнение командного списка
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Представление кадра
    swapChain->Present(1, 0);

    WaitForPreviousFrame();
}

void WaitForPreviousFrame() {
    // Поднимаем сигнал и ждем когда GPU закончит все команды
    const UINT64 currentFenceValue = fenceValue;
    commandQueue->Signal(fence, currentFenceValue);
    fenceValue++;

    if (fence->GetCompletedValue() < currentFenceValue) {
        fence->SetEventOnCompletion(currentFenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}
void OnResize(HWND hwnd, UINT width, UINT height) {
    // Ждем завершения всех команд
    WaitForPreviousFrame();

    // Освобождаем старые render targets
    for (int i = 0; i < FRAME_COUNT; i++) {
        if (renderTargets[i]) renderTargets[i]->Release();
        renderTargets[i] = nullptr;
    }

    // Изменяем размер swap chain
    DXGI_SWAP_CHAIN_DESC desc = {};
    swapChain->GetDesc(&desc);
    swapChain->ResizeBuffers(FRAME_COUNT, width, height, desc.BufferDesc.Format, desc.Flags);

    // Пересоздаем render target views
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < FRAME_COUNT; n++) {
        swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
        device->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void CleanupD3D12() {
    // Ждем завершения всех команд перед очисткой
    WaitForPreviousFrame();
    CloseHandle(fenceEvent);

    // Освобождаем ресурсы
    for (int i = 0; i < FRAME_COUNT; i++) {
        if (renderTargets[i]) renderTargets[i]->Release();
    }
    if (fence) fence->Release();
    if (rtvHeap) rtvHeap->Release();
    if (swapChain) swapChain->Release();
    if (commandList) commandList->Release();
    if (commandAllocator) commandAllocator->Release();
    if (commandQueue) commandQueue->Release();
    if (device) device->Release();
}