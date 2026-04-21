#include "Renderer/Renderer.h"

#include "Modules/UI/TexturedQuad.h"
#include "Modules/StaticMesh/StaticMesh.h"
#include "Modules/TextureDictionary/TextureDictionary.h"
#include "Modules/Camera/CameraSystem.h"

#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>

namespace GV
{
    static RenderCache g_renderCache;

    static void SetupCamera()
    {
        const Camera& cam = CameraSystem::GetActiveCamera();

        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective(
            47.0f,
            480.0f / 272.0f,
            cam.nearClip,
            cam.farClip
        );

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGumRotateZ(-cam.rotZ);
        sceGumRotateY(-cam.rotY);
        sceGumRotateX(-cam.rotX);

        ScePspFVector3 negCam =
        {
            -cam.posX,
            -cam.posY,
            -cam.posZ
        };

        sceGumTranslate(&negCam);
        sceGumUpdateMatrix();

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        sceGumUpdateMatrix();
    }

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

        sceGuClearDepth(65535);
        sceGuDepthRange(0, 65535);
        sceGuDepthFunc(GU_LEQUAL);
        sceGuDepthMask(GU_FALSE);

        sceGuEnable(GU_DEPTH_TEST);
        sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

        SetupCamera();
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


        sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGB);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
        sceGuTexScale(1.0f, 1.0f);
        sceGuTexOffset(0.0f, 0.0f);
        sceGuColor(0xFFFFFFFF);

        uint32_t currentTex = 0xFFFFFFFF;

        for (uint32_t i = 0; i < count; i++)
        {
            const TexturedQuadData& q = TexturedQuad::Get(i);

            if (!q.visible)
                continue;

            RenderCache::Entry* entry =
                g_renderCache.Get(RCACHE_TEXTURED_QUAD, i);

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

            sceKernelDcacheWritebackRange(data->verts, sizeof(data->verts));

            
            sceGuDrawArray(
                GU_TRIANGLES,
                GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
                6,
                0,
                data->verts);
        }
        
    }

    // --------------------------------------------------
    // STATIC MESH RENDERING
    // --------------------------------------------------
    void Renderer::DrawStaticMeshes()
    {
        const uint32_t meshCount = StaticMesh::GetCount();
        if (meshCount == 0)
            return;

        sceGuEnable(GU_DEPTH_TEST);
        sceGuEnable(GU_CULL_FACE);
        sceGuFrontFace(GU_CCW);
        sceGuDisable(GU_LIGHTING);
        

        sceGuDepthRange(0, 65535);
        sceGuDepthFunc(GU_LEQUAL);
        sceGuDepthMask(GU_FALSE);
        
        sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
        sceGuTexScale(1.0f, 1.0f);
        sceGuTexOffset(0.0f, 0.0f);
        
        sceGuColor(0xFFFFFFFF);

        uint32_t currentTex = 0xFFFFFFFF;

        for (uint32_t i = 0; i < meshCount; i++)
        {
            const StaticMeshInstance& mesh = StaticMesh::Get(i);

            if (!mesh.visible)
                continue;

            const uint32_t submeshCount = (uint32_t)mesh.submeshes.size();

            for (uint32_t s = 0; s < submeshCount; s++)
            {
                const uint32_t cacheIndex = (i << 16) | s;

                RenderCache::Entry* entry =
                    g_renderCache.Get(RCACHE_STATIC_MESH, cacheIndex);

                if (!entry || entry->dirty)
                {
                    const uint32_t vertexCount = mesh.submeshes[s].vertexCount;

                    void* ptr = g_renderCache.Allocate(
                        RCACHE_STATIC_MESH,
                        cacheIndex,
                        sizeof(MeshGpuData) + vertexCount * sizeof(PSPVertex),
                        64);

                    if (!ptr)
                        continue;

                    StaticMesh::BuildRenderData(i, s, ptr);
                    entry = g_renderCache.Get(RCACHE_STATIC_MESH, cacheIndex);
                }

                if (!entry || !entry->ptr)
                    continue;

                MeshGpuData* data = (MeshGpuData*)entry->ptr;

                if (data->textureID != currentTex)
                {
                    TextureDictionary::Bind(data->textureID);
                    currentTex = data->textureID;
                }

                sceGumMatrixMode(GU_MODEL);
                sceGumLoadIdentity();

                ScePspFVector3 pos =
                {
                    mesh.posX,
                    mesh.posY,
                    mesh.posZ
                };

                ScePspFVector3 scale =
                {
                    mesh.scaleX,
                    mesh.scaleY,
                    mesh.scaleZ
                };

                sceGumTranslate(&pos);
                sceGumRotateZ(mesh.rotZ);
                sceGumRotateY(mesh.rotY);
                sceGumRotateX(mesh.rotX);
                sceGumScale(&scale);
                sceGumUpdateMatrix();

                sceKernelDcacheWritebackRange(
                    data->verts,
                    data->vertexCount * sizeof(PSPVertex));

                sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
                sceGumDrawArray(
                    GU_TRIANGLES,
                    GU_TEXTURE_16BIT | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
                    data->vertexCount,
                    0,
                    data->verts);
            }
        }

    }
}