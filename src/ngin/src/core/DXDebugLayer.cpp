#if defined(_WIN32)

#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>

#include <cstdio>
#include <format>

bool DXDebugLayer::Init() {
#ifdef NGIN_DEBUG
  HRESULT res = D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug));

  if (FAILED(res)) {
    Ngin::logDebug(std::format("D3D12Debug Interface could not be created: {}", res));

    return false;
  }

  m_d3d12Debug->EnableDebugLayer();

  res = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug));

  if (FAILED(res)) {
    Ngin::logDebug(std::format("DXGIDebug Interface could not be created: {}", res));

    return false;
  }

  m_dxgiDebug->EnableLeakTrackingForThread();

  return true;
#else
  return false;
#endif
}

bool DXDebugLayer::InitDevice(ID3D12Device* device) {
#ifdef NGIN_DEBUG
  if (!device)
    return false;

  HRESULT res = device->QueryInterface(IID_PPV_ARGS(&m_infoQueue));

  if (FAILED(res)) {
    Ngin::logDebug(std::format("ID3D12InfoQueue1 could not be created: {}", res));

    return false;
  }

  res = m_infoQueue->RegisterMessageCallback(&DXDebugLayer::DebugMessageCallback,
      D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &m_callbackCookie);

  if (FAILED(res)) {
    Ngin::logDebug(std::format("D3D12 debug callback could not be registered: {}", res));

    m_infoQueue.reset();
    return false;
  }

  return true;
#else
  (void)device;
  return false;
#endif
}

void CALLBACK DXDebugLayer::DebugMessageCallback(D3D12_MESSAGE_CATEGORY category,
    D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR description, void* context) {
#ifdef NGIN_DEBUG
  (void)category;
  (void)id;
  (void)context;

  const char* severityString = "UNKNOWN";

  switch (severity) {
    case D3D12_MESSAGE_SEVERITY_CORRUPTION:
      severityString = "CORRUPTION";
      break;

    case D3D12_MESSAGE_SEVERITY_ERROR:
      severityString = "ERROR";
      break;

    case D3D12_MESSAGE_SEVERITY_WARNING:
      severityString = "WARNING";
      break;

    case D3D12_MESSAGE_SEVERITY_INFO:
      severityString = "INFO";
      break;

    case D3D12_MESSAGE_SEVERITY_MESSAGE:
      severityString = "MESSAGE";
      break;
  }

  std::fprintf(stderr, "[D3D12 %s] %s\n", severityString, description ? description : "");

  std::fflush(stderr);
#endif
}

void DXDebugLayer::Report() {
#ifdef NGIN_DEBUG
  if (!m_dxgiDebug)
    return;

  // ReportLiveObjects uses debugger output rather than the
  // ID3D12InfoQueue callback, so keep the DBWIN capture scoped
  // specifically to this operation.
  Ngin::DebugOutputToStdout dbg;

  OutputDebugStringW(L"DXGI Reports living device objects:\n");

  m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
      DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
#endif
}

void DXDebugLayer::Shutdown() {
#ifdef NGIN_DEBUG
  // Unregister before releasing the info queue/device.
  if (m_infoQueue && m_callbackCookie != 0) {
    m_infoQueue->UnregisterMessageCallback(m_callbackCookie);

    m_callbackCookie = 0;
  }

  m_infoQueue.reset();

  if (m_dxgiDebug) {
    Ngin::DebugOutputToStdout dbg;

    OutputDebugStringW(L"DXGI Reports living device objects:\n");

    m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
        DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
  }

  m_dxgiDebug.reset();
  m_d3d12Debug.reset();
#endif
}

#endif
