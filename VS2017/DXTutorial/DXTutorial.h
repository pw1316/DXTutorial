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
    static void SetHInstance(HINSTANCE hInst);
    static System* GetInstance();
    static void ReleaseInstance();

    HRESULT Initialize();
    HRESULT Shutdown();
    void MainLoop();
    LRESULT CALLBACK HandleMSG(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    System() {}
    System(const System& rhs) = delete;
    System(System&& rhs) = delete;
    ~System() {}
    HRESULT Update();
    HRESULT InitializeWindow();
    HRESULT ShutdownWindow();

    static System* m_instance;
    static HINSTANCE m_hInst;
    HWND m_hwnd = nullptr;
    WCHAR m_appName[MAX_LOADSTRING];
    WCHAR m_className[MAX_LOADSTRING];

    Input* m_input = nullptr;
    Graphics* m_graphics = nullptr;
};

static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
