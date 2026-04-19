#include "Renderer/Renderer.h"

#include "Modules/UI/TexturedQuad.h"
#include "Modules/TextureDictionary/TextureDictionary.h"

#include <pspgu.h>
#include <pspgum.h>

namespace GV
{
    static RenderCache g_renderCache;

    // --------------------------------------------------
    // Init
    // --------------------------------------------------
    void Renderer::Init(void* memory, uint32_t size)
    {
        g_renderCache.Init(memory, size);
    }

    // --------------------------------------------------
    // Frame Begin
    // --------------------------------------------------
    void Renderer::BeginFrame()
    {
        g_renderCache.BeginFrame();
    }

    // --------------------------------------------------
    // Frame End
    // --------------------------------------------------
    void Renderer::EndFrame()
    {
    }

    // --------------------------------------------------
    // QUAD RENDERING
    // --------------------------------------------------
    void Renderer::DrawQuads()
{
    const uint32_t count = TexturedQuad::GetCount();
    if (count == 0)
        return;

    sceGuDisable(GU_DEPTH_TEST);
    sceGuDisable(GU_CULL_FACE);
    sceGuDisable(GU_LIGHTING);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuColor(0xFFFFFFFF);

    uint32_t currentTex = 0xFFFFFFFF;

    for (uint32_t i = 0; i < count; i++)
    {
        const TexturedQuadData& q = TexturedQuad::Get(i);

        if (!q.visible)
            continue;

        RenderCache::Entry* entry = g_renderCache.Get(RCACHE_TEXTURED_QUAD, i);

        if (!entry || entry->dirty)
        {
            void* ptr = g_renderCache.Allocate(
                RCACHE_TEXTURED_QUAD,
                i,
                sizeof(QuadGpuData),
                64);

            if (!ptr)
                continue;

            TexturedQuad::BuildRenderData(i, ptr);
            entry = g_renderCache.Get(RCACHE_TEXTURED_QUAD, i);
        }

        if (!entry || !entry->ptr)
            continue;

        QuadGpuData* data = (QuadGpuData*)entry->ptr;

        if (data->textureID != currentTex)
        {
            TextureDictionary::Bind(data->textureID);
            currentTex = data->textureID;
        }

        sceGuDrawArray(
            GU_TRIANGLES,
            GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
            6,
            0,
            data->verts);
    }
}
}