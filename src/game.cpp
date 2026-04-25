#include "game.h"
#include "Platform/PSP/PSPSystem.h"
#include "Framework/ScenesHandler/SceneLoader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Renderer/Renderer.h"
#include "Modules/Camera/CameraSystem.h"
#include "Modules/ControlInput/Controller.h"
#include "Font/FontRenderer.h"
#include "Font/FontAtlas.h"
#include "Framework/Utils/Profiler.h"

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
    static InputController g_controller;

    bool Game::Initialize()
    {
        InitDebug();

        PSPSystem::Initialize();

        GraphicsInitDesc desc;
        desc.frameWidth  = 480;
        desc.frameHeight = 272;
        desc.enableDepth = true;

        if (!m_graphics.Initialize(desc))
            return false;

        Renderer::Init(g_renderMemory, sizeof(g_renderMemory));

        SceneFile scene;

        CameraSystem::Clear();

        if (!SceneLoader::Load("Anticlere.bin", scene))
            return false;

        g_controller.OnStart();

        MessageHandler::Send("RenderCamera2");

        InitFontGlyphs();
        Font_Init();

        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

        m_running = true;
        return true;
    }

    void Game::Run()
    {
        if (!m_running)
            return;

        static bool prevSelectPressed = false;
        static bool journalOpen = false;
        static bool hideMenuText = false;
        static bool prevStartPressed = false;

        while (m_running)
        {
            Profiler_Reset();

            m_graphics.BeginFrame({ 0, 0, 0, 255 });

            if (PSPSystem::ShouldExit())
            {
                m_running = false;
                break;
            }

            SceCtrlData pad{};

            g_controller.OnUpdate(0.0f);
            sceCtrlPeekBufferPositive(&pad, 1);

            float ax = (pad.Lx - 128) / 128.0f;
            float ay = (pad.Ly - 128) / 128.0f;

            const float deadzone = 0.15f;

            if (ax > -deadzone && ax < deadzone)
                ax = 0.0f;

            if (ay > -deadzone && ay < deadzone)
                ay = 0.0f;

            float moveSpeed = 0.15f;
            float rotSpeed  = 0.01f;

            if (pad.Buttons & PSP_CTRL_LTRIGGER)
                CameraSystem::AddRotation(0.0f, rotSpeed);

            if (pad.Buttons & PSP_CTRL_RTRIGGER)
                CameraSystem::AddRotation(0.0f, -rotSpeed);

            CameraSystem::MoveForward(ay * moveSpeed);
            CameraSystem::MoveRight(ax * moveSpeed);

            if (pad.Buttons & PSP_CTRL_TRIANGLE)
                CameraSystem::MoveUp(moveSpeed);

            if (pad.Buttons & PSP_CTRL_CROSS)
                CameraSystem::MoveUp(-moveSpeed);

            if (pad.Buttons & PSP_CTRL_SQUARE)
                CameraSystem::AddRotation(rotSpeed, 0.0f);

            if (pad.Buttons & PSP_CTRL_UP)
                MessageHandler::Send("RenderCamera2");

            if (pad.Buttons & PSP_CTRL_DOWN)
                MessageHandler::Send("RenderJournal");

            bool selectPressed = (pad.Buttons & PSP_CTRL_SELECT) != 0;

            if (selectPressed && !prevSelectPressed)
            {
                if (journalOpen)
                    MessageHandler::Send("CloseJournal");
                else
                    MessageHandler::Send("RenderJournal");

                journalOpen = !journalOpen;
            }

            bool startPressed = (pad.Buttons & PSP_CTRL_START) != 0;

            if (startPressed && !prevStartPressed)
            {
                SceneFile scene2;

                CameraSystem::Clear();

                if (SceneLoader::Load("Daggerfall.bin", scene2))
                    hideMenuText = true;

                MessageHandler::Send("tests");
            }

            prevSelectPressed = selectPressed;
            prevStartPressed = startPressed;

            Renderer::BeginFrame();
            Renderer::DrawStaticMeshes();
            Renderer::DrawQuads();
            Renderer::EndFrame();

            BeginUI();

            Font_Begin();

            if (!hideMenuText)
            {
                DrawText("Continue", 60.0f, 200.0f, 0xFF5A6F86);
                DrawText("New",      140.0f, 200.0f, 0xFF2F3E4E);
                DrawText("Load",     200.0f, 200.0f, 0xFF2F3E4E);
                DrawText("Options",  260.0f, 200.0f, 0xFF2F3E4E);
                DrawText("Credits",  340.0f, 200.0f, 0xFF2F3E4E);
                DrawText("Exit",     420.0f, 200.0f, 0xFF2F3E4E);
            }

            DrawDebug();

            Profiler_Draw(10.0f, 80.0f);

            Font_End();

            m_graphics.EndFrame();
        }
    }

    void Game::Shutdown()
    {
        m_graphics.Shutdown();
    }
}