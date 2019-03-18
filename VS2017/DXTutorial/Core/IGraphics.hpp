#pragma once
#include <stdafx.h>

namespace Naiive::Core
{
    class IGraphics
    {
    public:
        IGraphics() = default;
        virtual ~IGraphics() = default;
        /* No copy, no move */
        IGraphics(const IGraphics& rhs) = delete;
        IGraphics(IGraphics&& rhs) = delete;
        IGraphics &operator=(const IGraphics &rhs) = delete;
        IGraphics &operator=(IGraphics &&rhs) = delete;

        virtual void Initialize(HWND hWnd, UINT width, UINT height) = 0;
        virtual void Shutdown() = 0;
        virtual BOOL OnUpdate() = 0;
        virtual LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    };
}
