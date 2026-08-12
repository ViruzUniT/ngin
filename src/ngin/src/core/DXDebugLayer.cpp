#if defined(_WIN32)
#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>

bool DXDebugLayer::Init() {
#ifdef NGIN_DEBUG
  ID3D12Debug5* temp3Debug = nullptr;
  HRESULT res = D3D12GetDebugInterface(IID_PPV_ARGS(&temp3Debug));
  if (SUCCEEDED(res)) {
    m_d3d12Debug.reset(temp3Debug);
    temp3Debug = nullptr;

    m_d3d12Debug->EnableDebugLayer();

    IDXGIDebug1* tempxDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&tempxDebug)))) {
      m_dxgiDebug.reset(tempxDebug);
      tempxDebug = nullptr;

      m_dxgiDebug->EnableLeakTrackingForThread();
      return true;
    } else {
      Ngin::logDebug(std::format("DXGIDebug Interface could not be created: {}", res));
    }
  } else {
    Ngin::logDebug(std::format("D3D12Debug Interface could not be created: {} ", res));
  }
#endif
  return false;
}
void DXDebugLayer::Shutdown() {
#ifdef NGIN_DEBUG
  if (m_dxgiDebug) {
    OutputDebugStringW(L"DXGI Reports living device objects:\n");
    m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
        DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
  }
  m_dxgiDebug.reset(nullptr);
  m_d3d12Debug.reset(nullptr);
#endif
}
#endif
