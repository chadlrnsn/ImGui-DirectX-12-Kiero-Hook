#pragma once
#include <logger/logger.h>
void WaitForLastSubmittedFrame();
void CleanupRenderTarget();
void InitD3D12Hook();
void ReleaseD3D12Hook();