// Dear ImGui: standalone example application for DirectX 12
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important: to compile on 32-bit systems, the DirectX12 backend requires code to be compiled with '#define ImTextureID ImU64'.
// This is because we need ImTextureID to carry a 64-bit value and by default ImTextureID is defined as void*.
// This define is set in the example .vcxproj file and need to be replicated in your app or by adding it to your imconfig.h file.

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <shlobj_core.h>

#include <hwm/task/task_queue.hpp>
#include <minecraft-file.hpp>
#include <nfd.h>
extern "C" {
#include <uuid4.h>
}
#include <variant>

#include "version.hpp"
#include "string.hpp"
#include "texture.hpp"
#include "platform.hpp"
#include "texture-set.hpp"
#include "temporary-directory.hpp"
#include "model/node.hpp"
#include "model/node.impl.hpp"
#include "model/directory-contents.impl.hpp"
#include "model/region.impl.hpp"
#include "model/compound.impl.hpp"
#include "model/state.hpp"
#include "imgui-ext.hpp"
#include "render/legal.hpp"
#include "render/render.hpp"

struct FrameContext {
  ID3D12CommandAllocator *CommandAllocator;
  UINT64 FenceValue;
};

// Data
static int const NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_frameIndex = 0;

static int const NUM_BACK_BUFFERS = 3;
static ID3D12Device *g_pd3dDevice = NULL;
static ID3D12DescriptorHeap *g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap *g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue *g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList *g_pd3dCommandList = NULL;
static ID3D12Fence *g_fence = NULL;
static HANDLE g_fenceEvent = NULL;
static UINT64 g_fenceLastSignaledValue = 0;
static IDXGISwapChain3 *g_pSwapChain = NULL;
static HANDLE g_hSwapChainWaitableObject = NULL;
static ID3D12Resource *g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext *WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  namespace fs = std::filesystem;

  fs::path file;
  if (lstrlenW(pCmdLine) > 0) {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(pCmdLine, &argc);
    if (!argv) {
      return 1;
    }
    if (argc > 0) {
      file = argv[0];
    }
    LocalFree(argv);
  }

  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("nbte"), NULL};
  ::RegisterClassEx(&wc);
  HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("nbte"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  if (!CreateDeviceD3D(hwnd)) {
    CleanupDeviceD3D();
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.IniFilename = nullptr;

  // Setup Dear ImGui style
  ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                      DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                      g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                      g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

  if (auto udevFont = nbte::LoadNamedResource("UDEVGothic35_Regular.ttf"); udevFont) {
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    assert(udevFont->fSystemOwned);

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesThai());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.BuildRanges(&ranges);

    ImFont *font = io.Fonts->AddFontFromMemoryTTF(udevFont->fData, udevFont->fSize, 15.0f, &cfg, ranges.Data);
    io.Fonts->Build();
  }

  uuid4_init();

  nbte::State state;
  state.fMinecraftSaveDirectory = nbte::MinecraftSaveDirectory();

  if (fs::exists(file)) {
    if (fs::is_regular_file(file)) {
      state.open(file);
    } else if (fs::is_directory(file)) {
      state.openDirectory(file);
    }
  }

  state.loadTextures(nullptr);

  // Main loop
  bool done = false;
  while (!done) {
    // Poll and handle messages (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        done = true;
    }
    if (done)
      break;

    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    SetWindowTextA(hwnd, (char const *)state.winowTitle().c_str());

    state.fDisplaySize = io.DisplaySize;
    nbte::Render(state);
    if (state.fMainMenuBarQuitSelected) {
      done = true;
    }

    // Rendering
    ImGui::Render();

    FrameContext *frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    g_pd3dCommandList->Reset(frameCtx->CommandAllocator, NULL);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    ImGuiStyle const &style = ImGui::GetStyle();
    ImVec4 clearColor = style.Colors[ImGuiCol_WindowBg];
    const float clear_color_with_alpha[4] = {clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w};
    g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, NULL);
    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList *const *)&g_pd3dCommandList);

    g_pSwapChain->Present(1, 0); // Present with vsync
    //g_pSwapChain->Present(0, 0); // Present without vsync

    UINT64 fenceValue = g_fenceLastSignaledValue + 1;
    g_pd3dCommandQueue->Signal(g_fence, fenceValue);
    g_fenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;
  }

  WaitForLastSubmittedFrame();

  // Cleanup
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClass(wc.lpszClassName, wc.hInstance);

  return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd) {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC1 sd;
  {
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = NUM_BACK_BUFFERS;
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.Stereo = FALSE;
  }

  // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
  ID3D12Debug *pdx12Debug = NULL;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
    pdx12Debug->EnableDebugLayer();
#endif

  // Create device
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
  if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
    return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
  if (pdx12Debug != NULL) {
    ID3D12InfoQueue *pInfoQueue = NULL;
    g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
    pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    pInfoQueue->Release();
    pdx12Debug->Release();
  }
#endif

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = NUM_BACK_BUFFERS;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 1;
    if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
      return false;

    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
      g_mainRenderTargetDescriptor[i] = rtvHandle;
      rtvHandle.ptr += rtvDescriptorSize;
    }
  }

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
      return false;
  }

  {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 1;
    if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
      return false;
  }

  for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
      return false;

  if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
      g_pd3dCommandList->Close() != S_OK)
    return false;

  if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
    return false;

  g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (g_fenceEvent == NULL)
    return false;

  {
    IDXGIFactory4 *dxgiFactory = NULL;
    IDXGISwapChain1 *swapChain1 = NULL;
    if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
      return false;
    if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1) != S_OK)
      return false;
    if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
      return false;
    swapChain1->Release();
    dxgiFactory->Release();
    g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
    g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
  }

  CreateRenderTarget();
  return true;
}

void CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->SetFullscreenState(false, NULL);
    g_pSwapChain->Release();
    g_pSwapChain = NULL;
  }
  if (g_hSwapChainWaitableObject != NULL) {
    CloseHandle(g_hSwapChainWaitableObject);
  }
  for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    if (g_frameContext[i].CommandAllocator) {
      g_frameContext[i].CommandAllocator->Release();
      g_frameContext[i].CommandAllocator = NULL;
    }
  if (g_pd3dCommandQueue) {
    g_pd3dCommandQueue->Release();
    g_pd3dCommandQueue = NULL;
  }
  if (g_pd3dCommandList) {
    g_pd3dCommandList->Release();
    g_pd3dCommandList = NULL;
  }
  if (g_pd3dRtvDescHeap) {
    g_pd3dRtvDescHeap->Release();
    g_pd3dRtvDescHeap = NULL;
  }
  if (g_pd3dSrvDescHeap) {
    g_pd3dSrvDescHeap->Release();
    g_pd3dSrvDescHeap = NULL;
  }
  if (g_fence) {
    g_fence->Release();
    g_fence = NULL;
  }
  if (g_fenceEvent) {
    CloseHandle(g_fenceEvent);
    g_fenceEvent = NULL;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = NULL;
  }

#ifdef DX12_ENABLE_DEBUG_LAYER
  IDXGIDebug1 *pDebug = NULL;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug)))) {
    pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
    pDebug->Release();
  }
#endif
}

void CreateRenderTarget() {
  for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
    ID3D12Resource *pBackBuffer = NULL;
    g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, g_mainRenderTargetDescriptor[i]);
    g_mainRenderTargetResource[i] = pBackBuffer;
  }
}

void CleanupRenderTarget() {
  WaitForLastSubmittedFrame();

  for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    if (g_mainRenderTargetResource[i]) {
      g_mainRenderTargetResource[i]->Release();
      g_mainRenderTargetResource[i] = NULL;
    }
}

void WaitForLastSubmittedFrame() {
  FrameContext *frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

  UINT64 fenceValue = frameCtx->FenceValue;
  if (fenceValue == 0)
    return; // No fence was signaled

  frameCtx->FenceValue = 0;
  if (g_fence->GetCompletedValue() >= fenceValue)
    return;

  g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
  WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext *WaitForNextFrameResources() {
  UINT nextFrameIndex = g_frameIndex + 1;
  g_frameIndex = nextFrameIndex;

  HANDLE waitableObjects[] = {g_hSwapChainWaitableObject, NULL};
  DWORD numWaitableObjects = 1;

  FrameContext *frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
  UINT64 fenceValue = frameCtx->FenceValue;
  if (fenceValue != 0) // means no fence was signaled
  {
    frameCtx->FenceValue = 0;
    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    waitableObjects[1] = g_fenceEvent;
    numWaitableObjects = 2;
  }

  WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

  return frameCtx;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
  case WM_SIZE:
    if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
      WaitForLastSubmittedFrame();
      CleanupRenderTarget();
      HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
      assert(SUCCEEDED(result) && "Failed to resize swapchain.");
      CreateRenderTarget();
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

#if 0
// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NOMINMAX
#include <windows.h>
#include <shlobj_core.h>

#include <hwm/task/task_queue.hpp>
#include <minecraft-file.hpp>
#include <nfd.h>
extern "C" {
#include <uuid4.h>
}
#include <variant>

#include "version.hpp"
#include "string.hpp"
#include "texture.hpp"
#include "platform.hpp"
#include "texture-set.hpp"
#include "temporary-directory.hpp"
#include "model/node.hpp"
#include "model/node.impl.hpp"
#include "model/directory-contents.impl.hpp"
#include "model/region.impl.hpp"
#include "model/compound.impl.hpp"
#include "model/state.hpp"
#include "imgui-ext.hpp"
#include "render/legal.hpp"
#include "render/render.hpp"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#pragma comment(lib, "opengl32.lib")

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  namespace fs = std::filesystem;

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    return 1;
  }

  fs::path file;
  if (lstrlenW(pCmdLine) > 0) {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(pCmdLine, &argc);
    if (!argv) {
      return 1;
    }
    if (argc > 0) {
      file = argv[0];
    }
    LocalFree(argv);
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(1280, 720, "nbte", NULL, NULL);
  if (window == NULL) {
    return 1;
  }

  if (auto nbte32 = nbte::LoadNamedResource("nbte32.png"); nbte32) {
    int width, height, components;
    unsigned char *img = stbi_load_from_memory((stbi_uc const *)nbte32->fData, nbte32->fSize, &width, &height, &components, 4);
    GLFWimage icon = {width, height, img};
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(img);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.IniFilename = nullptr;

  ImGui::StyleColorsLight();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  if (auto udevFont = nbte::LoadNamedResource("UDEVGothic35_Regular.ttf"); udevFont) {
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    assert(udevFont->fSystemOwned);

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesThai());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.BuildRanges(&ranges);

    ImFont *font = io.Fonts->AddFontFromMemoryTTF(udevFont->fData, udevFont->fSize, 15.0f, &cfg, ranges.Data);
    io.Fonts->Build();
  }

  uuid4_init();

  nbte::State state;
  state.fMinecraftSaveDirectory = nbte::MinecraftSaveDirectory();

  if (fs::exists(file)) {
    if (fs::is_regular_file(file)) {
      state.open(file);
    } else if (fs::is_directory(file)) {
      state.openDirectory(file);
    }
  }

  state.loadTextures(nullptr);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    glfwSetWindowTitle(window, (char const *)state.winowTitle().c_str());

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    state.fDisplaySize = ImVec2(display_w, display_h);
    nbte::Render(state);
    if (state.fMainMenuBarQuitSelected) {
      glfwSetWindowShouldClose(window, 1);
    }

    ImGui::Render();

    glViewport(0, 0, display_w, display_h);

    ImGuiStyle const &style = ImGui::GetStyle();
    ImVec4 clearColor = style.Colors[ImGuiCol_WindowBg];
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
#endif
