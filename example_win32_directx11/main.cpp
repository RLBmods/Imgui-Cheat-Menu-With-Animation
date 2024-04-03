#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "fonts.h"
#include "images.h"
#include <d3d11.h>
#include <D3DX11tex.h>
#include <tchar.h>

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

//Helpers
ImDrawList* drawlist;
ImVec2 pos;
int tabs = 0;
ImFont* icons;
ID3D11ShaderResourceView* logo = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
    if (logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logohxd, sizeof(logohxd), &info, pump, &logo, 0);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    ImFont* mainfont = io.Fonts->AddFontFromMemoryTTF(&mainfonthxd, sizeof mainfonthxd, 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
    icons = io.Fonts->AddFontFromMemoryTTF(&iconshxd, sizeof iconshxd, 16, NULL, io.Fonts->GetGlyphRangesCyrillic());

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            //basic sizing
            ImGui::SetWindowSize(ImVec2(720, 470));

            //helpers def
            drawlist = ImGui::GetWindowDrawList();
            pos = ImGui::GetWindowPos();

            //main rect
            drawlist->AddRectFilled(pos, ImVec2(pos.x + 740, pos.y + 470), ImColor(27,27,27), 4.f);

            //tabs bg
            drawlist->AddRectFilled(pos, ImVec2(pos.x + 160, pos.y + 470), ImColor(18, 18, 18), 4.f);
            drawlist->AddRect(pos, ImVec2(pos.x + 160, pos.y + 470), ImColor(0, 0, 0), 4.f);

            //logo
            ImGui::SetCursorPos(ImVec2(30, 20));
            ImGui::Image(logo, ImVec2(100, 100));

            //tabs
            ImGui::SetCursorPos(ImVec2(50, 175));
            ImGui::BeginGroup();
            {
                if (ImGui::TabButton("Aimbot", "A", ImVec2(65, 65), tabs == 0))
                    tabs = 0;

                if (ImGui::TabButton("Visuals", "B", ImVec2(65, 65), tabs == 1))
                    tabs = 1;

                if (ImGui::TabButton("Misc", "C", ImVec2(65, 65), tabs == 2))
                    tabs = 2;
            }
            ImGui::EndGroup();

            //functions
            if (tabs == 0)
            {
                ImGui::SetCursorPos(ImVec2(180, 10));
                ImGui::BeginChild("Child0", ImVec2(250, 450), true);
                {
                    //functions for the demo
                    static bool checkbox[5];
                    static int combo = 0;
                    static int key = 0;

                    ImGui::SetCursorPos(ImVec2(15, 15));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Enable Aimbot", &checkbox[1]);
                        ImGui::Checkbox("Triggerbot", &checkbox[2]);
                        ImGui::Checkbox("Visible Check", &checkbox[3]);
                        ImGui::Checkbox("Show FOV", &checkbox[4]);
                        ImGui::Combo("Aim Bone", &combo, "Head\0\Neck\0\Chest\0\Pelvis");
                        ImGui::Keybind("Aim Key", &key, true);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(450, 10));
                ImGui::BeginChild("Child1", ImVec2(250, 450), true);
                {
                    //functions for the demo
                    static float sliderfloat0 = 0.f;
                    static float sliderfloat1 = 0.f;

                    ImGui::SetCursorPos(ImVec2(15, 15));
                    ImGui::BeginGroup();
                    {
                        ImGui::SliderFloat("Smoothness", &sliderfloat0, 0.f, 30.f);
                        ImGui::SliderFloat("FOV Value", &sliderfloat1, 0.f, 300.f);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }

            if (tabs == 1)
            {
                ImGui::SetCursorPos(ImVec2(180, 10));
                ImGui::BeginChild("Child0", ImVec2(250, 450), true);
                {
                    //functions for the demo
                    static bool checkbox[8];

                    ImGui::SetCursorPos(ImVec2(15, 15));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Enable Visuals", &checkbox[0]);
                        ImGui::Checkbox("Box ESP", &checkbox[1]);
                        ImGui::Checkbox("Skeleton", &checkbox[2]);
                        ImGui::Checkbox("Distance", &checkbox[3]);
                        ImGui::Checkbox("Snaplines", &checkbox[4]);
                        ImGui::Checkbox("View Angle", &checkbox[5]);
                        ImGui::Checkbox("Name ESP", &checkbox[6]);
                        ImGui::Checkbox("Weapon ESP", &checkbox[7]);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(450, 10));
                ImGui::BeginChild("Child1", ImVec2(250, 450), true);
                {
                    //functions for the demo
                    static float sliderfloat = 0.f;
                    static int combo0 = 0;
                    static int combo1 = 0;
                    static float col0[4] = { 1.00f, 0.00f, 0.00f, 1.00f };
                    static float col1[4] = { 1.00f, 0.00f, 0.00f, 1.00f };
                    static float col2[4] = { 1.00f, 0.00f, 0.00f, 1.00f };
                    static float col3[4] = { 1.00f, 0.00f, 0.00f, 1.00f };

                    ImGui::SetCursorPos(ImVec2(15, 15));
                    ImGui::BeginGroup();
                    {
                        ImGui::SliderFloat("Max Distance", &sliderfloat, 0.f, 500.f);
                        ImGui::Combo("Box Type", &combo0, "Box Type\0\Rounded\0\Cornered");
                        ImGui::Combo("Line Type", &combo1, "Top\0\Middle\0\Bottom");
                        ImGui::ColorEdit4("Visible Color", (float*)&col0, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4("Invisible Color", (float*)&col1, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4("ESP Color", (float*)&col2, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4("FOV Circle Color", (float*)&col3, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }

            if (tabs == 2)
            {
                ImGui::SetCursorPos(ImVec2(180, 10));
                ImGui::BeginChild("Child0", ImVec2(250, 450), true);
                {
                    //functions for the demo
                    static bool checkbox[3];

                    ImGui::SetCursorPos(ImVec2(15, 15));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Outlined ESP", &checkbox[0]);
                        ImGui::Checkbox("Test Exploit", &checkbox[1]);
                        ImGui::Checkbox("Crosshair", &checkbox[2]);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(450, 10));
                ImGui::BeginChild("Child1", ImVec2(250, 450), true);
                {
                    //functions for the demo
                    static int sliderint0 = 0;
                    static int sliderint1 = 0;
                    static int sliderint2 = 0;
                    static int sliderint3 = 0;

                    ImGui::SetCursorPos(ImVec2(15, 15));
                    ImGui::BeginGroup();
                    {
                        ImGui::SliderInt("Skeleton Thickness", &sliderint0, 0, 5);
                        ImGui::SliderInt("Box Thickness", &sliderint1, 0, 5);
                        ImGui::SliderInt("Line Thickness", &sliderint2, 0, 5);
                        ImGui::SliderInt("Head Circle Size", &sliderint3, 0, 50);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }

            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
