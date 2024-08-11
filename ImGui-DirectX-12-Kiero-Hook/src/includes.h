#pragma once
#include <Windows.h>
#include <vector>
#include <iostream>

// MinHook
#include <minhook/include/MinHook.h>


// Imgui
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <imgui_internal.h>

// Dx
#include <d3d12.h>
#include <dxgi1_4.h>

#include <filesystem>
#include <mutex>

std::mutex mutex;