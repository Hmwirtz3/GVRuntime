#pragma once

#include <vector>
#include <string>
#include <cstdint>

#ifdef GV_EDITOR
BEGIN_LOGIC_UNIT(Button, GV_CHUNK_BUTTON)

    UI_SEPARATOR("Transform")

    UI_PARAM_FLOAT(posX, 0.0f, "Position X")
    UI_PARAM_FLOAT(posY, 0.0f, "Position Y")

    UI_PARAM_FLOAT(scaleX, 1.0f, "Scale X")
    UI_PARAM_FLOAT(scaleY, 1.0f, "Scale Y")

    UI_PARAM_FLOAT(rotation, 0.0f, "Rotation")

    UI_SEPARATOR("Identity")

    UI_PARAM_STRING(buttonID, "", "Unique Button ID")

    UI_SEPARATOR("Navigation")

    UI_PARAM_STRING(upID, "", "Up Button")
    UI_PARAM_STRING(downID, "", "Down Button")
    UI_PARAM_STRING(leftID, "", "Left Button")
    UI_PARAM_STRING(rightID, "", "Right Button")

    UI_SEPARATOR("Visual")

    UI_PARAM_ASSET(textureIdle, "", "Idle Texture")
    UI_PARAM_FLOAT(idleWidth, 64.0f, "Idle Width")
    UI_PARAM_FLOAT(idleHeight, 32.0f, "Idle Height")
    UI_PARAM_FLOAT(idleAlpha, 0.15f, "Idle Alpha")

    UI_PARAM_ASSET(textureSelected, "", "Selected Texture")
    UI_PARAM_FLOAT(selectedWidth, 64.0f, "Selected Width")
    UI_PARAM_FLOAT(selectedHeight, 32.0f, "Selected Height")
    UI_PARAM_FLOAT(selectedAlpha, 1.0f, "Selected Alpha")

    UI_PARAM_ASSET(textureDisabled, "", "Disabled Texture")
    UI_PARAM_FLOAT(disabledWidth, 64.0f, "Disabled Width")
    UI_PARAM_FLOAT(disabledHeight, 32.0f, "Disabled Height")
    UI_PARAM_FLOAT(disabledAlpha, 0.35f, "Disabled Alpha")

    UI_PARAM_FLOAT(selectedScale, 1.03f, "Selected Scale")

    UI_PARAM_FLOAT(highlightFadeSpeed, 8.0f, "Highlight Fade Speed")
    UI_PARAM_FLOAT(selectionInterpSpeed, 8.0f, "Selection Interp Speed")

    UI_PARAM_FLOAT(glowIntensity, 0.1f, "Glow Intensity")

    UI_PARAM_BOOL(enablePulse, false, "Enable Pulse")
    UI_PARAM_FLOAT(pulseAmount, 0.01f, "Pulse Amount")

    UI_PARAM_INT(colorIdle, 0xFFFFFFFF, "Idle Color")
    UI_PARAM_INT(colorSelected, 0xFFFFFFFF, "Selected Color")
    UI_PARAM_INT(colorDisabled, 0x80FFFFFF, "Disabled Color")

    UI_SEPARATOR("Text")

    UI_PARAM_STRING(label, "", "Button Text")

    UI_PARAM_INT(textColorIdle, 0xC0B090FF, "Text Color Idle")
    UI_PARAM_INT(textColorSelected, 0xFFFFFFFF, "Text Color Selected")
    UI_PARAM_INT(textColorDisabled, 0x808080FF, "Text Color Disabled")

    UI_PARAM_FLOAT(textScale, 1.0f, "Text Scale")

    UI_PARAM_FLOAT(textOffsetX, 0.0f, "Text Offset X")
    UI_PARAM_FLOAT(textOffsetY, 0.0f, "Text Offset Y")

    UI_PARAM_BOOL(textCentered, true, "Center Text")

    UI_PARAM_BOOL(enableTextShadow, true, "Enable Text Shadow")

    UI_PARAM_FLOAT(shadowOffsetX, 1.0f, "Shadow Offset X")
    UI_PARAM_FLOAT(shadowOffsetY, 1.0f, "Shadow Offset Y")

    UI_PARAM_INT(shadowColor, 0x60000000, "Shadow Color")

    UI_SEPARATOR("Behavior")

    UI_MESSAGE(onActivate, "", "Message sent when activated")
    UI_MESSAGE(onSelect, "", "Message sent when selected")
    UI_MESSAGE(onDeselect, "", "Message sent when deselected")

    UI_PARAM_BOOL(isEnabled, true, "Whether the button is enabled")
    UI_PARAM_BOOL(isVisible, true, "Whether the button is visible")

    UI_PARAM_BOOL(startsSelected, false, "Start selected")

    UI_SEPARATOR("Input")

    UI_MESSAGE(inputUp, "", "Message to listen for Up input")
    UI_MESSAGE(inputDown, "", "Message to listen for Down input")
    UI_MESSAGE(inputLeft, "", "Message to listen for Left input")
    UI_MESSAGE(inputRight, "", "Message to listen for Right input")

    UI_MESSAGE(inputActivate, "", "Message to listen for Activate input")
    UI_MESSAGE(inputCancel, "", "Message to listen for Cancel input")

    UI_SEPARATOR("State")

    UI_MESSAGE(toggleEnabled, "", "Toggle enabled state")
    UI_MESSAGE(setEnabled, "", "Set enabled state")

    UI_MESSAGE(toggleVisible, "", "Toggle visibility")
    UI_MESSAGE(setVisible, "", "Set visibility")

    UI_MESSAGE(selectButton, "", "Force select button")
    UI_MESSAGE(deselectButton, "", "Force deselect button")

END_LOGIC_UNIT
#endif

namespace GV
{
    struct ButtonData
    {
        float posX;
        float posY;

        float scaleX;
        float scaleY;

        float rotation;

        std::string buttonID;

        std::string upID;
        std::string downID;
        std::string leftID;
        std::string rightID;

        float idleWidth;
        float idleHeight;
        float idleAlpha;

        float selectedWidth;
        float selectedHeight;
        float selectedAlpha;

        float disabledWidth;
        float disabledHeight;
        float disabledAlpha;

        float selectedScale;

        float highlightFadeSpeed;
        float selectionInterpSpeed;

        float glowIntensity;

        bool enablePulse;
        float pulseAmount;

        int colorIdle;
        int colorSelected;
        int colorDisabled;

        std::string label;

        int textColorIdle;
        int textColorSelected;
        int textColorDisabled;

        float textScale;

        float textOffsetX;
        float textOffsetY;

        bool textCentered;

        bool enableTextShadow;

        float shadowOffsetX;
        float shadowOffsetY;

        int shadowColor;

        std::string onActivate;
        std::string onSelect;
        std::string onDeselect;

        bool isEnabled;
        bool isVisible;
        bool isSelected;

        bool startsSelected;

        float currentHighlight;
        float currentScale;

        std::string inputUp;
        std::string inputDown;
        std::string inputLeft;
        std::string inputRight;

        std::string inputActivate;
        std::string inputCancel;

        std::string toggleEnabled;
        std::string setEnabled;

        std::string toggleVisible;
        std::string setVisible;

        std::string selectButton;
        std::string deselectButton;

        uint32_t textureIdle;
        uint32_t textureSelected;
        uint32_t textureDisabled;
    };

    class Button
    {
    public:
        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end);

        static void HandleMessage(
            uint32_t index,
            const std::string& msg,
            uint32_t senderType,
            uint32_t senderIndex,
            const void* payload,
            uint32_t payloadSize);

        static void BuildRenderData(
            uint32_t index,
            void* dst);

        static void MoveSelection(
            const std::string& direction);

        static void SetActive(
            uint32_t index);

        static uint32_t GetActive();

        static uint32_t GetCount();

        static const ButtonData& Get(
            uint32_t index);
    };
}