#include <ngin/renderer/imgui.h>
#include <ngin/rhi_dx12/descritporAllocator.h>

namespace Ngin {
DearImGui::DearImGui(WeakRef<RHI> wRhi, const HWND hwnd)
    : context(ImGui::CreateContext()), io(ImGui::GetIO()), wRhi(wRhi) {
  IMGUI_CHECKVERSION();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  auto rhi = wRhi.lock();
  if (!rhi)
    return;

  ImGui_ImplDX12_InitInfo initInfo = {};
  initInfo.Device = rhi->getDevice();
  initInfo.CommandQueue = rhi->getCmdQueue();
  initInfo.NumFramesInFlight = FRAME_COUNT;
  initInfo.RTVFormat = rhi->COLOR_FORMAT;
  initInfo.SrvDescriptorHeap = rhi->getSrvDescriptorHeap();
  initInfo.UserData = rhi->getSrvHeap();
  initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
                                    D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                    D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
    static_cast<ExampleDescriptorHeapAllocator*>(info->UserData)
      ->Alloc(out_cpu_handle, out_gpu_handle);
  };
  initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info,
                                   D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                   D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
    static_cast<ExampleDescriptorHeapAllocator*>(info->UserData)->Free(cpu_handle, gpu_handle);
  };

  if (!ImGui_ImplDX12_Init(&initInfo)) {
    Ngin::logFatal("Imgui init failed");
    std::exit(1);
    return;
  }

  if (!ImGui_ImplWin32_Init(hwnd)) {
    Ngin::logFatal("Imgui init failed");
    std::exit(1);
    return;
  }
}

DearImGui::~DearImGui() {
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

Error DearImGui::Render() {
  auto rhi = wRhi.lock();
  if (!rhi)
    return Error{ErrorCode::InvalidArgument, "Could not render Imgui. WeakRef of rhi is nullptr"};

  ImGui::Render();
  ID3D12DescriptorHeap* descriptorHeaps[] = {rhi->getSrvDescriptorHeap()};
  rhi->getCmdList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), rhi->getCmdList());
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
  return Error{};
}

}  // namespace Ngin
