#pragma once

#include "stdafx.h"
#include "resource.h"
#include "Input.h"
#include "Graphics.h"

#define MAX_LOADSTRING 100

class System
{
public:
    static HINSTANCE GetHInstance();
    static System* CreateInstance(HINSTANCE hInst);
    static System* GetInstance();
    static void ReleaseInstance();

    HRESULT Initialize();
    void Shutdown();
    void MainLoop();
    LRESULT CALLBACK HandleMSG(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    System() = default;
    System(const System &rhs) = delete;
    System(System &&rhs) = delete;
    ~System() {}
    HRESULT Update();
    HRESULT InitializeWindow();
    void ShutdownWindow();

    static HINSTANCE m_hInst;
    static System* m_instance;
    HWND m_hwnd = nullptr;
    WCHAR m_appName[MAX_LOADSTRING];
    WCHAR m_className[MAX_LOADSTRING];

    bool m_isInited = false;
    Input* m_input = nullptr;
    Graphics* m_graphics = nullptr;
};

static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
