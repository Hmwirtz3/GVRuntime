#pragma once


#include <cstdint>
#include <string>
#include <vector>

#ifdef GV_Editor

BEGIN_LOGIC_UNIT(InputController,GV_CHUNK_CONTROLLER)

    UI_MESSAGE(Cross_Message, "", "")
    UI_PARAM_BOOL(Cross_Continuous, false, "")

    UI_MESSAGE(Circle_Message, "", "")
    UI_PARAM_BOOL(Circle_Continuous, false, "")

    UI_MESSAGE(Square_Message, "", "")
    UI_PARAM_BOOL(Square_Continuous, false, "")

    UI_MESSAGE(Triangle_Message, "", "")
    UI_PARAM_BOOL(Triangle_Continuous, false, "")

    UI_MESSAGE(Start_Message, "", "")
    UI_PARAM_BOOL(Start_Continuous, false, "")

    UI_MESSAGE(Select_Message, "", "")
    UI_PARAM_BOOL(Select_Continuous, false, "")

    UI_MESSAGE(L1_Message, "", "")
    UI_PARAM_BOOL(L1_Continuous, false, "")

    UI_MESSAGE(R1_Message, "", "")
    UI_PARAM_BOOL(R1_Continuous, false, "")

    UI_MESSAGE(DPadUp_Message, "", "")
    UI_PARAM_BOOL(DPadUp_Continuous, false, "")

    UI_MESSAGE(DPadDown_Message, "", "")
    UI_PARAM_BOOL(DPadDown_Continuous, false, "")

    UI_MESSAGE(DPadLeft_Message, "", "")
    UI_PARAM_BOOL(DPadLeft_Continuous, false, "")

    UI_MESSAGE(DPadRight_Message, "", "")
    UI_PARAM_BOOL(DPadRight_Continuous, false, "")

    UI_PARAM_FLOAT(LeftStick_DeadZone, 0.1f, "")
    UI_PARAM_FLOAT(RightStick_DeadZone, 0.1f, "")

    UI_PARAM_FLOAT(LeftStick_Sensitivity, 1.0f, "")
    UI_PARAM_FLOAT(RightStick_Sensitivity, 1.0f, "")

    UI_PARAM_FLOAT(LeftStick_Expo, 1.0f, "")
    UI_PARAM_FLOAT(RightStick_Expo, 1.0f, "")

    UI_PARAM_BOOL(LeftStick_InvertX, false, "")
    UI_PARAM_BOOL(LeftStick_InvertY, false, "")

    UI_PARAM_BOOL(RightStick_InvertX, false, "")
    UI_PARAM_BOOL(RightStick_InvertY, false, "")

    UI_PARAM_FLOAT(LeftStick_Min, 0.0f, "")
    UI_PARAM_FLOAT(RightStick_Min, 0.0f, "")

    UI_PARAM_FLOAT(LeftStick_Max, 1.0f, "")
    UI_PARAM_FLOAT(RightStick_Max, 1.0f, "")

END_LOGIC_UNIT

#endif

namespace GV
{
    struct InputControllerData
    {
        std::string Cross_Message; bool Cross_Continuous;
        std::string Circle_Message; bool Circle_Continuous;
        std::string Square_Message; bool Square_Continuous;
        std::string Triangle_Message; bool Triangle_Continuous;
        std::string Start_Message; bool Start_Continuous;
        std::string Select_Message; bool Select_Continuous;
        std::string L1_Message; bool L1_Continuous;
        std::string R1_Message; bool R1_Continuous;

        std::string DPadUp_Message; bool DPadUp_Continuous;
        std::string DPadDown_Message; bool DPadDown_Continuous;
        std::string DPadLeft_Message; bool DPadLeft_Continuous;
        std::string DPadRight_Message; bool DPadRight_Continuous;

        float LeftStick_DeadZone;
        float RightStick_DeadZone;

        float LeftStick_Sensitivity;
        float RightStick_Sensitivity;

        float LeftStick_Expo;
        float RightStick_Expo;

        bool LeftStick_InvertX;
        bool LeftStick_InvertY;

        bool RightStick_InvertX;
        bool RightStick_InvertY;

        float LeftStick_Min;
        float RightStick_Min;

        float LeftStick_Max;
        float RightStick_Max;
    };

    class InputController
    {
    public:
        static void Load(const std::vector<uint8_t>& bytes, uint32_t start, uint32_t end);
        static void HandleMessage(uint32_t, const std::string&);

        void OnStart();
        void OnUpdate(float dt);

    private:
        void UpdateButtons();
        void UpdateLeftStick();
        void UpdateRightStick();

        void HandleButton(bool current, bool& previous, const std::string& message, bool continuous);

        float ApplyDeadZone(float value, float deadZone);
        float ApplyResponse(float value, float sensitivity, float expo);
        float ApplyClamp(float value, float minVal, float maxVal);

    private:
        bool m_prevCross{};
        bool m_prevCircle{};
        bool m_prevSquare{};
        bool m_prevTriangle{};
        bool m_prevStart{};
        bool m_prevSelect{};
        bool m_prevL1{};
        bool m_prevR1{};

        bool m_prevDPadUp{};
        bool m_prevDPadDown{};
        bool m_prevDPadLeft{};
        bool m_prevDPadRight{};
    };
}