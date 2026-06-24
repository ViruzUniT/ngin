#include <d3d12.h>

HRESULT CreateCommandQueue(ID3D12Device* device) {
  ID3D12CommandQueue* commandQueue = nullptr;
  D3D12_COMMAND_QUEUE_DESC cmdQueDesc = {};
  cmdQueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  cmdQueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  return device->CreateCommandQueue(&cmdQueDesc, IID_PPV_ARGS(&commandQueue));
}

namespace Ngin {
namespace Device {
void Create() {
  ID3D12Device* device = nullptr;
  HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device));
  hr = CreateCommandQueue(device);
}
}  // namespace Device
}  // namespace Ngin
