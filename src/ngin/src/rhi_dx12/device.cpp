#include <d3dcompiler.h>
#include <ngin/rhi_dx12/device.h>

void setBlendState(D3D12_BLEND_DESC& blendDesc);
void setRasterizerState(D3D12_RASTERIZER_DESC& rasterizerDesc);
void setDepthStencilState(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
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
  ComScope<IDXGIFactory7> factory;
  ComScope<ID3D12RootSignature> rootSignature;
  ComScope<ID3DBlob> signatureBlob;
  ComScope<ID3DBlob> errorBlob;
  ComScope<ID3D12PipelineState> pipelineState;
  List<ComScope<ID3D12Resource>> renderTargets = {};

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
  hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
  if (FAILED(hr))
    return hr;

  logDebug("Creating Swap chain");
  hr = CreateSwapChain(factory.get(), swapChain, cmdQueue.get(), windowWidth, windowHeight, hwnd,
      true);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Rtv Heap");
  hr = CreateRtvHeap(device.get(), swapChain.get(), rtvHeap, renderTargets);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Sig");
  hr = CreateSignature(device.get(), rootSignature, signatureBlob, errorBlob);
  if (FAILED(hr))
    return hr;

  logDebug("Creating Pipeline");
  hr = CreatePipeline(device.get(), rootSignature.get(), pipelineState);
  if (FAILED(hr))
    return hr;

  logDebug("Creating RHI");
  if (rhi.get() != nullptr)
    rhi.reset();
  rhi = std::make_unique<RHI>(device, cmdQueue, fence, fenceEvent, swapChain, cmdAlloc, cmdList,
      rtvHeap, factory, rootSignature, pipelineState, renderTargets);

  return hr;
}

Error RHI::Update() { return ExecuteCommandList(); }

Error RHI::SignalAndWait() {
  CmdQueue->Signal(Fence.get(), ++FenceValue);
  HRESULT hr = Fence->SetEventOnCompletion(FenceValue, FenceEvent);
  if (SUCCEEDED(hr)) {
    DWORD res = WaitForSingleObject(FenceEvent, 21000);
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
  ID3D12CommandList* list[] = {CmdList.get()};
  CmdQueue->ExecuteCommandLists(1, list);
  return SignalAndWait();
  // }
  // return Error{Unknown, "Cmd list was not closed, but not shure if its a bad thing"};
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

HRESULT RHI::CreateSwapChain(IDXGIFactory7* factory, ComScope<IDXGISwapChain4>& swapChain,
    ID3D12CommandQueue* cmdQueue, uint16_t width, uint16_t height, HWND hwnd, bool windowed) {
  DXGI_SWAP_CHAIN_DESC1 desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.BufferCount = 2;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  desc.Scaling = DXGI_SCALING_STRETCH;
  desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

  IDXGISwapChain1* tempSwapChain = nullptr;
  HRESULT hr =
      factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, nullptr, nullptr, &tempSwapChain);
  if (FAILED(hr))
    return hr;

  hr = tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
  tempSwapChain->Release();

  return hr;
}

HRESULT RHI::CreateRtvHeap(ID3D12Device10* device, IDXGISwapChain4* swapChain,
    ComScope<ID3D12DescriptorHeap>& rtvHeap, List<ComScope<ID3D12Resource>>& renderTargets) {
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = 2;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

  HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
  if (FAILED(hr))
    return hr;

  renderTargets.resize(2);
  UINT rtvIncrementSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

  for (UINT i = 0; i < 2; i++) {
    hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
    if (FAILED(hr))
      return hr;
    device->CreateRenderTargetView(renderTargets[i].get(), nullptr, rtvHandle);
    rtvHandle.ptr += rtvIncrementSize;
  }
  return hr;
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

HRESULT RHI::CreatePipeline(ID3D12Device10* device, ID3D12RootSignature* rootSignature,
    ComScope<ID3D12PipelineState>& pipelineState) {
  ComScope<ID3DBlob> vertexShader;
  ComScope<ID3DBlob> pixelShader;

  auto shaderPath = Ngin::getExecutableDirectory();
  HRESULT hr = D3DCompileFromFile((shaderPath / L"vertex.hlsl").c_str(), nullptr, nullptr, "main",
      "vs_5_0", 0, 0, &vertexShader, nullptr);
  if (FAILED(hr))
    return hr;
  hr = D3DCompileFromFile((shaderPath / L"pixel.hlsl").c_str(), nullptr, nullptr, "main",
      "ps_5_0", 0, 0, &pixelShader, nullptr);
  if (FAILED(hr))
    return hr;

  // Pipeline state
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.pRootSignature = rootSignature;
  psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
  psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
  psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
  psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
  setBlendState(psoDesc.BlendState);
  psoDesc.SampleMask = UINT_MAX;
  setRasterizerState(psoDesc.RasterizerState);
  setDepthStencilState(psoDesc.DepthStencilState);
  psoDesc.InputLayout.pInputElementDescs = nullptr;
  psoDesc.InputLayout.NumElements = 0;
  psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  psoDesc.SampleDesc.Count = 1;
  psoDesc.SampleDesc.Quality = 0;

  hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

  return hr;
}
};  // namespace Ngin

void setBlendState(D3D12_BLEND_DESC& blendDesc) {
  blendDesc = {};

  blendDesc.AlphaToCoverageEnable = FALSE;
  blendDesc.IndependentBlendEnable = FALSE;

  D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {};
  defaultRenderTargetBlendDesc.BlendEnable = FALSE;
  defaultRenderTargetBlendDesc.LogicOpEnable = FALSE;
  defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
  defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
  defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
  defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
  defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
  defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
  defaultRenderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
  defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
    blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
  }
}

void setRasterizerState(D3D12_RASTERIZER_DESC& rasterizerDesc) {
  rasterizerDesc = {};

  rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
  rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
  rasterizerDesc.FrontCounterClockwise = FALSE;
  rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
  rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
  rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  rasterizerDesc.DepthClipEnable = TRUE;
  rasterizerDesc.MultisampleEnable = FALSE;
  rasterizerDesc.AntialiasedLineEnable = FALSE;
  rasterizerDesc.ForcedSampleCount = 0;
  rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}
void setDepthStencilState(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc) {
  depthStencilDesc = {};

  depthStencilDesc.DepthEnable = FALSE;
  depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

  depthStencilDesc.StencilEnable = FALSE;
  depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
  depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

  depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

  depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
}
