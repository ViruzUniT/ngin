#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <ngin/rhi_dx12/rhi.h>

namespace Ngin {
struct Fence : NonCopyable {
  Fence() = delete;
  Fence(ID3D12Device10* device, ID3D12CommandQueue* cmdQueue, Error& err,
    uint32_t timeBeforeTimeoutMS = 20000)
      : cmdQueue(cmdQueue), timeBeforeTimeoutMS(timeBeforeTimeoutMS) {
    if (cmdQueue == nullptr) {
      err = Error{InvalidArgument, "Creating fence Failed. cmdQueue was a nullptr"};
    }
    HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
      err = Error{InvalidArgumentIGuess, std::format("Creating fence failed: {}", hr)};
      return;
    }

    fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    if (fenceEvent == NULL) {
      err =
        Error{FenceError, std::format("fenceEvent is null, mby this helps: {}", GetLastError())};
    }
  }

  ~Fence() {
    logTrace("Destructing Fence");
    Flush(FRAME_COUNT);
    if (fenceEvent)
      CloseHandle(fenceEvent);
  }

  inline void Flush(size_t count) {
    for (size_t i = 0; i < count; i++) {
      SignalAndWait();
    }
  }
  uint64_t fenceValue = 0;

  inline ID3D12Fence1* getFence() const noexcept { return fence.get(); }

  inline Error SignalAndWait() {
    auto err = validate();
    if (err)
      return err;

    HRESULT hr = cmdQueue->Signal(fence.get(), ++fenceValue);
    if (FAILED(hr))
      return Error{
        InvalidArgumentIGuess, std::format("Signaling the Command Queue failed: {}", hr)
      };

    hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
    if (FAILED(hr))
      return Error{FenceError, std::format("Setting the event failed: {}", hr)};

    DWORD res = WaitForSingleObject(fenceEvent, timeBeforeTimeoutMS);
    if (res != WAIT_OBJECT_0)
      return Error{FenceTimeout, std::format("Fence timed out: {}", res)};

    return Error{};
  }

  inline Error validate() const noexcept {
    if (fence == nullptr)
      return Error{FenceError, "Fence was not created correctly"};
    if (fenceEvent == NULL)
      return Error{FenceError, "FenceEvent was not created correctly"};
    if (cmdQueue == nullptr)
      return Error{InvalidArgument, "cmdQueue is nullptr"};

    return Error{};
  }
  Fence& operator=(Fence&& other) {
    if (this == &other)
      return *this;

    fenceValue = other.fenceValue;
    timeBeforeTimeoutMS = other.timeBeforeTimeoutMS;
    fenceEvent = other.fenceEvent;
    fence = std::move(other.fence);
    cmdQueue = other.cmdQueue;
    return *this;
  }

 private:
  DWORD timeBeforeTimeoutMS;
  HANDLE fenceEvent;
  ComScope<ID3D12Fence1> fence;

  ID3D12CommandQueue* cmdQueue;
};
}  // namespace Ngin
