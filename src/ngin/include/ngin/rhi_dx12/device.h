#pragma once
#include <d3d12.h>
#include <ngin/core/utility.h>
#include <ngin/pch.h>

#include <utility>

namespace Ngin {
struct RHI : NonCopyable {
 public:
  RHI() = delete;
  static HRESULT Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, RHI*& rhi);

  RHI(RHI&& other) {
    Device = std::move(other.Device);
    CmdQueue = std::move(other.CmdQueue);
    SwapChain = std::move(other.SwapChain);
    CmdAlloc = std::move(other.CmdAlloc);
    CmdList = std::move(other.CmdList);
    RtvHeap = std::move(other.RtvHeap);
    Factory = std::move(other.Factory);
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
    return *this;
  }

 private:
  RHI(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, IDXGISwapChain* swapChain,
      ID3D12CommandAllocator* cmdAlloc, ID3D12CommandList* cmdList, ID3D12DescriptorHeap* rtvHeap,
      IDXGIFactory* factory) {
    Device.reset(device);
    CmdQueue.reset(cmdQueue);
    SwapChain.reset(swapChain);
    CmdAlloc.reset(cmdAlloc);
    CmdList.reset(cmdList);
    RtvHeap.reset(rtvHeap);
    Factory.reset(factory);
  }

 public:
  ComScope<ID3D12Device> Device;
  ComScope<ID3D12CommandQueue> CmdQueue;
  ComScope<IDXGISwapChain> SwapChain;
  Scope<ID3D12CommandAllocator> CmdAlloc;
  Scope<ID3D12CommandList> CmdList;
  Scope<ID3D12DescriptorHeap> RtvHeap;
  Scope<IDXGIFactory> Factory;

 private:
  static HRESULT CreateCommandQueue(ID3D12Device* device, ID3D12CommandQueue*& cmdQueue);
  static HRESULT CreateCommandAllocator(ID3D12Device* device, ID3D12CommandAllocator*& cmdAlloc);
  static HRESULT CreateCommandList(ID3D12Device* device, ID3D12GraphicsCommandList*& cmdList,
      ID3D12CommandAllocator* cmdAlloc);
  static HRESULT CreateSwapChain(IDXGIFactory* factory, IDXGISwapChain*& swapChain,
      ID3D12CommandQueue* cmdQueue, uint16_t windowWidth, uint16_t windowHeight, HWND hwnd,
      bool windowed);
  static HRESULT CreateRtvHeap(ID3D12Device* device, IDXGISwapChain* swapChain,
      ID3D12DescriptorHeap*& rtvHeap);
};
}  // namespace Ngin
