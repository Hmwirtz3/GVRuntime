#include "game.h"
#include "Platform/PSP/PSPSystem.h"
#include "Framework/ScenesHandler/SceneLoader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Renderer/Renderer.h"
#include "Modules/Camera/CameraSystem.h"
#include "Modules/UI/Button.h"
#include "Modules/ControlInput/Controller.h"
#include "Modules/AreaTriggerBox/AreaTriggerBox.h"
#include "Modules/Audio/AudioSource.h"

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
        SceneFile player;

        CameraSystem::Clear();

        if (!SceneLoader::Load("Player.bin", player))
            return false;

        if (!SceneLoader::Load("UI.bin", scene))
            return false;

        InputController::GetController().OnStart();

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

            InputController::GetController().OnUpdate(0.0f);

            sceCtrlPeekBufferPositive(&pad, 1);

            bool startPressed = (pad.Buttons & PSP_CTRL_START) != 0;

            if (startPressed && !prevStartPressed)
            {
                SceneFile scene2;

                CameraSystem::Clear();

                if (SceneLoader::Load("Daggerfall.bin", scene2))
                    hideMenuText = true;

                MessageHandler::DebugSend("DisableMenu");
            }

            prevStartPressed = startPressed;

            Renderer::BeginFrame();

            Renderer::DrawStaticMeshes();
            Renderer::DrawQuads();

            AreaTriggerBox::Update();
            AudioSource::Update();

            Renderer::EndFrame();

            BeginUI();

            Font_Begin();

            if (!hideMenuText)
            {
                //DrawText("Continue", 60.0f, 200.0f, 1.0f, 0xFF5A6F86);
                //DrawText("New",      140.0f, 200.0f, 1.0f, 0xFF2F3E4E);
                //DrawText("Load",     200.0f, 200.0f, 1.00f, 0xFF2F3E4E);
                //DrawText("Options",  260.0f, 200.0f, 1.0f, 0xFF2F3E4E);
                //DrawText("Credits",  340.0f, 200.0f, 1.0f, 0xFF2F3E4E);
                //DrawText("Exit",     420.0f, 200.0f, 1.0f, 0xFF2F3E4E);
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