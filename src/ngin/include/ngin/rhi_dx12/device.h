#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <ngin/core/types.h>
#include <ngin/pch.h>

#include <utility>
namespace Ngin {
struct RHI : NonCopyable {
 public:
  RHI() = delete;
  static HRESULT Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, Scope<RHI>& rhi);

  RHI(RHI&& other) : FenceEvent(other.FenceEvent) {
    Device = std::move(other.Device);
    CmdQueue = std::move(other.CmdQueue);
    Fence = std::move(other.Fence);
    SwapChain = std::move(other.SwapChain);
    CmdAlloc = std::move(other.CmdAlloc);
    CmdList = std::move(other.CmdList);
    RtvHeap = std::move(other.RtvHeap);
    Factory = std::move(other.Factory);
    RootSignature = std::move(other.RootSignature);
    RenderTargets = std::move(other.RenderTargets);
    PipelineState = std::move(other.PipelineState);
  }
  RHI& operator=(RHI&& other) {
    if (this == &other)
      return *this;

    Device = std::move(other.Device);
    CmdQueue = std::move(other.CmdQueue);
    Fence = std::move(other.Fence);
    SwapChain = std::move(other.SwapChain);
    CmdAlloc = std::move(other.CmdAlloc);
    CmdList = std::move(other.CmdList);
    RtvHeap = std::move(other.RtvHeap);
    Factory = std::move(other.Factory);
    RootSignature = std::move(other.RootSignature);
    RenderTargets = std::move(other.RenderTargets);
    PipelineState = std::move(other.PipelineState);
    FenceEvent = other.FenceEvent;
    return *this;
  }

  RHI(ComScope<ID3D12Device10>& device, ComScope<ID3D12CommandQueue>& cmdQueue,
      ComScope<ID3D12Fence1>& fence, HANDLE fenceEvent, ComScope<IDXGISwapChain4>& swapChain,
      ComScope<ID3D12CommandAllocator>& cmdAlloc, ComScope<ID3D12GraphicsCommandList7>& cmdList,
      ComScope<ID3D12DescriptorHeap>& rtvHeap, ComScope<IDXGIFactory7>& factory,
      ComScope<ID3D12RootSignature>& rootSignature, ComScope<ID3D12PipelineState>& pipelineState,
      List<ComScope<ID3D12Resource>>& renderTargets)
      : FenceEvent(fenceEvent) {
    Device = std::move(device);
    CmdQueue = std::move(cmdQueue);
    Fence = std::move(fence);
    SwapChain = std::move(swapChain);
    CmdAlloc = std::move(cmdAlloc);
    CmdList = std::move(cmdList);
    RtvHeap = std::move(rtvHeap);
    Factory = std::move(factory);
    RootSignature = std::move(rootSignature);
    RenderTargets = std::move(renderTargets);
    PipelineState = std::move(pipelineState);
  }

  ~RHI() {
    Flush(GetFrameCount());
    if (FenceEvent)
      CloseHandle(FenceEvent);
  }

  Error Update();
  Error SignalAndWait();
  Error ExecuteCommandList();
  Error Present();

  inline void Flush(size_t count) {
    for (size_t i = 0; i < count; i++) {
      SignalAndWait();
    }
  }

 public:
  static uint8_t GetFrameCount() { return 2; }

 private:
  uint64_t FenceValue = 0;
  HANDLE FenceEvent;

  ComScope<ID3D12Device10> Device;
  ComScope<ID3D12CommandQueue> CmdQueue;
  ComScope<ID3D12Fence1> Fence;
  ComScope<IDXGISwapChain4> SwapChain;
  ComScope<ID3D12CommandAllocator> CmdAlloc;
  ComScope<ID3D12GraphicsCommandList7> CmdList;
  ComScope<ID3D12DescriptorHeap> RtvHeap;
  ComScope<IDXGIFactory7> Factory;
  ComScope<ID3D12RootSignature> RootSignature;
  ComScope<ID3D12PipelineState> PipelineState;
  List<ComScope<ID3D12Resource>> RenderTargets;

  static HRESULT CreateCommandQueue(ID3D12Device10* device,
      ComScope<ID3D12CommandQueue>& cmdQueue);
  static HRESULT CreateFence(ID3D12Device10* device, const uint64_t fenceValue,
      ComScope<ID3D12Fence1>& fence, HANDLE& fenceEvent);
  static HRESULT CreateCommandAllocator(ID3D12Device10* device,
      ComScope<ID3D12CommandAllocator>& cmdAlloc);
  static HRESULT CreateGraphicsCommandList7(ID3D12Device10* device,
      ComScope<ID3D12GraphicsCommandList7>& cmdList, ID3D12CommandAllocator* cmdAlloc);
  static HRESULT CreateSwapChain(IDXGIFactory7* factory, ComScope<IDXGISwapChain4>& swapChain,
      ID3D12CommandQueue* cmdQueue, uint16_t width, uint16_t geight, HWND hwnd, bool windowed);
  static HRESULT CreateRtvHeap(ID3D12Device10* device, IDXGISwapChain4* swapChain,
      ComScope<ID3D12DescriptorHeap>& rtvHeap, List<ComScope<ID3D12Resource>>& renderTargets);
  static HRESULT CreateSignature(ID3D12Device10* device,
      ComScope<ID3D12RootSignature>& rootSignature, ComScope<ID3DBlob>& signatureBlob,
      ComScope<ID3DBlob>& errorBlob);
  static HRESULT CreatePipeline(ID3D12Device10* device, ID3D12RootSignature* rootSignature,
      ComScope<ID3D12PipelineState>& pipelineState);
  static HRESULT CreateCommandList(ID3D12Device10* device,
      ComScope<ID3D12GraphicsCommandList7>& cmdList, ID3D12CommandAllocator* cmdAlloc);
};
}  // namespace Ngin
