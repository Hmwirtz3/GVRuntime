#include "Platform/PSP/PSPSystem.h"

#include <pspkernel.h>

namespace GV
{
    static volatile bool g_shouldExit = false;

    static int ExitCallback(int, int, void*)
    {
        g_shouldExit = true;
        return 0;
    }

    static int CallbackThread(SceSize, void*)
    {
        int cbid = sceKernelCreateCallback("Exit Callback", ExitCallback, nullptr);
        sceKernelRegisterExitCallback(cbid);
        sceKernelSleepThreadCB();
        return 0;
    }

    void PSPSystem::Initialize()
    {
        int thid = sceKernelCreateThread(
            "exit_thread",
            CallbackThread,
            0x11,
            0xFA0,
            0,
            nullptr
        );

        if (thid >= 0)
        {
            sceKernelStartThread(thid, 0, nullptr);
        }
    }

    bool PSPSystem::ShouldExit()
    {
        return g_shouldExit;
    }
}