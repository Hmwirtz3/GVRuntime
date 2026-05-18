#include "Modules/UI/Button.h"
#include "Modules/UI/TexturedQuad.h"

#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Font/FontRenderer.h"

#include <cstring>

static uint32_t g_buttonCount = 0;
static int g_activeButton = -1;
static bool g_navDispatchLocked = false;
static int g_navDispatchActive = -1;

namespace GV
{
    static std::vector<ButtonData> g_buttons;

    static void Align16(const uint8_t*& ptr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;
        ptr = (const uint8_t*)p;
    }

    static std::string ReadStringValue(const uint8_t*& ptr)
    {
        uint32_t len = ReadUInt32(ptr);

        std::string out;
        if (len > 0)
            out.assign((const char*)ptr, len);

        ptr += len;
        return out;
    }

    static bool StringsMatch(const std::string& a, const std::string& b)
    {
        return std::strcmp(a.c_str(), b.c_str()) == 0;
    }

    void Button::Load(const std::vector<uint8_t>& bytes, uint32_t start, uint32_t end)
    {
        const uint8_t* base = bytes.data();
        const uint8_t* ptr  = base + start;

        if (ptr >= base + end)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = ReadUInt32(ptr);

        if (paramCount < 20)
            return;

        ButtonData b{};
        b.isSelected = false;

        b.posX = ReadFloat(ptr);
        b.posY = ReadFloat(ptr);

        b.buttonID = ReadStringValue(ptr);

        b.upID = ReadStringValue(ptr);
        b.downID = ReadStringValue(ptr);
        b.leftID = ReadStringValue(ptr);
        b.rightID = ReadStringValue(ptr);

        b.idleWidth = ReadFloat(ptr);
        b.idleHeight = ReadFloat(ptr);

        b.selectedWidth = ReadFloat(ptr);
        b.selectedHeight = ReadFloat(ptr);

        b.disabledWidth = ReadFloat(ptr);
        b.disabledHeight = ReadFloat(ptr);

        b.label = ReadStringValue(ptr);

        b.textColorIdle = ReadInt(ptr);
        b.textColorSelected = ReadInt(ptr);
        b.textColorDisabled = ReadInt(ptr);

        b.onActivate = ReadStringValue(ptr);

        b.isEnabled = ReadBool(ptr);
        b.isVisible = ReadBool(ptr);

        if (paramCount >= 30)
        {
            b.inputUp = ReadStringValue(ptr);
            b.inputDown = ReadStringValue(ptr);
            b.inputLeft = ReadStringValue(ptr);
            b.inputRight = ReadStringValue(ptr);
            b.inputActivate = ReadStringValue(ptr);
            b.inputCancel = ReadStringValue(ptr);

            b.toggleEnabled = ReadStringValue(ptr);
            b.setEnabled = ReadStringValue(ptr);

            b.toggleVisible = ReadStringValue(ptr);
            b.setVisible = ReadStringValue(ptr);
        }
        else
        {
            for (uint32_t i = 20; i < paramCount; i++)
                ReadStringValue(ptr);
        }

        Align16(ptr);

        b.textureIdle = ReadUInt32(ptr);
        b.textureSelected = ReadUInt32(ptr);
        b.textureDisabled = ReadUInt32(ptr);

        uint32_t index = g_buttonCount++;

        if (g_buttons.size() <= index)
            g_buttons.resize(index + 1);

        g_buttons[index] = b;

        if (g_activeButton == -1 && b.isEnabled)
        {
            g_activeButton = (int)index;
            g_buttons[index].isSelected = true;
        }

        auto reg = [&](const std::string& m)
        {
            if (!m.empty())
                MessageHandler::Register(m, GV_CHUNK_BUTTON, index);
        };

        reg(b.inputUp);
        reg(b.inputDown);
        reg(b.inputLeft);
        reg(b.inputRight);
        reg(b.inputActivate);
        reg(b.inputCancel);

        reg(b.toggleEnabled);
        reg(b.setEnabled);
        reg(b.toggleVisible);
        reg(b.setVisible);

        reg(b.onActivate);
    }

    void Button::HandleMessage(uint32_t index,
                               const std::string& msg,
                               uint32_t,
                               uint32_t,
                               const void* payload,
                               uint32_t payloadSize)
    {
        
        if (index >= g_buttons.size())
            return;

        if (!g_navDispatchLocked)
        {
            g_navDispatchLocked = true;
            g_navDispatchActive = g_activeButton;
        }

        ButtonData& b = g_buttons[index];

        if ((int)index == g_navDispatchActive)
        {
            if (StringsMatch(msg, b.inputUp))
                MoveSelection("up");
            else if (StringsMatch(msg, b.inputDown))
                MoveSelection("down");
            else if (StringsMatch(msg, b.inputLeft))
                MoveSelection("left");
            else if (StringsMatch(msg, b.inputRight))
                MoveSelection("right");
            else if (StringsMatch(msg, b.inputActivate))
            {
                if (b.isEnabled && !b.onActivate.empty())
                    MessageHandler::Send(b.onActivate, GV_CHUNK_BUTTON, index);
            }
            else if (StringsMatch(msg, b.inputCancel))
            {
            }
        }

        if (StringsMatch(msg, b.toggleEnabled))
        {
            b.isEnabled = !b.isEnabled;
        }
        else if (StringsMatch(msg, b.setEnabled) && payload && payloadSize == sizeof(bool))
        {
            b.isEnabled = *(const bool*)payload;
        }

        if (StringsMatch(msg, b.toggleVisible))
        {
            b.isVisible = !b.isVisible;
        }
        else if (StringsMatch(msg, b.setVisible) && payload && payloadSize == sizeof(bool))
        {
            b.isVisible = *(const bool*)payload;
        }

        if (index == g_buttons.size() - 1)
        {
            g_navDispatchLocked = false;
            g_navDispatchActive = -1;
        }
    }

    void Button::SetActive(uint32_t index)
    {
        if (index >= g_buttons.size())
            return;

        if (g_activeButton >= 0 && g_activeButton < (int)g_buttons.size())
            g_buttons[g_activeButton].isSelected = false;

        g_activeButton = (int)index;
        g_buttons[index].isSelected = true;
    }

    void Button::MoveSelection(const std::string& direction)
    {
        if (g_activeButton < 0)
            return;

        ButtonData& current = g_buttons[g_activeButton];

        std::string nextID;

        if (direction == "up") nextID = current.upID;
        else if (direction == "down") nextID = current.downID;
        else if (direction == "left") nextID = current.leftID;
        else if (direction == "right") nextID = current.rightID;

        if (nextID.empty())
            return;

        for (uint32_t i = 0; i < g_buttons.size(); i++)
        {
            if (StringsMatch(g_buttons[i].buttonID, nextID) && g_buttons[i].isEnabled)
            {
                SetActive(i);
                return;
            }
        }
    }

    uint32_t Button::GetActive()
    {
        return (uint32_t)g_activeButton;
    }

    uint32_t Button::GetCount()
    {
        return (uint32_t)g_buttons.size();
    }

    const ButtonData& Button::Get(uint32_t index)
    {
        return g_buttons[index];
    }

    void Button::BuildRenderData(uint32_t index, void* dst)
    {
        if (index >= g_buttons.size() || !dst)
            return;

        const ButtonData& b = g_buttons[index];

        QuadGpuData* out = (QuadGpuData*)dst;

        uint32_t textureID = b.textureIdle;
        float w = b.idleWidth;
        float h = b.idleHeight;

        if (!b.isEnabled)
        {
            textureID = b.textureDisabled;
            w = b.disabledWidth;
            h = b.disabledHeight;
        }
        else if (b.isSelected)
        {
            textureID = b.textureSelected;
            w = b.selectedWidth;
            h = b.selectedHeight;
        }

        float x0 = b.posX;
        float y0 = b.posY;
        float x1 = x0 + w;
        float y1 = y0 + h;

        out->verts[0] = { 0, 0, x0, y0, 0 };
        out->verts[1] = { w, 0, x1, y0, 0 };
        out->verts[2] = { w, h, x1, y1, 0 };

        out->verts[3] = { 0, 0, x0, y0, 0 };
        out->verts[4] = { w, h, x1, y1, 0 };
        out->verts[5] = { 0, h, x0, y1, 0 };

        out->textureID = textureID;
    }
}