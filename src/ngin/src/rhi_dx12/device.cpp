#include <d3dcompiler.h>
#include <ngin/rhi_dx12/device.h>

namespace Ngin {
HRESULT RHI::Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, Scope<RHI>& rhi) {
  ID3D12Device* tempDevice = nullptr;
  ID3D12CommandQueue* tempCmdQueue = nullptr;
  IDXGISwapChain* tempSwapChain = nullptr;
  ID3D12CommandAllocator* tempCmdAlloc = nullptr;
  ID3D12GraphicsCommandList* tempCmdList = nullptr;
  ID3D12DescriptorHeap* tempRtvHeap = nullptr;
  IDXGIFactory* tempFactory = nullptr;
  ID3D12PipelineState* tempPipeline = nullptr;

  ComScope<ID3D12Device> device = nullptr;
  ComScope<ID3D12CommandQueue> cmdQueue = nullptr;
  ComScope<IDXGISwapChain> swapChain = nullptr;
  ComScope<ID3D12CommandAllocator> cmdAlloc = nullptr;
  ComScope<ID3D12GraphicsCommandList> cmdList = nullptr;
  ComScope<ID3D12DescriptorHeap> rtvHeap = nullptr;
  ComScope<IDXGIFactory> factory = nullptr;
  ComScope<ID3D12RootSignature> rootSignature = nullptr;
  ComScope<ID3DBlob> signatureBlob = nullptr;
  ComScope<ID3DBlob> errorBlob = nullptr;
  ComScope<ID3D12PipelineState> pipelineState = nullptr;
  List<ComScope<ID3D12Resource>> renderTargets = {};

  HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&tempDevice));
  if (FAILED(hr))
    return hr;
  device.reset(tempDevice);

  hr = CreateCommandQueue(tempDevice, tempCmdQueue);
  if (FAILED(hr))
    return hr;
  cmdQueue.reset(tempCmdQueue);

  hr = CreateCommandAllocator(tempDevice, tempCmdAlloc);
  if (FAILED(hr))
    return hr;
  cmdAlloc.reset(tempCmdAlloc);

  hr = CreateCommandList(tempDevice, tempCmdList, tempCmdAlloc);
  if (FAILED(hr))
    return hr;
  cmdList.reset(tempCmdList);

  hr = CreateDXGIFactory1(IID_PPV_ARGS(&tempFactory));
  if (FAILED(hr))
    return hr;
  factory.reset(tempFactory);

  hr = CreateSwapChain(tempFactory, tempSwapChain, tempCmdQueue, windowWidth, windowHeight, hwnd,
      true);
  if (FAILED(hr))
    return hr;
  swapChain.reset(tempSwapChain);

  hr = CreateRtvHeap(tempDevice, tempSwapChain, tempRtvHeap, renderTargets);
  if (FAILED(hr))
    return hr;
  rtvHeap.reset(tempRtvHeap);

  hr = CreateSignature(tempDevice, rootSignature, signatureBlob, errorBlob);
  if (FAILED(hr))
    return hr;

  auto tempRHI = Scope<RHI>(new RHI(std::move(device), std::move(cmdQueue), std::move(swapChain),
      std::move(cmdAlloc), std::move(cmdList), std::move(rtvHeap), std::move(factory),
      std::move(rootSignature), std::move(renderTargets)));
  rhi = std::move(tempRHI);
  return hr;
}

HRESULT RHI::CreateCommandQueue(ID3D12Device* device, ID3D12CommandQueue*& cmdQueue) {
  D3D12_COMMAND_QUEUE_DESC cmdQueDesc = {};
  cmdQueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  cmdQueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  return device->CreateCommandQueue(&cmdQueDesc, IID_PPV_ARGS(&cmdQueue));
}

HRESULT RHI::CreateCommandAllocator(ID3D12Device* device, ID3D12CommandAllocator*& cmdAlloc) {
  HRESULT hr =
      device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
  if (FAILED(hr))
    return hr;
  return cmdAlloc->Reset();
}

HRESULT RHI::CreateCommandList(ID3D12Device* device, ID3D12GraphicsCommandList*& cmdList,
    ID3D12CommandAllocator* cmdAlloc) {
  HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr,
      IID_PPV_ARGS(&cmdList));
  if (FAILED(hr))
    return hr;
  return cmdList->Close();
}

HRESULT RHI::CreateSwapChain(IDXGIFactory* factory, IDXGISwapChain*& swapChain,
    ID3D12CommandQueue* cmdQueue, uint16_t windowWidth, uint16_t windowHeight, HWND hwnd,
    bool windowed) {
  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferCount = 2;
  swapChainDesc.BufferDesc.Width = windowWidth;
  swapChainDesc.BufferDesc.Height = windowHeight;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapChainDesc.OutputWindow = hwnd;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.Windowed = windowed;

  IDXGISwapChain* tempSwapChain = nullptr;
  HRESULT hr = factory->CreateSwapChain(cmdQueue, &swapChainDesc, &tempSwapChain);
  if (FAILED(hr))
    return hr;

  hr = tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
  tempSwapChain->Release();
  tempSwapChain = nullptr;
  if (FAILED(hr))
    return hr;

  return hr;
}

HRESULT RHI::CreateRtvHeap(ID3D12Device* device, IDXGISwapChain* swapChain,
    ID3D12DescriptorHeap*& rtvHeap, List<ComScope<ID3D12Resource>>& renderTargets) {
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
    ID3D12Resource* tempRenderTarget = nullptr;
    hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&tempRenderTarget));
    if (FAILED(hr))
      return hr;

    renderTargets[i].reset(tempRenderTarget);
    device->CreateRenderTargetView(renderTargets[i].get(), nullptr, rtvHandle);
    rtvHandle.ptr += rtvIncrementSize;
  }
  return hr;
}

HRESULT RHI::CreateSignature(ID3D12Device* device, ComScope<ID3D12RootSignature>& rootSignature,
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

  ID3DBlob* tempSignatureBlob = nullptr;
  ID3DBlob* tempErrorBlob = nullptr;

  HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
      &tempSignatureBlob, &tempErrorBlob);
  signatureBlob.reset(tempSignatureBlob);
  errorBlob.reset(tempErrorBlob);
  if (FAILED(hr))
    return hr;

  ID3D12RootSignature* tempRootSignature = nullptr;
  hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
      signatureBlob->GetBufferSize(), IID_PPV_ARGS(&tempRootSignature));
  if (FAILED(hr))
    return hr;

  rootSignature.reset(tempRootSignature);
  return hr;
}

void set_blend_state(D3D12_BLEND_DESC& blend_desc) {
  blend_desc = {};

  blend_desc.AlphaToCoverageEnable = FALSE;
  blend_desc.IndependentBlendEnable = FALSE;

  D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc = {};
  default_render_target_blend_desc.BlendEnable = FALSE;
  default_render_target_blend_desc.LogicOpEnable = FALSE;
  default_render_target_blend_desc.SrcBlend = D3D12_BLEND_ONE;
  default_render_target_blend_desc.DestBlend = D3D12_BLEND_ZERO;
  default_render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
  default_render_target_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
  default_render_target_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
  default_render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
  default_render_target_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
  default_render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
    blend_desc.RenderTarget[i] = default_render_target_blend_desc;
  }
}

void set_rasterizer_state(D3D12_RASTERIZER_DESC& rasterizer_desc) {
  rasterizer_desc = {};

  rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
  rasterizer_desc.CullMode = D3D12_CULL_MODE_BACK;
  rasterizer_desc.FrontCounterClockwise = FALSE;
  rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
  rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
  rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  rasterizer_desc.DepthClipEnable = TRUE;
  rasterizer_desc.MultisampleEnable = FALSE;
  rasterizer_desc.AntialiasedLineEnable = FALSE;
  rasterizer_desc.ForcedSampleCount = 0;
  rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}
void set_depth_stencil_state(D3D12_DEPTH_STENCIL_DESC& depth_stencil_desc) {
  depth_stencil_desc = {};

  depth_stencil_desc.DepthEnable = FALSE;
  depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

  depth_stencil_desc.StencilEnable = FALSE;
  depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
  depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

  depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

  depth_stencil_desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
}

HRESULT RHI::CreatePipeline(ID3D12Device* device, ID3D12RootSignature* rootSignature,
    ID3D12PipelineState*& pipelineState) {
  ID3DBlob* vertexShader = nullptr;
  ID3DBlob* pixelShader = nullptr;
  HRESULT hr = D3DCompileFromFile(L"vertex.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0,
      &vertexShader, nullptr);
  if (FAILED(hr))
    return hr;
  hr = D3DCompileFromFile(L"pixel.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShader,
      nullptr);
  if (FAILED(hr)) {
    vertexShader->Release();
    return hr;
  }

  // Pipeline state
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.pRootSignature = rootSignature;
  psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
  psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
  psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
  psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
  set_blend_state(psoDesc.BlendState);
  psoDesc.SampleMask = UINT_MAX;
  set_rasterizer_state(psoDesc.RasterizerState);
  set_depth_stencil_state(psoDesc.DepthStencilState);
  psoDesc.InputLayout.pInputElementDescs = nullptr;
  psoDesc.InputLayout.NumElements = 0;
  psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  psoDesc.SampleDesc.Count = 1;
  psoDesc.SampleDesc.Quality = 0;

  hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

  vertexShader->Release();
  pixelShader->Release();

  return hr;
}
}  // namespace Ngin
