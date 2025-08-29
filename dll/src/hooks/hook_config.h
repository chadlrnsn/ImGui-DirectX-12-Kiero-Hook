#pragma once

namespace Hook {
    enum class Backend {
        D3D12,
        D3D11
    };

    // Global toggle to force a specific graphics backend hook.
    // Set this before initialization in dllmain.
    inline Backend g_ForceBackend = Backend::D3D11; // Change to Backend::D3D11 to force DX11
}

