#include <d3dcompiler.h>
#include <ngin/rhi_dx12/device.h>

void setBlendState(D3D12_BLEND_DESC& blendDesc);
void setRasterizerState(D3D12_RASTERIZER_DESC& rasterizerDesc);
void setDepthStencilState(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
namespace Ngin {
HRESULT RHI::Create(HWND hwnd, uint16_t windowWidth, uint16_t windowHeight, Scope<RHI>& rhi) {
  ID3D12Device* tempDevice = nullptr;
  ID3D12CommandQueue* tempCmdQueue = nullptr;
  IDXGISwapChain4* tempSwapChain = nullptr;
  ID3D12CommandAllocator* tempCmdAlloc = nullptr;
  ID3D12GraphicsCommandList* tempCmdList = nullptr;
  ID3D12DescriptorHeap* tempRtvHeap = nullptr;
  IDXGIFactory7* tempFactory = nullptr;
  ID3D12PipelineState* tempPipelineState = nullptr;

  ComScope<ID3D12Device> device = nullptr;
  ComScope<ID3D12CommandQueue> cmdQueue = nullptr;
  ComScope<IDXGISwapChain4> swapChain = nullptr;
  ComScope<ID3D12CommandAllocator> cmdAlloc = nullptr;
  ComScope<ID3D12GraphicsCommandList> cmdList = nullptr;
  ComScope<ID3D12DescriptorHeap> rtvHeap = nullptr;
  ComScope<IDXGIFactory7> factory = nullptr;
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

  hr = CreateDXGIFactory(IID_PPV_ARGS(&tempFactory));
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

  hr = CreatePipeline(device.get(), rootSignature.get(), tempPipelineState);
  if (FAILED(hr))
    return hr;
  pipelineState.reset(tempPipelineState);

  auto tempRHI = Scope<RHI>(new RHI(std::move(device), std::move(cmdQueue), std::move(swapChain),
      std::move(cmdAlloc), std::move(cmdList), std::move(rtvHeap), std::move(factory),
      std::move(rootSignature), std::move(pipelineState), std::move(renderTargets)));
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
  return device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
}

HRESULT RHI::CreateCommandList(ID3D12Device* device, ID3D12GraphicsCommandList*& cmdList,
    ID3D12CommandAllocator* cmdAlloc) {
  HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr,
      IID_PPV_ARGS(&cmdList));
  if (FAILED(hr))
    return hr;
  return cmdList->Close();
}

HRESULT RHI::CreateSwapChain(IDXGIFactory7* factory, IDXGISwapChain4*& swapChain,
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

  IDXGISwapChain* baseSwapChain = nullptr;
  HRESULT hr = factory->CreateSwapChain(cmdQueue, &swapChainDesc, &baseSwapChain);
  if (FAILED(hr))
    return hr;

  hr = baseSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
  baseSwapChain->Release();
  baseSwapChain = nullptr;
  return hr;

  return hr;
}

HRESULT RHI::CreateRtvHeap(ID3D12Device* device, IDXGISwapChain4* swapChain,
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

HRESULT RHI::CreatePipeline(ID3D12Device* device, ID3D12RootSignature* rootSignature,
    ID3D12PipelineState*& pipelineState) {
  ID3DBlob* tempVertexShader = nullptr;
  ID3DBlob* tempPixelShader = nullptr;
  ComScope<ID3DBlob> vertexShader = nullptr;
  ComScope<ID3DBlob> pixelShader = nullptr;
  HRESULT hr = D3DCompileFromFile(L"vertex.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0,
      &tempVertexShader, nullptr);
  if (FAILED(hr))
    return hr;
  vertexShader.reset(tempVertexShader);
  hr = D3DCompileFromFile(L"pixel.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0,
      &tempPixelShader, nullptr);
  if (FAILED(hr))
    return hr;
  pixelShader.reset(tempPixelShader);

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
}  // namespace Ngin
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
