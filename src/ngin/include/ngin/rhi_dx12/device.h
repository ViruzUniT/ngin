#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/types.h>
#include <ngin/pch.h>

#include <utility>
namespace Ngin {
struct RHI : NonCopyable {
 public:
  RHI() = delete;
  static HRESULT Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, Scope<RHI>& rhi);

  RHI(RHI&& other) : fenceEvent(other.fenceEvent) {
    device = std::move(other.device);
    DXDebugLayer::Get().InitDevice(this->device);
    cmdQueue = std::move(other.cmdQueue);
    fence = std::move(other.fence);
    swapChain = std::move(other.swapChain);
    other.ReleaseBuffers();
    GetBuffers(device, swapChain, buffers, rtvHeap, rtvHandles);
    cmdAlloc = std::move(other.cmdAlloc);
    cmdList = std::move(other.cmdList);
    rtvHeap = std::move(other.rtvHeap);
    factory = std::move(other.factory);
    rootSignature = std::move(other.rootSignature);
    renderTargets = std::move(other.renderTargets);
    pipelineState = std::move(other.pipelineState);
  }
  RHI& operator=(RHI&& other) {
    if (this == &other)
      return *this;

    device = std::move(other.device);
    DXDebugLayer::Get().InitDevice(this->device);
    cmdQueue = std::move(other.cmdQueue);
    fence = std::move(other.fence);
    swapChain = std::move(other.swapChain);
    other.ReleaseBuffers();
    GetBuffers(device, swapChain, buffers, rtvHeap, rtvHandles);
    cmdAlloc = std::move(other.cmdAlloc);
    cmdList = std::move(other.cmdList);
    rtvHeap = std::move(other.rtvHeap);
    factory = std::move(other.factory);
    rootSignature = std::move(other.rootSignature);
    renderTargets = std::move(other.renderTargets);
    pipelineState = std::move(other.pipelineState);
    fenceEvent = other.fenceEvent;
    return *this;
  }

 private:
  template <size_t N>
  RHI(ComScope<ID3D12Device10>& device, ComScope<ID3D12CommandQueue>& cmdQueue,
      ComScope<ID3D12Fence1>& fence, HANDLE fenceEvent, ComScope<IDXGISwapChain4>& swapChain,
      ComScope<ID3D12CommandAllocator>& cmdAlloc, ComScope<ID3D12GraphicsCommandList7>& cmdList,
      ComScope<ID3D12DescriptorHeap>& rtvHeap, Array<D3D12_CPU_DESCRIPTOR_HANDLE, N> rtvHandles,
      ComScope<IDXGIFactory7>& factory, ComScope<ID3D12RootSignature>& rootSignature,
      ComScope<ID3D12PipelineState>& pipelineState, List<ComScope<ID3D12Resource>>& renderTargets)
      : fenceEvent(fenceEvent) {
    this->device = std::move(device);
    DXDebugLayer::Get().InitDevice(this->device);
    this->cmdQueue = std::move(cmdQueue);
    this->fence = std::move(fence);
    this->swapChain = std::move(swapChain);
    this->rtvHeap = std::move(rtvHeap);

    for (size_t i = 0; i < FRAME_COUNT; i++) {
      this->rtvHandles[i] = rtvHandles[i];
    }
    GetBuffers(this->device, this->swapChain, this->buffers, this->rtvHeap, this->rtvHandles);

    this->cmdAlloc = std::move(cmdAlloc);
    this->cmdList = std::move(cmdList);
    this->factory = std::move(factory);
    this->rootSignature = std::move(rootSignature);
    this->renderTargets = std::move(renderTargets);
    this->pipelineState = std::move(pipelineState);
  }

 public:
  ~RHI() {
    Flush(FRAME_COUNT);
    if (fenceEvent)
      CloseHandle(fenceEvent);
  }

  Error Update();
  Error SignalAndWait();
  Error ExecuteCommandList();
  Error Present();
  Error BeginFrame();
  Error EndFrame();
  Error Resize(uint16_t width, uint16_t height);
  inline Error GetBuffers() {
    HRESULT hr = GetBuffers(device, swapChain, buffers, rtvHeap, rtvHandles);
    if (FAILED(hr))
      return Error{Unknown, std::format("Getting the buffers failed: {}", hr)};
    return Error{};
  }
  inline void ReleaseBuffers() {
    for (size_t i = 0; i < FRAME_COUNT; i++) {
      buffers[i].reset();
    }
  }

  inline void Flush(size_t count) {
    for (size_t i = 0; i < count; i++) {
      SignalAndWait();
    }
  }

 public:
  static constexpr uint8_t FRAME_COUNT = 2;

 private:
  uint64_t fenceValue = 0;
  HANDLE fenceEvent;

  ComScope<ID3D12Device10> device;
  ComScope<ID3D12CommandQueue> cmdQueue;
  ComScope<ID3D12Fence1> fence;
  ComScope<IDXGISwapChain4> swapChain;
  ComScope<ID3D12Resource2> buffers[FRAME_COUNT];
  uint8_t currentBuffer = 0;
  ComScope<ID3D12CommandAllocator> cmdAlloc;
  ComScope<ID3D12GraphicsCommandList7> cmdList;
  ComScope<ID3D12DescriptorHeap> rtvHeap;
  Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT> rtvHandles;
  ComScope<IDXGIFactory7> factory;
  ComScope<ID3D12RootSignature> rootSignature;
  ComScope<ID3D12PipelineState> pipelineState;
  List<ComScope<ID3D12Resource>> renderTargets;

  static const uint32_t SWAPCHAIN_FLAGS =
      DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  static HRESULT CreateCommandQueue(ID3D12Device10* device,
      ComScope<ID3D12CommandQueue>& cmdQueue);
  static HRESULT CreateFence(ID3D12Device10* device, const uint64_t fenceValue,
      ComScope<ID3D12Fence1>& fence, HANDLE& fenceEvent);
  static HRESULT CreateCommandAllocator(ID3D12Device10* device,
      ComScope<ID3D12CommandAllocator>& cmdAlloc);
  static HRESULT CreateGraphicsCommandList7(ID3D12Device10* device,
      ComScope<ID3D12GraphicsCommandList7>& cmdList, ID3D12CommandAllocator* cmdAlloc);
  static HRESULT CreateSwapChain(IDXGIFactory7* factory, ComScope<IDXGISwapChain4>& swapChain,
      ID3D12CommandQueue* cmdQueue, uint16_t width, uint16_t height, HWND hwnd, bool windowed);
  static HRESULT GetBuffers(ID3D12Device10* device, ComScope<IDXGISwapChain4>& swapChain,
      ComScope<ID3D12Resource2> buffers[], ID3D12DescriptorHeap* rtvHeap,
      Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT>& rtvHandles);
  static HRESULT CreateRtvHeap(ID3D12Device10* device, IDXGISwapChain4* swapChain,
      ComScope<ID3D12DescriptorHeap>& rtvHeap,
      Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT>& rtvHandles);
  static HRESULT CreateSignature(ID3D12Device10* device,
      ComScope<ID3D12RootSignature>& rootSignature, ComScope<ID3DBlob>& signatureBlob,
      ComScope<ID3DBlob>& errorBlob);
  static HRESULT CreatePipeline(ID3D12Device10* device, ID3D12RootSignature* rootSignature,
      ComScope<ID3D12PipelineState>& pipelineState);
  static HRESULT CreateCommandList(ID3D12Device10* device,
      ComScope<ID3D12GraphicsCommandList7>& cmdList, ID3D12CommandAllocator* cmdAlloc);
};
}  // namespace Ngin
