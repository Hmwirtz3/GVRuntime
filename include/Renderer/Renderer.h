#pragma once

#include "Framework/Utils/RenderCache.h"

namespace GV
{
    class Renderer
    {
    public:
        static void Init(void* memory, uint32_t size);
        static void BeginFrame();
        static void EndFrame();

        static void DrawQuads();
        static void DrawStaticMeshes(); 
    };
}