#pragma once
#include <d3d12.h>
#include <ngin/core/utility.h>
#include <ngin/pch.h>

namespace Ngin {
struct RHI : NonCopyable {
  RHI() = delete;
  static RHI* Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight);

 private:
  RHI(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, IDXGISwapChain* swapChain,
      ID3D12CommandAllocator* cmdAlloc, ID3D12CommandList* cmdList, ID3D12DescriptorHeap* rtvHeap)
      : Device(device),
        CmdQueue(cmdQueue),
        SwapChain(swapChain),
        CmdAlloc(cmdAlloc),
        CmdList(cmdList),
        RtvHeap(rtvHeap) {}

 public:
  ID3D12Device* Device;
  ID3D12CommandQueue* CmdQueue;
  IDXGISwapChain* SwapChain;
  ID3D12CommandAllocator* CmdAlloc;
  ID3D12CommandList* CmdList;
  ID3D12DescriptorHeap* RtvHeap;

 private:
  static HRESULT CreateCommandQueue(ID3D12Device* device, ID3D12CommandQueue* cmdQueue);
  static HRESULT CreateCommandAllocator(ID3D12Device* device, ID3D12CommandAllocator* cmdAlloc);
  static HRESULT CreateCommandList(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
      ID3D12CommandAllocator* cmdAlloc);
  static HRESULT CreateSwapChain(IDXGIFactory* factory, IDXGISwapChain* swapChain,
      ID3D12CommandQueue* cmdQueue, uint16_t windowWidth, uint16_t windowHeight, HWND hwnd,
      bool windowed);
  static HRESULT CreateRtvHeap(ID3D12Device* device, IDXGISwapChain* swapChain,
      ID3D12DescriptorHeap* rtvHeap);
};
}  // namespace Ngin
