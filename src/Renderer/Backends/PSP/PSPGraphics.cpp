#include "Renderer/Backends/PSP/PSPGraphics.h"

#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspge.h>
#include <cstring>

namespace GV
{
    struct GraphicsBackend::Impl
    {
        bool initialized = false;
        int frameWidth = 480;
        int frameHeight = 272;
        bool depthEnabled = true;

        static constexpr std::size_t DisplayListSize = 262144;
        alignas(16) unsigned char displayList[DisplayListSize];

        void* frameBuffer0 = nullptr;
        void* frameBuffer1 = nullptr;
        void* depthBuffer  = nullptr;
        bool useFirstBuffer = true;
    };

    static int ToPSPPrimitive(PrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
            case PrimitiveType::Triangles:     return GU_TRIANGLES;
            case PrimitiveType::TriangleStrip: return GU_TRIANGLE_STRIP;
            case PrimitiveType::Lines:         return GU_LINES;
            case PrimitiveType::LineStrip:     return GU_LINE_STRIP;
            default:                           return GU_TRIANGLES;
        }
    }

    static int ToPSPTextureFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RGBA8888: return GU_PSM_8888;
            case TextureFormat::RGB565:   return GU_PSM_5650;
            case TextureFormat::Indexed4: return GU_PSM_T4;
            case TextureFormat::Indexed8: return GU_PSM_T8;
            default:                      return GU_PSM_8888;
        }
    }

    static int ToPSPFilter(FilterMode mode)
    {
        switch (mode)
        {
            case FilterMode::Nearest: return GU_NEAREST;
            case FilterMode::Linear:  return GU_LINEAR;
            default:                  return GU_NEAREST;
        }
    }

    static int ToPSPWrap(WrapMode mode)
    {
        switch (mode)
        {
            case WrapMode::Clamp:  return GU_CLAMP;
            case WrapMode::Repeat: return GU_REPEAT;
            default:               return GU_CLAMP;
        }
    }

    GraphicsBackend::GraphicsBackend()
        : m_impl(new Impl())
    {
    }

    GraphicsBackend::~GraphicsBackend()
    {
        Shutdown();
        delete m_impl;
        m_impl = nullptr;
    }

    bool GraphicsBackend::Initialize(const GraphicsInitDesc& desc)
    {
        if (m_impl->initialized)
            return true;

        m_impl->frameWidth = desc.frameWidth;
        m_impl->frameHeight = desc.frameHeight;
        m_impl->depthEnabled = desc.enableDepth;

        sceGuInit();

        // NOTE:
        // These addresses are the usual PSP VRAM-relative offsets used by GU.
        // If you already have a VRAM allocator/static buffer helper in your engine,
        // route this through that instead.
        m_impl->frameBuffer0 = reinterpret_cast<void*>(0);
        m_impl->frameBuffer1 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(512 * 272 * 4));
        m_impl->depthBuffer  = reinterpret_cast<void*>(static_cast<std::uintptr_t>(512 * 272 * 4 * 2));

        sceGuStart(GU_DIRECT, m_impl->displayList);

        sceGuDrawBuffer(GU_PSM_8888, m_impl->frameBuffer0, 512);
        sceGuDispBuffer(m_impl->frameWidth, m_impl->frameHeight, m_impl->frameBuffer1, 512);
        sceGuDepthBuffer(m_impl->depthBuffer, 512);

        sceGuOffset(2048 - (m_impl->frameWidth / 2), 2048 - (m_impl->frameHeight / 2));
        sceGuViewport(2048, 2048, m_impl->frameWidth, m_impl->frameHeight);

        sceGuDepthRange(65535, 0);
        sceGuScissor(0, 0, m_impl->frameWidth, m_impl->frameHeight);
        sceGuEnable(GU_SCISSOR_TEST);

        if (m_impl->depthEnabled)
        {
            sceGuEnable(GU_DEPTH_TEST);
            sceGuDepthMask(GU_FALSE);
            sceGuDepthFunc(GU_LEQUAL);
        }
        else
        {
            sceGuDisable(GU_DEPTH_TEST);
            sceGuDepthMask(GU_TRUE);
        }

        sceGuFrontFace(GU_CW);
        sceGuShadeModel(GU_SMOOTH);
        sceGuDisable(GU_BLEND);
        sceGuEnable(GU_CULL_FACE);
        sceGuEnable(GU_TEXTURE_2D);

        sceGuFinish();
        sceGuSync(0, 0);

        sceDisplayWaitVblankStart();
        sceGuDisplay(GU_TRUE);

        m_impl->initialized = true;
        return true;
    }

    void GraphicsBackend::Shutdown()
    {
        if (!m_impl || !m_impl->initialized)
            return;

        sceGuTerm();
        m_impl->initialized = false;
    }

    void GraphicsBackend::BeginFrame(const Color32& clearColor)
    {
    if (!m_impl || !m_impl->initialized)
        return;

    sceGuStart(GU_DIRECT, m_impl->displayList);

    const int width  = m_impl->frameWidth;
    const int height = m_impl->frameHeight;

    sceGuOffset(2048 - (width / 2), 2048 - (height / 2));
    sceGuViewport(2048, 2048, width, height);

    sceGuScissor(0, 0, width, height);
    sceGuEnable(GU_SCISSOR_TEST);

    sceGuClearColor(
        (static_cast<unsigned int>(clearColor.a) << 24) |
        (static_cast<unsigned int>(clearColor.b) << 16) |
        (static_cast<unsigned int>(clearColor.g) << 8)  |
        (static_cast<unsigned int>(clearColor.r))
    );

    int clearMask = GU_COLOR_BUFFER_BIT;
    if (m_impl->depthEnabled)
        clearMask |= GU_DEPTH_BUFFER_BIT;

    sceGuClear(clearMask);
    }

    void GraphicsBackend::EndFrame()
    {
        if (!m_impl || !m_impl->initialized)
            return;

        //sceDisplayWaitVblankStart();
        sceGuFinish();
        sceGuSync(0, 0);
        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    }

    void GraphicsBackend::SetViewport(const Viewport& viewport)
{
    if (!m_impl || !m_impl->initialized)
        return;

    const int width  = viewport.width;
    const int height = viewport.height;

    // Correct PSP viewport setup
    sceGuOffset(2048 - (width / 2), 2048 - (height / 2));
    sceGuViewport(2048, 2048, width, height);

    // Scissor stays in screen space
    sceGuScissor(
        viewport.x,
        viewport.y,
        viewport.x + width,
        viewport.y + height
    );

    sceGuEnable(GU_SCISSOR_TEST);
}

    void GraphicsBackend::SetDepthTestEnabled(bool enabled)
    {
        if (!m_impl || !m_impl->initialized)
            return;

        m_impl->depthEnabled = enabled;

        if (enabled)
            sceGuEnable(GU_DEPTH_TEST);
        else
            sceGuDisable(GU_DEPTH_TEST);
    }

    void GraphicsBackend::SetDepthWriteEnabled(bool enabled)
    {
        if (!m_impl || !m_impl->initialized)
            return;

        sceGuDepthMask(enabled ? GU_FALSE : GU_TRUE);
    }

    void GraphicsBackend::SetBlendMode(BlendMode mode)
    {
        if (!m_impl || !m_impl->initialized)
            return;

        switch (mode)
        {
            case BlendMode::None:
                sceGuDisable(GU_BLEND);
                break;

            case BlendMode::Alpha:
                sceGuEnable(GU_BLEND);
                sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
                break;
        }
    }

    void GraphicsBackend::BindTexture(const TextureDesc& texture)
    {
        if (!m_impl || !m_impl->initialized)
            return;

        if (!texture.pixels || texture.width <= 0 || texture.height <= 0)
            return;

        sceGuEnable(GU_TEXTURE_2D);

        const int pspFormat = ToPSPTextureFormat(texture.format);

        sceGuTexMode(pspFormat, 0, 0, GU_FALSE);
        sceGuTexImage(0, texture.width, texture.height, texture.stride, const_cast<void*>(texture.pixels));
        sceGuTexFilter(ToPSPFilter(texture.minFilter), ToPSPFilter(texture.magFilter));
        sceGuTexWrap(ToPSPWrap(texture.wrapU), ToPSPWrap(texture.wrapV));
        sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    }

    void GraphicsBackend::UnbindTexture()
    {
        if (!m_impl || !m_impl->initialized)
            return;

        sceGuDisable(GU_TEXTURE_2D);
    }

    void GraphicsBackend::Draw(PrimitiveType primitiveType, const VertexStream& stream)
    {
        if (!m_impl || !m_impl->initialized)
            return;

        if (!stream.vertices || stream.vertexCount <= 0)
            return;

        // NOTE:
        // The actual vertex format flags are intentionally not exposed yet.
        // For now this assumes the renderer/backend agree on a fixed vertex format.
        // This should eventually be promoted into a backend-neutral vertex layout description.
        constexpr int VertexFormat =
            GU_TEXTURE_16BIT |
            GU_COLOR_8888 |
            GU_VERTEX_32BITF |
            GU_TRANSFORM_3D;

        sceGuDrawArray(
            ToPSPPrimitive(primitiveType),
            VertexFormat,
            stream.vertexCount,
            nullptr,
            const_cast<void*>(stream.vertices)
        );
    }
}