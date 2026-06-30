#pragma once
#include <d3d12.h>
#include <ngin/core/utility.h>
#include <ngin/pch.h>

#include <utility>

#include "ngin/rhi_dx12/shader.h"

namespace Ngin {
struct RHI : NonCopyable {
 public:
  RHI() = delete;
  static HRESULT Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, Scope<RHI>& rhi);

  RHI(RHI&& other) {
    Device = std::move(other.Device);
    CmdQueue = std::move(other.CmdQueue);
    SwapChain = std::move(other.SwapChain);
    CmdAlloc = std::move(other.CmdAlloc);
    CmdList = std::move(other.CmdList);
    RtvHeap = std::move(other.RtvHeap);
    Factory = std::move(other.Factory);
    RootSignature = std::move(other.RootSignature);
    RenderTargets = std::move(other.RenderTargets);
  }
  RHI& operator=(RHI&& other) {
    if (this == &other)
      return *this;

    Device = std::move(other.Device);
    CmdQueue = std::move(other.CmdQueue);
    SwapChain = std::move(other.SwapChain);
    CmdAlloc = std::move(other.CmdAlloc);
    CmdList = std::move(other.CmdList);
    RtvHeap = std::move(other.RtvHeap);
    Factory = std::move(other.Factory);
    RootSignature = std::move(other.RootSignature);
    RenderTargets = std::move(other.RenderTargets);
    return *this;
  }

 private:
  RHI(ComScope<ID3D12Device> device, ComScope<ID3D12CommandQueue> cmdQueue,
      ComScope<IDXGISwapChain> swapChain, ComScope<ID3D12CommandAllocator> cmdAlloc,
      ComScope<ID3D12CommandList> cmdList, ComScope<ID3D12DescriptorHeap> rtvHeap,
      ComScope<IDXGIFactory> factory, ComScope<ID3D12RootSignature> rootSignature,
      List<ComScope<ID3D12Resource>> renderTargets) {
    Device = std::move(device);
    CmdQueue = std::move(cmdQueue);
    SwapChain = std::move(swapChain);
    CmdAlloc = std::move(cmdAlloc);
    CmdList = std::move(cmdList);
    RtvHeap = std::move(rtvHeap);
    Factory = std::move(factory);
    RootSignature = std::move(rootSignature);
    RenderTargets = std::move(renderTargets);
  }

 public:
  ComScope<ID3D12Device> Device;
  ComScope<ID3D12CommandQueue> CmdQueue;
  ComScope<IDXGISwapChain> SwapChain;
  ComScope<ID3D12CommandAllocator> CmdAlloc;
  ComScope<ID3D12CommandList> CmdList;
  ComScope<ID3D12DescriptorHeap> RtvHeap;
  ComScope<IDXGIFactory> Factory;
  ComScope<ID3D12RootSignature> RootSignature;
  List<ComScope<ID3D12Resource>> RenderTargets;

 private:
  static HRESULT CreateCommandQueue(ID3D12Device* device, ID3D12CommandQueue*& cmdQueue);
  static HRESULT CreateCommandAllocator(ID3D12Device* device, ID3D12CommandAllocator*& cmdAlloc);
  static HRESULT CreateCommandList(ID3D12Device* device, ID3D12GraphicsCommandList*& cmdList,
      ID3D12CommandAllocator* cmdAlloc);
  static HRESULT CreateSwapChain(IDXGIFactory* factory, IDXGISwapChain*& swapChain,
      ID3D12CommandQueue* cmdQueue, uint16_t windowWidth, uint16_t windowHeight, HWND hwnd,
      bool windowed);
  static HRESULT CreateRtvHeap(ID3D12Device* device, IDXGISwapChain* swapChain,
      ID3D12DescriptorHeap*& rtvHeap, List<ComScope<ID3D12Resource>>& renderTargets);
  static HRESULT CreateSignature(ID3D12Device* device,
      ComScope<ID3D12RootSignature>& rootSignature, ComScope<ID3DBlob>& signatureBlob,
      ComScope<ID3DBlob>& errorBlob);
  static HRESULT CreatePipeline(ID3D12Device* device, ID3D12RootSignature* rootSignature,
      ID3D12PipelineState*& pipelineState);
};
}  // namespace Ngin
