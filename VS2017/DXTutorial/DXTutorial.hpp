#pragma once
#include <stdafx.h>
#include <Resource.h>

#include "Input.h"
#include "Graphics.h"
#include <MessageHandler.hpp>

namespace PW
{
    namespace Core
    {
        const UINT MAX_LOADSTRING = 100;

        class System
        {
        public:
            static HINSTANCE GetHInstance()
            {
                assert(g_hInst != nullptr);
                return g_hInst;
            }
            static System* GetInstance(HINSTANCE hInst = nullptr)
            {
                assert((g_hInst != nullptr) || (hInst != nullptr));
                if (g_hInst == nullptr)
                {
                    assert(g_instance == nullptr);
                    g_hInst = hInst;
                }
                if (g_instance == nullptr)
                {
                    HRESULT hr = S_OK;
                    g_instance = new System;
                    hr = g_instance->Awake();
                    if (FAILED(hr))
                    {
                        g_instance->Destroy();
                        delete g_instance;
                        g_instance = nullptr;
                        g_hInst = nullptr;
                    }
                }
                return g_instance;
            }
            static void ReleaseInstance()
            {
                if (g_instance != nullptr)
                {
                    g_instance->Destroy();
                    delete g_instance;
                    g_instance = nullptr;
                    g_hInst = nullptr;
                }
            }

            void MainLoop()
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
            void SendMessageCommand(const std::string &message, ULONG body)
            {
                m_messageHandler->ExecuteCommand(Message(message, body));
            }
            void SendMessageCommand(const std::string &message)
            {
                m_messageHandler->ExecuteCommand(Message(message, 0));
            }
            LRESULT CALLBACK SysMessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

        private:
            static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
            {
                switch (message)
                {
                case WM_COMMAND:
                {
                    int wmId = LOWORD(wParam);
                    switch (wmId)
                    {
                    case IDM_ABOUT:
                        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                        break;
                    case IDM_EXIT:
                        PostQuitMessage(0);
                        break;
                    default:
                        return g_instance->SysMessageHandler(hWnd, message, wParam, lParam);
                    }
                }
                break;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    break;
                default:
                    return g_instance->SysMessageHandler(hWnd, message, wParam, lParam);
                }
                return 0;
            }
            static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
            System() = default;
            ~System() = default;

            /* No copy, no move */
            System(const System &rhs) = delete;
            System(System &&rhs) = delete;
            System &operator=(const System &rhs) = delete;
            System &operator=(System &&rhs) = delete;

            HRESULT Awake()
            {
                assert(!m_isInited);
                HRESULT hr = S_OK;

                m_messageHandler = MessageHandler::GetInstance();
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
            void Destroy()
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
            HRESULT Update()
            {
                static float rotation = 0.0f;
                HRESULT hr = S_OK;
                if (m_input->IsKeyDown(VK_ESCAPE))
                {
                    return E_FAIL;
                }
                rotation += (float)D3DX_PI * 0.005f;
                if (rotation > (float)D3DX_PI * 2)
                {
                    rotation -= (float)D3DX_PI * 2;
                }
                hr = m_graphics->OnRender(rotation);
                return hr;
            }
            HRESULT InitializeWindow()
            {
                HRESULT hr = S_OK;

                /* Load String */
                LoadStringW(g_hInst, IDS_APP_TITLE, m_appName, MAX_LOADSTRING);
                LoadStringW(g_hInst, IDC_DXTUTORIAL, m_className, MAX_LOADSTRING);

                /* Window Class */
                WNDCLASSEXW wcex{};
                wcex.cbSize = sizeof(WNDCLASSEX);
                wcex.style = CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc = WinProc;
                wcex.cbClsExtra = 0;
                wcex.cbWndExtra = 0;
                wcex.hInstance = g_hInst;
                wcex.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DXTUTORIAL));
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
                    UnregisterClassW(m_className, g_hInst);
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
                    g_hInst,
                    nullptr
                );
                hr = m_hwnd ? S_OK : E_FAIL;
                if (FAILED(hr))
                {
                    UnregisterClassW(m_className, g_hInst);
                    return E_FAIL;
                }
                ShowWindow(m_hwnd, SW_SHOWNORMAL);
                UpdateWindow(m_hwnd);
                return S_OK;
            }
            void ShutdownWindow()
            {
                ShowCursor(true);
                DestroyWindow(m_hwnd);
                m_hwnd = nullptr;
                UnregisterClassW(m_className, g_hInst);
            }

            static HINSTANCE g_hInst;
            static System* g_instance;
            HWND m_hwnd = nullptr;
            WCHAR m_appName[MAX_LOADSTRING];
            WCHAR m_className[MAX_LOADSTRING];

            BOOL m_isInited = false;
            PW::Core::MessageHandler *m_messageHandler = nullptr;
            Input* m_input = nullptr;
            Graphics* m_graphics = nullptr;
        };
    }
}
