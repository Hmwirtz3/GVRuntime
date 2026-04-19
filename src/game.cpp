#include "game.h"
#include "Platform/PSP/PSPSystem.h"
#include "Framework/ScenesHandler/SceneLoader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Renderer/Renderer.h"

#include <pspdebug.h>
#include <pspctrl.h>
#include <pspkernel.h>

PSP_MODULE_INFO("GravitasRuntime", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

namespace GV
{
    static void InitDebug()
    {
        pspDebugScreenInit();
        pspDebugScreenClear();
    }

    static uint8_t g_renderMemory[2 * 1024 * 1024];

    bool Game::Initialize()
    {
        InitDebug();

        pspDebugScreenPrintf("Initializing Game...\n");

        PSPSystem::Initialize();

        GraphicsInitDesc desc;
        desc.frameWidth  = 480;
        desc.frameHeight = 272;
        desc.enableDepth = true;

        if (!m_graphics.Initialize(desc))
        {
            pspDebugScreenPrintf("Graphics init FAILED\n");
            return false;
        }

        pspDebugScreenPrintf("Graphics initialized\n");

        Renderer::Init(g_renderMemory, sizeof(g_renderMemory));
        pspDebugScreenPrintf("Renderer initialized\n");

        SceneFile scene;

        if (!SceneLoader::Load("test_scene.bin", scene))
        {
            pspDebugScreenPrintf("Scene load FAILED\n");
            return false;
        }

        pspDebugScreenPrintf("Scene loaded\n");
        pspDebugScreenPrintf("Chunks: %d\n", scene.chunks.size());

        for (size_t i = 0; i < scene.chunks.size() && i < 10; i++)
        {
            const SceneChunk& c = scene.chunks[i];

            pspDebugScreenPrintf(
                "Chunk %d: type=%08X size=%u depth=%d\n",
                i,
                c.type,
                c.size,
                c.depth
            );
        }

        m_running = true;
        return true;
    }

    void Game::Run()
    {
        if (!m_running)
            return;

        pspDebugScreenPrintf("Entering main loop...\n");

        while (m_running)
        {
            if (PSPSystem::ShouldExit())
            {
                m_running = false;
                break;
            }

            SceCtrlData pad;
            sceCtrlPeekBufferPositive(&pad, 1);

            if (pad.Buttons & PSP_CTRL_START)
            {
                // MessageHandler::Send("RenderCamera");
            }

            if (pad.Buttons & PSP_CTRL_CROSS)
            {
                // MessageHandler::Send("RenderCamera2");
                MessageHandler::Send("RenderTexture");
            }

            Color32 clearColor;
            clearColor.r = 0;
            clearColor.g = 0;
            clearColor.b = 0;
            clearColor.a = 255;

            m_graphics.BeginFrame(clearColor);

            Renderer::DrawQuads();

            m_graphics.EndFrame();
        }
    }

    void Game::Shutdown()
    {
        pspDebugScreenPrintf("Shutting down...\n");

        m_graphics.Shutdown();
    }
}