#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <ngin/pch.h>
#include <ngin/rhi/shader.h>
#include <ngin/rhi_dx12/fence.h>

namespace Ngin {
class RHI : public NonCopyableNonMovable {
 public:
  RHI() = delete;
  static Error Create(HWND hwnd, Vec2Rounded windowDimensions, bool& windowed, Scope<RHI>& rhi);

 public:
  Error Update();
  Error ExecuteCommandList();
  Error Present();
  Error BeginFrame();
  Error EndFrame();
  Error Resize(uint16_t width, uint16_t height);

  inline void DoPSO() {
    cmdList->SetPipelineState(pipelineState);
    cmdList->SetGraphicsRootSignature(rootSignature);
  }
  inline void DoIA() {
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  }
  void DoRS(Vec2Rounded windowDimensions);
  // currently not working cuz of blend settings
  inline void DoOM() {
    f32 bf[] = {0.1f, 0.1f, 0.1f, 0.1f};
    cmdList->OMSetBlendFactor(bf);
  }

  inline void Draw() { cmdList->DrawInstanced(_countof(verticies), 1, 0, 0); }

  Error GetBuffers();

  inline void ReleaseBuffers() {
    for (size_t i = 0; i < FRAME_COUNT; i++) {
      buffers[i].reset();
    }
  }

 private:
  RHI(HWND hwnd, Vec2Rounded windowDimensions, bool& windowed, Error& err);

 private:
  static constexpr uint32_t SWAPCHAIN_FLAGS =
    DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
  static constexpr DXGI_FORMAT COLOR_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

  // Temp
  constexpr static Vertex verticies[] = {
    // T1
    {-1.f, -1.f}, {0.f, 1.f}, {1.f, -1.f}
  };

 private:
  ComScope<ID3D12Device10> device;
  ComScope<ID3D12CommandQueue> cmdQueue;
  Scope<Fence> fence;
  ComScope<ID3D12CommandAllocator> cmdAlloc;
  ComScope<ID3D12GraphicsCommandList7> cmdList;
  ComScope<IDXGIFactory7> factory;
  ComScope<IDXGISwapChain4> swapChain;
  ComScope<ID3D12DescriptorHeap> rtvHeap;
  Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT> rtvHandles;
  ComScope<ID3D12Resource2> uploadBuffer, vertexBuffer;
  ComScope<ID3D12RootSignature> rootSignature;
  ComScope<ID3D12PipelineState> pipelineState;
  D3D12_VERTEX_BUFFER_VIEW vbv;
  Array<ComScope<ID3D12Resource2>, FRAME_COUNT> buffers;
  uint8_t currentBuffer = 0;
  bool& windowed;

 private:
  // Creation of the attributes
  Error createCmdQueue();
  Error createFence();
  Error createCommandAllocator();
  Error CreateCommandList();
  Error CreateSwapChain(HWND hwnd, Vec2Rounded windowDimensions, bool windowed);
  Error CreateRtvHeap();
  Error CreateUploadAndVertexBuffer();
};
}  // namespace Ngin
