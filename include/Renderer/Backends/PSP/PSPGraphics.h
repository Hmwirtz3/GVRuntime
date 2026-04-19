#pragma once

#include <cstdint>

namespace GV
{
    enum class PrimitiveType
    {
        Triangles,
        TriangleStrip,
        Lines,
        LineStrip
    };

    enum class TextureFormat
    {
        Unknown,
        RGBA8888,
        RGB565,
        Indexed4,
        Indexed8
    };

    enum class FilterMode
    {
        Nearest,
        Linear
    };

    enum class WrapMode
    {
        Clamp,
        Repeat
    };

    enum class BlendMode
    {
        None,
        Alpha
    };

    struct Color32
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;
    };

    struct Viewport
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
    };

    struct TextureDesc
    {
        const void* pixels = nullptr;
        int width = 0;
        int height = 0;
        int stride = 0;
        TextureFormat format = TextureFormat::Unknown;
        FilterMode minFilter = FilterMode::Nearest;
        FilterMode magFilter = FilterMode::Nearest;
        WrapMode wrapU = WrapMode::Clamp;
        WrapMode wrapV = WrapMode::Clamp;
    };

    struct VertexStream
    {
        const void* vertices = nullptr;
        int vertexCount = 0;
        int vertexStride = 0;
    };

    struct GraphicsInitDesc
    {
        int frameWidth = 480;
        int frameHeight = 272;
        bool enableDepth = true;
    };

    class GraphicsBackend
    {
    public:
        GraphicsBackend();
        ~GraphicsBackend();

        bool Initialize(const GraphicsInitDesc& desc);
        void Shutdown();

        void BeginFrame(const Color32& clearColor);
        void EndFrame();

        void SetViewport(const Viewport& viewport);

        void SetDepthTestEnabled(bool enabled);
        void SetDepthWriteEnabled(bool enabled);
        void SetBlendMode(BlendMode mode);

        void BindTexture(const TextureDesc& texture);
        void UnbindTexture();

        void Draw(PrimitiveType primitiveType, const VertexStream& stream);

    private:
        struct Impl;
        Impl* m_impl;
    };
}