#pragma once
#include <stdafx.h>
#include <Resource.h>

#include <Core/MessageHandler.hpp>
#include <Manager/InputManager.hpp>
#include <Manager/GraphicsManager.hpp>

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
                    g_instance = new System;
                    g_instance->Awake();
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
                    SendMessageCommand("SYS_KEY_DOWN", (UINT)wParam);
                    break;
                }
                case WM_KEYUP:
                {
                    SendMessageCommand("SYS_KEY_UP", (UINT)wParam);
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

            void Awake()
            {
                assert(!m_isInited);
                HRESULT hr = S_OK;

                m_messageHandler = MessageHandler::GetInstance();
                /* Window */
                InitializeWindow();

                /* Input */
                m_input = new Manager::InputManager;
                hr = m_input->Awake();
                FAILTHROW;

                /* Graphics */
                m_graphics = new PW::Manager::GraphicsManager;
                hr = m_graphics->Awake();
                FAILTHROW;
                m_graphics->Initialize(m_hwnd, 1024, 768);
                m_isInited = true;
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
                HRESULT hr = S_OK;
                /* Physics */
                // TODO physics
                /* Input */
                if (m_input->IsKeyDown(VK_ESCAPE))
                {
                    return E_FAIL;
                }
                /* Logic */
                /* Graphics */
                m_graphics->OnRender();
                /* GUI */
                // TODO gui
                return hr;
            }
            void InitializeWindow()
            {
                HRESULT hr = S_OK;

                /* Load String */
                LoadString(g_hInst, IDS_APP_TITLE, m_appName, MAX_LOADSTRING);
                LoadString(g_hInst, IDC_DXTUTORIAL, m_className, MAX_LOADSTRING);

                /* Window Class */
                WNDCLASSEX wcex{};
                wcex.cbSize = sizeof(WNDCLASSEX);
                wcex.style = CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc = WinProc;
                wcex.cbClsExtra = 0;
                wcex.cbWndExtra = 0;
                wcex.hInstance = g_hInst;
                wcex.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DXTUTORIAL));
                wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
                wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
                wcex.lpszMenuName = MAKEINTRESOURCE(IDC_DXTUTORIAL);
                wcex.lpszClassName = m_className;
                wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
                hr = (RegisterClassEx(&wcex) == 0) ? E_FAIL : S_OK;
                FAILTHROW;

                RECT paintRect{ 0, 0, 1024, 768 };
                hr = AdjustWindowRect(&paintRect, WS_OVERLAPPEDWINDOW, true) ? S_OK : E_FAIL;
                FAILTHROW;
                m_hwnd = CreateWindow
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
                FAILTHROW;
                ShowWindow(m_hwnd, SW_SHOWNORMAL);
                UpdateWindow(m_hwnd);
            }
            void ShutdownWindow()
            {
                ShowCursor(true);
                DestroyWindow(m_hwnd);
                m_hwnd = nullptr;
                UnregisterClass(m_className, g_hInst);
            }

            static HINSTANCE g_hInst;
            static System* g_instance;
            HWND m_hwnd = nullptr;
            CHAR m_appName[MAX_LOADSTRING];
            CHAR m_className[MAX_LOADSTRING];

            BOOL m_isInited = false;
            PW::Core::MessageHandler *m_messageHandler = nullptr;
            Manager::InputManager *m_input = nullptr;
            PW::Manager::GraphicsManager* m_graphics = nullptr;
        };
    }
}
