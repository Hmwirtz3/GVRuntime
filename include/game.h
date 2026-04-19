#pragma once

#include "Renderer/Backends/PSP/PSPGraphics.h"

namespace GV
{
    class Game
    {
    public:
        bool Initialize();
        void Run();
        void Shutdown();

    private:
        GraphicsBackend m_graphics;
        bool m_running = false;
    };
}