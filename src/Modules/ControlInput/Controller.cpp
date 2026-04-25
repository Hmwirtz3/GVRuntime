#include "Modules/ControlInput/Controller.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include "Font/FontRenderer.h"

#include <pspctrl.h>
#include <cmath>

namespace GV
{
    static InputControllerData g_input;

    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    void InputController::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base   = bytes.data();
        const uint8_t* ptr    = base + start;
        const uint8_t* endPtr = base + end;

        if (ptr >= endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        const GV_ChunkHeader* header = (const GV_ChunkHeader*)ptr;

        if (header->type != GV_CHUNK_CONTROLLER)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = ReadUInt32(ptr);

        if (paramCount < 38)
            return;

        g_input.Cross_Message = ReadString(ptr);
        g_input.Cross_Continuous = ReadBool(ptr);

        g_input.Circle_Message = ReadString(ptr);
        g_input.Circle_Continuous = ReadBool(ptr);

        g_input.Square_Message = ReadString(ptr);
        g_input.Square_Continuous = ReadBool(ptr);

        g_input.Triangle_Message = ReadString(ptr);
        g_input.Triangle_Continuous = ReadBool(ptr);

        g_input.Start_Message = ReadString(ptr);
        g_input.Start_Continuous = ReadBool(ptr);

        g_input.Select_Message = ReadString(ptr);
        g_input.Select_Continuous = ReadBool(ptr);

        g_input.L1_Message = ReadString(ptr);
        g_input.L1_Continuous = ReadBool(ptr);

        g_input.R1_Message = ReadString(ptr);
        g_input.R1_Continuous = ReadBool(ptr);

        g_input.DPadUp_Message = ReadString(ptr);
        g_input.DPadUp_Continuous = ReadBool(ptr);

        g_input.DPadDown_Message = ReadString(ptr);
        g_input.DPadDown_Continuous = ReadBool(ptr);

        g_input.DPadLeft_Message = ReadString(ptr);
        g_input.DPadLeft_Continuous = ReadBool(ptr);

        g_input.DPadRight_Message = ReadString(ptr);
        g_input.DPadRight_Continuous = ReadBool(ptr);

        g_input.LeftStick_DeadZone = ReadFloat(ptr);
        g_input.RightStick_DeadZone = ReadFloat(ptr);

        g_input.LeftStick_Sensitivity = ReadFloat(ptr);
        g_input.RightStick_Sensitivity = ReadFloat(ptr);

        g_input.LeftStick_Expo = ReadFloat(ptr);
        g_input.RightStick_Expo = ReadFloat(ptr);

        g_input.LeftStick_InvertX = ReadBool(ptr);
        g_input.LeftStick_InvertY = ReadBool(ptr);

        g_input.RightStick_InvertX = ReadBool(ptr);
        g_input.RightStick_InvertY = ReadBool(ptr);

        g_input.LeftStick_Min = ReadFloat(ptr);
        g_input.RightStick_Min = ReadFloat(ptr);

        g_input.LeftStick_Max = ReadFloat(ptr);
        g_input.RightStick_Max = ReadFloat(ptr);

        Align16(ptr);
    }

    void InputController::HandleMessage(uint32_t, const std::string&)
    {
    }

    void InputController::OnStart()
    {
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    }

    void InputController::OnUpdate(float)
    {
        UpdateButtons();
        UpdateLeftStick();
        UpdateRightStick();
    }

    void InputController::UpdateButtons()
    {
        SceCtrlData pad{};
        sceCtrlPeekBufferPositive(&pad, 1);

        HandleButton(pad.Buttons & PSP_CTRL_CROSS,    m_prevCross,    g_input.Cross_Message,    g_input.Cross_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_CIRCLE,   m_prevCircle,   g_input.Circle_Message,   g_input.Circle_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_SQUARE,   m_prevSquare,   g_input.Square_Message,   g_input.Square_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_TRIANGLE, m_prevTriangle, g_input.Triangle_Message, g_input.Triangle_Continuous);

        HandleButton(pad.Buttons & PSP_CTRL_START,    m_prevStart,    g_input.Start_Message,    g_input.Start_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_SELECT,   m_prevSelect,   g_input.Select_Message,   g_input.Select_Continuous);

        HandleButton(pad.Buttons & PSP_CTRL_LTRIGGER, m_prevL1,       g_input.L1_Message,       g_input.L1_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_RTRIGGER, m_prevR1,       g_input.R1_Message,       g_input.R1_Continuous);

        HandleButton(pad.Buttons & PSP_CTRL_UP,       m_prevDPadUp,    g_input.DPadUp_Message,    g_input.DPadUp_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_DOWN,     m_prevDPadDown,  g_input.DPadDown_Message,  g_input.DPadDown_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_LEFT,     m_prevDPadLeft,  g_input.DPadLeft_Message,  g_input.DPadLeft_Continuous);
        HandleButton(pad.Buttons & PSP_CTRL_RIGHT,    m_prevDPadRight, g_input.DPadRight_Message, g_input.DPadRight_Continuous);
    }

    void InputController::HandleButton(
    bool current,
    bool& previous,
    const std::string& message,
    bool continuous)
{
    if (message.empty())
    {
        previous = current;
        return;
    }

    if (continuous)
    {
        if (current)
        {
            Log("hold: %s", message.c_str());
            MessageHandler::Send(message);
        }
    }
    else
    {
        if (current && !previous)
        {
            Log("press: %s", message.c_str());
            MessageHandler::Send(message);
        }
    }

    previous = current;
}

    void InputController::UpdateLeftStick()
    {
        SceCtrlData pad{};
        sceCtrlPeekBufferPositive(&pad, 1);

        float x = ((int)pad.Lx - 128) / 128.0f;
        float y = ((int)pad.Ly - 128) / 128.0f;

        x = ApplyDeadZone(x, g_input.LeftStick_DeadZone);
        y = ApplyDeadZone(y, g_input.LeftStick_DeadZone);

        if (g_input.LeftStick_InvertX)
            x = -x;

        if (g_input.LeftStick_InvertY)
            y = -y;

        x = ApplyResponse(x, g_input.LeftStick_Sensitivity, g_input.LeftStick_Expo);
        y = ApplyResponse(y, g_input.LeftStick_Sensitivity, g_input.LeftStick_Expo);

        x = ApplyClamp(x, g_input.LeftStick_Min, g_input.LeftStick_Max);
        y = ApplyClamp(y, g_input.LeftStick_Min, g_input.LeftStick_Max);
    }

    void InputController::UpdateRightStick()
    {
    }

    float InputController::ApplyDeadZone(float value, float deadZone)
    {
        if (value > -deadZone && value < deadZone)
            return 0.0f;

        return value;
    }

    float InputController::ApplyResponse(float value, float sensitivity, float expo)
    {
        float v = value;

        if (v >= 0.0f)
            v = powf(v, expo);
        else
            v = -powf(-v, expo);

        return v * sensitivity;
    }

    float InputController::ApplyClamp(float value, float minVal, float maxVal)
    {
        if (value < minVal)
            return minVal;

        if (value > maxVal)
            return maxVal;

        return value;
    }
}