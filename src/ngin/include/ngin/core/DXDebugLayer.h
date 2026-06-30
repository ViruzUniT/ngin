#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <ngin/pch.h>

#ifdef NGIN_DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <wincodec.h>
class DXDebugLayer : Ngin::NonCopyable {
 public:
  bool Init();
  void Shutdown();

 private:
#ifdef NGIN_DEBUG
  Ngin::ComScope<ID3D12Debug5> m_d3d12Debug;
  Ngin::ComScope<IDXGIDebug1> m_dxgiDebug;
#endif

 public:
  inline static DXDebugLayer& Get() {
    static DXDebugLayer instance;
    return instance;
  }

 private:
  DXDebugLayer() = default;
};
