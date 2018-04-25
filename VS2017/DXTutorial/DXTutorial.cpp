// DXTutorial.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DXTutorial.h"

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

System* System::m_instance = nullptr;
HINSTANCE System::m_hInst = nullptr;

HINSTANCE System::GetHInstance()
{
    return m_hInst;
}

/* Once set, never change */
void System::SetHInstance(HINSTANCE hInst)
{
    if (m_hInst == nullptr)
    {
        m_hInst = hInst;
    }
}

System* System::GetInstance()
{
    if (m_instance == nullptr)
    {
        assert(m_hInst != nullptr);
        m_instance = new System;
    }
    return m_instance;
}

void System::ReleaseInstance()
{
    /* Call Shutdown first */
    assert(!(m_instance->inited));
    if (m_instance != nullptr)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

HRESULT System::Initialize()
{
    assert(!inited);
    HRESULT hr = S_OK;
    hr = InitializeWindow();
    FAILRETURN(hr);
    m_input = new Input;
    m_graphics = new Graphics;
    m_input->Initialize();
    hr = m_graphics->Initialize(m_hwnd, 1024, 768);
    FAILRETURN(hr);
    inited = true;
    return hr;
}

HRESULT System::Shutdown()
{
    assert(inited);
    HRESULT hr = S_OK;
    if (m_input)
    {
        delete m_input;
        m_input = nullptr;
    }

    if (m_graphics)
    {
        hr = m_graphics->Shutdown();
        delete m_graphics;
        m_graphics = nullptr;
    }
    FAILRETURN(hr);
    hr = ShutdownWindow();
    FAILRETURN(hr);
    inited = false;
    return hr;
}

void System::MainLoop()
{
    assert(inited);
    HRESULT hr = S_OK;
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        hr = Update();
        if (hr != S_OK)
        {
            break;
        }
    }
}

LRESULT System::HandleMSG(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
        m_input->KeyDown((unsigned int)wParam);
        break;
    }
    case WM_KEYUP:
    {
        m_input->KeyUp((unsigned int)wParam);
        break;
    }
    default:
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
    return 0;
}

HRESULT System::Update()
{
    HRESULT hr = S_OK;
    if (m_input->IsKeyDown(VK_ESCAPE))
    {
        return E_FAIL;
    }
    hr = m_graphics->OnRender();
    return hr;
}

HRESULT System::InitializeWindow()
{
    HRESULT hr = S_OK;
    LoadStringW(m_hInst, IDS_APP_TITLE, m_appName, MAX_LOADSTRING);
    LoadStringW(m_hInst, IDC_DXTUTORIAL, m_className, MAX_LOADSTRING);
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WinProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInst;
    wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_DXTUTORIAL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DXTUTORIAL);
    wcex.lpszClassName = m_className;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    auto registerResult = RegisterClassExW(&wcex);
    if (registerResult == 0)
    {
        return E_FAIL;
    }

    RECT rect{ 0, 0, 1024, 768 };
    auto adjustResult = AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, true);
    if (!adjustResult)
    {
        UnregisterClassW(m_className, m_hInst);
        return E_FAIL;
    }
    m_hwnd = CreateWindowW
    (
        m_className,
        m_appName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        m_hInst,
        nullptr
    );
    hr = m_hwnd ? S_OK : E_FAIL;
    if (FAILED(hr))
    {
        UnregisterClassW(m_className, m_hInst);
        return hr;
    }
    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    UpdateWindow(m_hwnd);
    return S_OK;
}

HRESULT System::ShutdownWindow()
{
    HRESULT hr = S_OK;
    ShowCursor(true);
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
    UnregisterClassW(m_className, m_hInst);
    return hr;
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = S_OK;
    System::SetHInstance(hInstance);
    System *sys = System::GetInstance();
    hr = sys->Initialize();
    FAILRETURN(hr);
    sys->MainLoop();
    hr = sys->Shutdown();
    FAILRETURN(hr);
    System::ReleaseInstance();
    return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInst = System::GetHInstance();
    System* sys = System::GetInstance();
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            PostQuitMessage(0);
            break;
        default:
            return sys->HandleMSG(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return sys->HandleMSG(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
