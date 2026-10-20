#include <d3dcompiler.h>
#include <ngin/rhi_dx12/device.h>

namespace Ngin {
HRESULT RHI::Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, Scope<RHI>& rhi) {
  // TODOO: check if it makes problems or if theres a better way for this shit
  constexpr uint64_t FENCE_VALUE = 0;
  HANDLE fenceEvent = nullptr;

  ComScope<ID3D12Device10> device;
  ComScope<ID3D12CommandQueue> cmdQueue;
  ComScope<ID3D12Fence1> fence;
  ComScope<IDXGISwapChain4> swapChain;
  ComScope<ID3D12CommandAllocator> cmdAlloc;
  ComScope<ID3D12GraphicsCommandList7> cmdList;
  ComScope<ID3D12DescriptorHeap> rtvHeap;
  Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT> rtvHandles;
  ComScope<IDXGIFactory7> factory;
  ComScope<ID3D12RootSignature> rootSignature;
  ComScope<ID3DBlob> signatureBlob;
  ComScope<ID3DBlob> errorBlob;
  ComScope<ID3D12PipelineState> pipelineState;
  List<ComScope<ID3D12Resource>> renderTargets = {};
  ComScope<ID3D12Resource2> uploadBuffer, vertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW vbv = {};

  logDebug("Creating Device");
  HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
  if (FAILED(hr))
    return hr;

  logDebug("Creating Cmd Queue");
  hr = CreateCommandQueue(device.get(), cmdQueue);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Fence");
  hr = CreateFence(device.get(), FENCE_VALUE, fence, fenceEvent);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Cmd Alloc");
  hr = CreateCommandAllocator(device.get(), cmdAlloc);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Cmd List");
  hr = CreateCommandList(device.get(), cmdList, cmdAlloc.get());
  if (FAILED(hr))
    return hr;

  logDebug("Creating Factory");
  hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
  if (FAILED(hr))
    return hr;

  logDebug("Creating Swap chain");
  hr = CreateSwapChain(factory.get(), swapChain, cmdQueue.get(), windowWidth, windowHeight, hwnd,
      true);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Rtv Heap");
  hr = CreateRtvHeap(device, swapChain, rtvHeap, rtvHandles);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Sig");
  hr = CreateSignature(device.get(), rootSignature, signatureBlob, errorBlob);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Upload and Vertex Buffers");
  hr = CreateUploadAndVertexBuffer(device.get(), cmdAlloc.get(), cmdQueue.get(), cmdList.get(),
      fence.get(), uploadBuffer, vertexBuffer, vbv);

  // TODO remove that bitch while refing
  logTrace("Waiting for buffers to finish their work");
  if (fence->GetCompletedValue() < 1) {
    HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event) {
      logError(std::format("Creating the event failed: {}", GetLastError()));
      return HRESULT_FROM_WIN32(GetLastError());
    }
    hr = fence->SetEventOnCompletion(1, event);

    if (FAILED(hr)) {
      CloseHandle(event);
      return hr;
    }

    DWORD result = WaitForSingleObject(event, INFINITE);
    CloseHandle(event);
    if (result != WAIT_OBJECT_0)
      return E_FAIL;
  }

  logDebug("Creating RHI");
  if (rhi.get() != nullptr)
    rhi.reset();
  rhi.reset(new RHI(device, cmdQueue, fence, fenceEvent, swapChain, cmdAlloc, cmdList, rtvHeap,
      rtvHandles, factory, rootSignature, pipelineState, renderTargets, uploadBuffer,
      vertexBuffer, vbv));

  return hr;
}

Error RHI::Update() {
  static Error err;
  err = ExecuteCommandList();
  if (err.code != None)
    return err;

  err = Present();
  if (err)
    return err;
  return err;
}

Error RHI::SignalAndWait() {
  cmdQueue->Signal(fence.get(), ++fenceValue);
  HRESULT hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
  if (SUCCEEDED(hr)) {
    DWORD res = WaitForSingleObject(fenceEvent, 21000);
    if (res != WAIT_OBJECT_0) {
      logFatal(std::format("Fence Event has timed out :( {}", res));
      return Error{FenceTimeout, std::format("Fence Event has timed out :( {}", res)};
    }
  } else {
    logFatal(std::format("Fence Event was unsuccessfull :( {}", hr));
    return Error{FenceError, std::format("Fence Event was unsuccessfull :( {}", hr)};
  }
  return Error{};
}

Error RHI::ExecuteCommandList() {
  // if (SUCCEEDED(CmdList->Close())) {
  ID3D12CommandList* list[] = {cmdList.get()};
  cmdQueue->ExecuteCommandLists(1, list);
  return SignalAndWait();
  // }
  // return Error{Unknown, "Cmd list was not closed, but not shure if its a bad thing"};
}

Error RHI::Present() {
  HRESULT hr = swapChain->Present(1, 0);
  if (FAILED(hr)) {
    return Error{Unknown, std::format("Presentation of the SwapChain failed: {}", hr)};
  }
  return Error{};
}
Error RHI::BeginFrame() {
  currentBuffer = swapChain->GetCurrentBackBufferIndex();

  HRESULT hr = cmdAlloc->Reset();
  if (FAILED(hr))
    return Error{Unknown, std::format("Command allocator reset failed: {}", hr)};

  hr = cmdList->Reset(cmdAlloc.get(), pipelineState.get());
  if (FAILED(hr))
    return Error{Unknown, std::format("Command list reset failed: {}", hr)};

  D3D12_RESOURCE_BARRIER barr = {};
  barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barr.Transition.pResource = buffers[currentBuffer];
  barr.Transition.Subresource = 0;
  barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

  cmdList->ResourceBarrier(1, &barr);
  cmdList->ClearRenderTargetView(rtvHandles[currentBuffer], (f32[4]){.4f, .4f, .8f, 1.f}, 0,
      nullptr);
  cmdList->OMSetRenderTargets(1, &rtvHandles[currentBuffer], false, nullptr);

  return Error{};
}
Error RHI::EndFrame() {
  D3D12_RESOURCE_BARRIER barr = {};
  barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barr.Transition.pResource = buffers[currentBuffer];
  barr.Transition.Subresource = 0;
  barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

  cmdList->ResourceBarrier(1, &barr);

  HRESULT hr = cmdList->Close();
  if (FAILED(hr))
    return Error{Unknown, std::format("Command list close failed: {}", hr)};

  return Error{};
}

Error RHI::Resize(uint16_t width, uint16_t height) {
  Flush(FRAME_COUNT);
  HRESULT hr =
      swapChain->ResizeBuffers(FRAME_COUNT, width, height, DXGI_FORMAT_UNKNOWN, SWAPCHAIN_FLAGS);
  if (FAILED(hr)) {
    return Error{Unknown, std::format("SwapChain could not be resized: {}", hr)};
  }
  return Error{};
}
}  // namespace Ngin

namespace Ngin {
HRESULT RHI::CreateCommandQueue(ID3D12Device10* device, ComScope<ID3D12CommandQueue>& cmdQueue) {
  D3D12_COMMAND_QUEUE_DESC cmdQueDesc = {};
  cmdQueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  cmdQueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
  cmdQueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cmdQueDesc.NodeMask = 0;
  return device->CreateCommandQueue(&cmdQueDesc, IID_PPV_ARGS(&cmdQueue));
}

HRESULT RHI::CreateFence(ID3D12Device10* device, const uint64_t fenceValue,
    ComScope<ID3D12Fence1>& fence, HANDLE& fenceEvent) {
  HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  if (FAILED(hr))
    return hr;

  fenceEvent = CreateEvent(nullptr, false, false, nullptr);
  return hr;
}

HRESULT RHI::CreateCommandAllocator(ID3D12Device10* device,
    ComScope<ID3D12CommandAllocator>& cmdAlloc) {
  return device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
}

HRESULT RHI::CreateCommandList(ID3D12Device10* device,
    ComScope<ID3D12GraphicsCommandList7>& cmdList, ID3D12CommandAllocator* cmdAlloc) {
  HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr,
      IID_PPV_ARGS(&cmdList));
  if (FAILED(hr))
    return hr;
  return cmdList->Close();
}

HRESULT RHI::CreateUploadAndVertexBuffer(ID3D12Device10* device, ID3D12CommandAllocator* cmdAlloc,
    ID3D12CommandQueue* cmdQueue, ID3D12GraphicsCommandList7* cmdList, ID3D12Fence1* fence,
    ComScope<ID3D12Resource2>& uploadBuffer, ComScope<ID3D12Resource2>& vertexBuffer,
    D3D12_VERTEX_BUFFER_VIEW& vbv) {
  D3D12_HEAP_PROPERTIES hpDefault{
      .Type = D3D12_HEAP_TYPE_DEFAULT,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 0,
      .VisibleNodeMask = 0
  };
  D3D12_HEAP_PROPERTIES hpUpload{
      .Type = D3D12_HEAP_TYPE_UPLOAD,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 0,
      .VisibleNodeMask = 0
  };

  D3D12_RESOURCE_DESC rd{
      .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
      .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
      .Width = 1024,
      .Height = 1,
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_UNKNOWN,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
      .Flags = D3D12_RESOURCE_FLAG_NONE
  };

  D3D12_INPUT_ELEMENT_DESC vertexLayout[] = {
      {"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          0},
  };

  Shader vertexShader("vertexShader.dxil");
  Shader pixelShader("pixelShader.dxil");

  D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
  gfxPsod.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  gfxPsod.InputLayout.NumElements = _countof(vertexLayout);
  gfxPsod.InputLayout.pInputElementDescs = vertexLayout;
  gfxPsod.VS.pShaderBytecode = vertexShader.GetBuffer();
  gfxPsod.VS.BytecodeLength = vertexShader.GetSize();
  gfxPsod.PS.pShaderBytecode = pixelShader.GetBuffer();
  gfxPsod.PS.BytecodeLength = pixelShader.GetSize();

  HRESULT hr = device->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rd,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
  if (FAILED(hr))
    return hr;
  hr = device->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rd,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer));
  if (FAILED(hr))
    return hr;

  void* uploadBufferAddr;
  D3D12_RANGE readRange{0, 0};
  hr = uploadBuffer->Map(0, &readRange, &uploadBufferAddr);
  if (FAILED(hr)) {
    logError("Mapping the upload buffer failed");
    return hr;
  }

  D3D12_RANGE writtenRange{0, sizeof(verticies)};
  mempcpy(uploadBufferAddr, verticies, sizeof(verticies));
  uploadBuffer->Unmap(0, &writtenRange);

  cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, sizeof(verticies));

  logDebug(std::format("Vertex Buffer: {}\nUpload Buffer: {}",
      static_cast<void*>(vertexBuffer.get()), static_cast<void*>(uploadBuffer.get())));

  ID3D12CommandList* list[] = {cmdList};
  cmdQueue->ExecuteCommandLists(1, list);

  hr = cmdQueue->Signal(fence, 1);

  vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
  vbv.SizeInBytes = sizeof(Vertex) * _countof(verticies);
  vbv.StrideInBytes = sizeof(Vertex);

  return hr;
}

HRESULT RHI::CreateSwapChain(IDXGIFactory7* factory, ComScope<IDXGISwapChain4>& swapChain,
    ID3D12CommandQueue* cmdQueue, uint16_t width, uint16_t height, HWND hwnd, bool windowed) {
  DXGI_SWAP_CHAIN_DESC1 desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.Stereo = false;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.BufferCount = FRAME_COUNT;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  desc.Scaling = DXGI_SCALING_STRETCH;
  desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
  desc.Flags = SWAPCHAIN_FLAGS;

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC descFull = {};
  descFull.Windowed = true;

  IDXGISwapChain1* tempSwapChain = nullptr;
  HRESULT hr =
      factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, &descFull, nullptr, &tempSwapChain);
  if (FAILED(hr))
    return hr;

  hr = tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
  tempSwapChain->Release();

  return hr;
}

HRESULT RHI::GetBuffers(ID3D12Device10* device, ComScope<IDXGISwapChain4>& swapChain,
    ComScope<ID3D12Resource2> buffers[], ID3D12DescriptorHeap* rtvHeap,
    Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT>& rtvHandles) {
  D3D12_RENDER_TARGET_VIEW_DESC desc{};
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  desc.Texture2D.MipSlice = 0;
  desc.Texture2D.PlaneSlice = 0;

  for (size_t i = 0; i < FRAME_COUNT; i++) {
    HRESULT hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&buffers[i]));
    if (FAILED(hr))
      return hr;
    device->CreateRenderTargetView(buffers[i].get(), &desc, rtvHandles[i]);
  }
  return 0;
}

HRESULT RHI::CreateRtvHeap(ID3D12Device10* device, IDXGISwapChain4* swapChain,
    ComScope<ID3D12DescriptorHeap>& rtvHeap,
    Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT>& rtvHandles) {
  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  desc.NumDescriptors = 2;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  desc.NodeMask = 0;

  HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap));
  if (FAILED(hr))
    return hr;

  auto firstHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
  auto handleIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  for (size_t i = 0; i < FRAME_COUNT; i++) {
    rtvHandles[i] = firstHandle;
    rtvHandles[i].ptr += handleIncrement * i;
  }

  return 0;
}

HRESULT RHI::CreateSignature(ID3D12Device10* device, ComScope<ID3D12RootSignature>& rootSignature,
    ComScope<ID3DBlob>& signatureBlob, ComScope<ID3DBlob>& errorBlob) {
  D3D12_ROOT_PARAMETER rootParameters[1] = {};
  rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
  rootParameters[0].Constants.Num32BitValues = 1;
  rootParameters[0].Constants.ShaderRegister = 0;
  rootParameters[0].Constants.RegisterSpace = 0;
  rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

  D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
  rootSignatureDesc.NumParameters = _countof(rootParameters);
  rootSignatureDesc.pParameters = rootParameters;
  rootSignatureDesc.NumStaticSamplers = 0;
  rootSignatureDesc.pStaticSamplers = nullptr;
  rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
      &signatureBlob, &errorBlob);
  if (FAILED(hr))
    return hr;

  hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
      signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
  if (FAILED(hr))
    return hr;
  return hr;
}
};  // namespace Ngin
