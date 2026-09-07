#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

#include "Sorting/BubbleSort.h"
#include "Sorting/SelectionSort.h"
#include "Sorting/QuickSort.h"
#include "Sorting/MergeSort.h"

#include "Threading/ThreadManager.h"

#include "Data/DataGenerator.h"

#include "Visualization/Visualizer.h"

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
bool CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Data Visualization
void DrawVisualizer(
    const char* title,
    Visualizer& visualizer,
	bool isRunning,
	bool finished,
	long long elapsedTime,
    ImVec2 position,
    ImVec2 size)
{
    ImGui::SetNextWindowPos(position, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(title, nullptr, flags);

    if (finished)
    {
        ImGui::Text("Status: Finished");
        ImGui::SameLine();
        ImGui::Text(
            "Time: %.2f s",
            elapsedTime / 1000.0
        );
    }
    else if (isRunning)
    {
        ImGui::Text("Status: Running");
    }
    else
    {
        ImGui::Text("Status: Ready");
    }

    std::vector<int> data = visualizer.GetData();

    if (!data.empty())
    {
        ImDrawList* drawList =
            ImGui::GetWindowDrawList();

        ImVec2 canvasPos =
            ImGui::GetCursorScreenPos();

        ImVec2 canvasSize =
            ImGui::GetContentRegionAvail();

        float barWidth =
            canvasSize.x /
            static_cast<float>(data.size());

        float maxValue = 100.0f;

        for (std::size_t i = 0; i < data.size(); ++i)
        {
            float height =
                (static_cast<float>(data[i]) / maxValue)
                * canvasSize.y;

            float x1 =
                canvasPos.x +
                static_cast<float>(i) * barWidth;

            float y1 =
                canvasPos.y +
                canvasSize.y - height;

            float x2 =
                x1 + barWidth - 1.0f;

            float y2 =
                canvasPos.y + canvasSize.y;

            drawList->AddRectFilled(
                ImVec2(x1, y1),
                ImVec2(x2, y2),
                IM_COL32(255, 200, 0, 255)
            );
        }

        ImGui::Dummy(canvasSize);
    }

    ImGui::End();
}

// Main code
int main(int, char**)
{
    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

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

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    DataGenerator generator;
    BubbleSort bubble;
    SelectionSort selection;
    QuickSort quick;
    MergeSort merge;

    Visualizer bubbleVisualizer;
    Visualizer selectionVisualizer;
    Visualizer quickVisualizer;
    Visualizer mergeVisualizer;

    std::thread bubbleThread;
    std::thread selectionThread;
    std::thread quickThread;
    std::thread mergeThread;

    std::atomic<bool> isRunning = false;
    std::atomic<int> finishedCount = 0;
    std::atomic<bool> bubbleFinished = false;
    std::atomic<bool> selectionFinished = false;
    std::atomic<bool> quickFinished = false;
    std::atomic<bool> mergeFinished = false;
    std::atomic<long long> bubbleTime = 0;
    std::atomic<long long> selectionTime = 0;
    std::atomic<long long> quickTime = 0;
    std::atomic<long long> mergeTime = 0;

    std::vector<int> originalData= generator.GenerateRandomData(100, 1, 100);
    std::vector<int> bubbleData = originalData;
    std::vector<int> selectionData = originalData;
    std::vector<int> quickData = originalData;
    std::vector<int> mergeData = originalData;

    bubbleVisualizer.Update(bubbleData);
    selectionVisualizer.Update(selectionData);
    quickVisualizer.Update(quickData);
    mergeVisualizer.Update(mergeData);

    bubble.SetStepCallback(
        [&bubbleVisualizer](const std::vector<int>& data)
        {
            bubbleVisualizer.Update(data);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    );

    selection.SetStepCallback(
        [&selectionVisualizer](const std::vector<int>& data)
        {
            selectionVisualizer.Update(data);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    );

    quick.SetStepCallback(
        [&quickVisualizer](const std::vector<int>& data)
        {
            quickVisualizer.Update(data);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    );

    merge.SetStepCallback(
        [&mergeVisualizer](const std::vector<int>& data)
        {
            mergeVisualizer.Update(data);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    );

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

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            const HRESULT resizeResult = g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            if (FAILED(resizeResult) || !CreateRenderTarget())
            {
                done = true;
                continue;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::IsKeyPressed(ImGuiKey_Q))
        {
            bubble.RequestStop();
            selection.RequestStop();
            quick.RequestStop();
            merge.RequestStop();

            done = true;
        }

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        float screenWidth = viewport->Size.x;
        float screenHeight = viewport->Size.y;

        float topOffset = 50.0f;

        float gap = 5.0f;

        float windowWidth =
            (screenWidth - gap) / 2.0f;

        float windowHeight =
            (screenHeight - topOffset - gap) / 2.0f;

        ImGui::SetNextWindowPos(
            ImVec2(
                viewport->Pos.x + 10.0f,
                viewport->Pos.y + 10.0f
            ),
            ImGuiCond_Always
        );

        ImGuiWindowFlags controlFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground;

        ImGui::Begin("Controls", nullptr, controlFlags);

        if (ImGui::Button("Start"))
        {
            if (!isRunning)
            {
                if (bubbleThread.joinable())
                    bubbleThread.join();

                if (selectionThread.joinable())
                    selectionThread.join();

                if (quickThread.joinable())
                    quickThread.join();

                if (mergeThread.joinable())
                    mergeThread.join();

                bubble.ResetStop();
                selection.ResetStop();
                quick.ResetStop();
                merge.ResetStop();

                bubbleData = originalData;
                selectionData = originalData;
                quickData = originalData;
                mergeData = originalData;

                bubbleVisualizer.Update(bubbleData);
                selectionVisualizer.Update(selectionData);
                quickVisualizer.Update(quickData);
                mergeVisualizer.Update(mergeData);


                isRunning = true;
                bubbleFinished = false;
                selectionFinished = false;
                quickFinished = false;
                mergeFinished = false;

                finishedCount = 0;
                bubbleTime = 0;
                selectionTime = 0;
                quickTime = 0;
                mergeTime = 0;

                bubbleThread = std::thread(
                    [&bubble, &bubbleData, &bubbleFinished, &bubbleTime, &finishedCount, &isRunning]()
                    {
                        auto start =
                            std::chrono::high_resolution_clock::now();

                        bubble.Sort(bubbleData);

                        auto end =
                            std::chrono::high_resolution_clock::now();

                        bubbleTime =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                end - start
                            ).count();

                        bubbleFinished = true;

                        if (++finishedCount == 4)
                        {
                            isRunning = false;
                        }
                    }
                );

                selectionThread = std::thread(
                    [&selection, &selectionData, &selectionTime, &selectionFinished, &finishedCount, &isRunning]()
                    {
                        auto start =
                            std::chrono::high_resolution_clock::now();

                        selection.Sort(selectionData);

                        auto end =
                            std::chrono::high_resolution_clock::now();

                        selectionTime =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                end - start
                            ).count();

                        selectionFinished = true;

                        if (++finishedCount == 4)
                        {
                            isRunning = false;
                        }
                    }
                );

                quickThread = std::thread(
                    [&quick, &quickData, &quickTime, &quickFinished, &finishedCount, &isRunning]()
                    {
                        auto start =
                            std::chrono::high_resolution_clock::now();

                        quick.Sort(quickData);

                        auto end =
                            std::chrono::high_resolution_clock::now();

                        quickTime =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                end - start
                            ).count();

                        quickFinished = true;

                        if (++finishedCount == 4)
                        {
                            isRunning = false;
                        }
                    }
                );

                mergeThread = std::thread(
                    [&merge, &mergeData, &mergeTime, &mergeFinished, &finishedCount, &isRunning]()
                    {
                        auto start =
                            std::chrono::high_resolution_clock::now();

                        merge.Sort(mergeData);

                        auto end =
                            std::chrono::high_resolution_clock::now();

                        mergeTime =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                end - start
                            ).count();

                        mergeFinished = true;

                        if (++finishedCount == 4)
                        {
                            isRunning = false;
                        }
                    }
                );
            }
        }


        if (isRunning)
        {
            ImGui::SameLine();
            ImGui::Text("Sorting...");
        }

        ImGui::SameLine();
        ImGui::Text("Press Q to Exit");

        ImGui::End();

        DrawVisualizer(
            "Bubble Sort",
            bubbleVisualizer,
            isRunning,
            bubbleFinished,
            bubbleTime,
            ImVec2(
                viewport->Pos.x,
                viewport->Pos.y + topOffset
            ),
            ImVec2(
                windowWidth,
                windowHeight
            )
        );

        DrawVisualizer(
            "Selection Sort",
            selectionVisualizer,
            isRunning,
            selectionFinished,
            selectionTime,
            ImVec2(
                viewport->Pos.x + windowWidth + gap,
                viewport->Pos.y + topOffset
            ),
            ImVec2(
                windowWidth,
                windowHeight
            )
        );

        DrawVisualizer(
            "Quick Sort",
            quickVisualizer,
            isRunning,
            quickFinished,
            quickTime,
            ImVec2(
                viewport->Pos.x,
                viewport->Pos.y + topOffset + windowHeight + gap
            ),
            ImVec2(
                windowWidth,
                windowHeight
            )
        );

        DrawVisualizer(
            "Merge Sort",
            mergeVisualizer,
            isRunning,
            mergeFinished,
            mergeTime,
            ImVec2(
                viewport->Pos.x + windowWidth + gap,
                viewport->Pos.y + topOffset + windowHeight + gap
            ),
            ImVec2(
                windowWidth,
                windowHeight
            )
        );

        ImGui::Render();

        const float clear_color_with_alpha[4] =
        {
            clear_color.x * clear_color.w,
            clear_color.y * clear_color.w,
            clear_color.z * clear_color.w,
            clear_color.w
        };

        g_pd3dDeviceContext->OMSetRenderTargets(
            1,
            &g_mainRenderTargetView,
            nullptr
        );

        g_pd3dDeviceContext->ClearRenderTargetView(
            g_mainRenderTargetView,
            clear_color_with_alpha
        );

        ImGui_ImplDX11_RenderDrawData(
            ImGui::GetDrawData()
        );

        g_pSwapChain->Present(1, 0);

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    if (bubbleThread.joinable())
        bubbleThread.join();

    if (selectionThread.joinable())
        selectionThread.join();

    if (quickThread.joinable())
        quickThread.join();

    if (mergeThread.joinable())
        mergeThread.join();

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
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
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

    return CreateRenderTarget();
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

bool CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    const HRESULT getBufferResult = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(getBufferResult) || pBackBuffer == nullptr)
        return false;

    const HRESULT createViewResult = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(createViewResult))
    {
        g_mainRenderTargetView = nullptr;
        return false;
    }

    return true;
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
