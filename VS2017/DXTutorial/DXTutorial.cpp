#include "stdafx.h"
#include "DXTutorial.h"

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

HINSTANCE System::m_hInst = nullptr;
System *System::m_instance = nullptr;

HINSTANCE System::GetHInstance()
{
    assert(m_hInst != nullptr);
    return m_hInst;
}

/* Call this before every thing!!! */
System *System::CreateInstance(HINSTANCE hInst)
{
    if (m_instance == nullptr)
    {
        m_hInst = hInst;
        m_instance = new System;
    }
    return m_instance;
}

System *System::GetInstance()
{
    assert(m_instance != nullptr);
    return m_instance;
}

void System::ReleaseInstance()
{
    /* Call Shutdown first */
    assert(!(m_instance->m_isInited));
    if (m_instance != nullptr)
    {
        delete m_instance;
        m_instance = nullptr;
        m_hInst = nullptr;
    }
}

HRESULT System::Initialize()
{
    assert(!m_isInited);
    HRESULT hr = S_OK;

    /* Window */
    hr = InitializeWindow();
    if (FAILED(hr))
    {
        ShutdownWindow();
        return E_FAIL;
    }

    /* Input */
    m_input = new Input;
    m_input->Initialize();

    /* Graphics */
    m_graphics = new Graphics;
    hr = m_graphics->Initialize(m_hwnd, 1024, 768);
    if (FAILED(hr))
    {
        assert(m_graphics != nullptr);
        m_graphics->Shutdown();
        delete m_graphics;
        m_graphics = nullptr;
        assert(m_input != nullptr);
        delete m_input;
        m_input = nullptr;
        ShutdownWindow();
        return E_FAIL;
    }
    m_isInited = true;
    return S_OK;
}

void System::Shutdown()
{
    assert(m_isInited);

    /* Graphics */
    if (m_graphics)
    {
        m_graphics->Shutdown();
        delete m_graphics;
        m_graphics = nullptr;
    }

    /* Input */
    if (m_input)
    {
        delete m_input;
        m_input = nullptr;
    }

    /* Window */
    ShutdownWindow();

    m_isInited = false;
}

void System::MainLoop()
{
    assert(m_isInited);
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

    /* Load String */
    LoadStringW(m_hInst, IDS_APP_TITLE, m_appName, MAX_LOADSTRING);
    LoadStringW(m_hInst, IDC_DXTUTORIAL, m_className, MAX_LOADSTRING);

    /* Window Class */
    WNDCLASSEXW wcex{};
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
    hr = (RegisterClassEx(&wcex) == 0) ? E_FAIL : S_OK;
    if (FAILED(hr))
    {
        return E_FAIL;
    }

    RECT paintRect{ 0, 0, 1024, 768 };
    hr = AdjustWindowRect(&paintRect, WS_OVERLAPPEDWINDOW, true) ? S_OK : E_FAIL;
    if (FAILED(hr))
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
        paintRect.right - paintRect.left,
        paintRect.bottom - paintRect.top,
        nullptr,
        nullptr,
        m_hInst,
        nullptr
    );
    hr = m_hwnd ? S_OK : E_FAIL;
    if (FAILED(hr))
    {
        UnregisterClassW(m_className, m_hInst);
        return E_FAIL;
    }
    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    UpdateWindow(m_hwnd);
    return S_OK;
}

void System::ShutdownWindow()
{
    ShowCursor(true);
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
    UnregisterClassW(m_className, m_hInst);
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
    System *sys = System::CreateInstance(hInstance);
    hr = sys->Initialize();
    if (FAILED(hr))
    {
        return -1;
    }
    sys->MainLoop();
    sys->Shutdown();
    System::ReleaseInstance();
    return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInst = System::GetHInstance();
    System *sys = System::GetInstance();
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
