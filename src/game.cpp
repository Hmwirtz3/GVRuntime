#include "game.h"
#include "Platform/PSP/PSPSystem.h"
#include "Framework/ScenesHandler/SceneLoader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Renderer/Renderer.h"
#include "Modules/Camera/CameraSystem.h"

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

        if (!SceneLoader::Load("Anticlere.bin", scene))
        {
            pspDebugScreenPrintf("Scene load FAILED\n");
            return false;
        }

        pspDebugScreenPrintf("Scene loaded\n");

        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

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

            SceCtrlData pad{};
            sceCtrlPeekBufferPositive(&pad, 1);

            // -----------------------------
            // ANALOG INPUT
            // -----------------------------
            float ax = (pad.Lx - 128) / 128.0f;
            float ay = (pad.Ly - 128) / 128.0f;

            const float deadzone = 0.15f;
            if (ax > -deadzone && ax < deadzone) ax = 0.0f;
            if (ay > -deadzone && ay < deadzone) ay = 0.0f;

            float moveSpeed = 0.15f;
            float rotSpeed  = 0.01f;

            // -----------------------------
            // ROTATION (YAW)
            // -----------------------------
            if (pad.Buttons & PSP_CTRL_LTRIGGER)
                CameraSystem::AddRotation(0.0f, rotSpeed);

            if (pad.Buttons & PSP_CTRL_RTRIGGER)
                CameraSystem::AddRotation(0.0f, -rotSpeed);

            // -----------------------------
            // MOVEMENT (matches monolithic runtime)
            // -----------------------------
            CameraSystem::MoveForward(ay * moveSpeed);
            CameraSystem::MoveRight(ax * moveSpeed);

            // -----------------------------
            // VERTICAL
            // -----------------------------
            if (pad.Buttons & PSP_CTRL_TRIANGLE)
                CameraSystem::MoveUp(moveSpeed);

            if (pad.Buttons & PSP_CTRL_CROSS)
                CameraSystem::MoveUp(-moveSpeed);

            // -----------------------------
            // PITCH
            // -----------------------------
            if (pad.Buttons & PSP_CTRL_SQUARE)
                CameraSystem::AddRotation(rotSpeed, 0.0f);

            // -----------------------------
            // DEBUG / MESSAGES
            // -----------------------------
            if (pad.Buttons & PSP_CTRL_UP)
                MessageHandler::Send("RenderCamera2");

            if (pad.Buttons & PSP_CTRL_DOWN)
                MessageHandler::Send("RenderTexture");

            // -----------------------------
            // RENDER
            // -----------------------------
            Color32 clearColor;
            clearColor.r = 0;
            clearColor.g = 0;
            clearColor.b = 0;
            clearColor.a = 255;

            m_graphics.BeginFrame(clearColor);

            Renderer::BeginFrame();
            Renderer::DrawStaticMeshes();
            Renderer::DrawQuads();
            Renderer::EndFrame();

            m_graphics.EndFrame();
        }
    }

    void Game::Shutdown()
    {
        pspDebugScreenPrintf("Shutting down...\n");
        m_graphics.Shutdown();
    }
}