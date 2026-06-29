#include <ngin/rhi_dx12/device.h>

namespace Ngin {
HRESULT RHI::Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, RHI*& rhi) {
  ID3D12Device* device = nullptr;
  ID3D12CommandQueue* cmdQueue = nullptr;
  IDXGISwapChain* swapChain = nullptr;
  ID3D12CommandAllocator* cmdAlloc = nullptr;
  ID3D12GraphicsCommandList* cmdList = nullptr;
  ID3D12DescriptorHeap* rtvHeap = nullptr;

  // TODO: handle that
  HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device));
  if (FAILED(hr))
    return hr;

  hr = CreateCommandQueue(device, cmdQueue);
  if (FAILED(hr))
    return hr;
  hr = CreateCommandAllocator(device, cmdAlloc);
  if (FAILED(hr))
    return hr;
  hr = CreateCommandList(device, cmdList, cmdAlloc);
  if (FAILED(hr))
    return hr;

  IDXGIFactory* factory = nullptr;
  hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  if (FAILED(hr))
    return hr;
  hr = CreateSwapChain(factory, swapChain, cmdQueue, windowWidth, windowHeight, hwnd, true);
  if (FAILED(hr))
    return hr;
  hr = CreateRtvHeap(device, swapChain, rtvHeap);
  if (FAILED(hr))
    return hr;

  rhi = new RHI(device, cmdQueue, swapChain, cmdAlloc, cmdList, rtvHeap, factory);
  return hr;
}

HRESULT RHI::CreateCommandQueue(ID3D12Device* device, ID3D12CommandQueue*& cmdQueue) {
  D3D12_COMMAND_QUEUE_DESC cmdQueDesc = {};
  cmdQueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  cmdQueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  return device->CreateCommandQueue(&cmdQueDesc, IID_PPV_ARGS(&cmdQueue));
}

HRESULT RHI::CreateCommandAllocator(ID3D12Device* device, ID3D12CommandAllocator*& cmdAlloc) {
  HRESULT hr =
      device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
  if (FAILED(hr))
    return hr;
  return cmdAlloc->Reset();
}

HRESULT RHI::CreateCommandList(ID3D12Device* device, ID3D12GraphicsCommandList*& cmdList,
    ID3D12CommandAllocator* cmdAlloc) {
  HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr,
      IID_PPV_ARGS(&cmdList));
  if (FAILED(hr))
    return hr;
  return cmdList->Close();
}

HRESULT RHI::CreateSwapChain(IDXGIFactory* factory, IDXGISwapChain*& swapChain,
    ID3D12CommandQueue* cmdQueue, uint16_t windowWidth, uint16_t windowHeight, HWND hwnd,
    bool windowed) {
  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferCount = 2;
  swapChainDesc.BufferDesc.Width = windowWidth;
  swapChainDesc.BufferDesc.Height = windowHeight;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapChainDesc.OutputWindow = hwnd;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.Windowed = windowed;

  IDXGISwapChain* tempSwapChain = nullptr;
  HRESULT hr = factory->CreateSwapChain(cmdQueue, &swapChainDesc, &tempSwapChain);
  if (FAILED(hr))
    return hr;

  hr = tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
  if (FAILED(hr))
    return hr;
  tempSwapChain->Release();
  tempSwapChain = nullptr;
  return hr;
}

HRESULT RHI::CreateRtvHeap(ID3D12Device* device, IDXGISwapChain* swapChain,
    ID3D12DescriptorHeap*& rtvHeap) {
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = 2;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
  if (FAILED(hr))
    return hr;

  ID3D12Resource* render_targets[2];

  UINT rtv_increment_size =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  {
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; i++) {
      hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&render_targets[i]));
      if (FAILED(hr))
        return hr;

      device->CreateRenderTargetView(render_targets[i], nullptr, rtv_handle);
      rtv_handle.ptr += rtv_increment_size;
    }
  }
  return hr;
}
}  // namespace Ngin
