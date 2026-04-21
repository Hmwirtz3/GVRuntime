#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "Framework/LogicUnitHandler/LogicUnitMacros.h"


#ifdef GV_EDITOR
BEGIN_LOGIC_UNIT(Camera, GV_CHUNK_CAMERA)

    UI_SEPARATOR("Transform")
    UI_PARAM_FLOAT(positionX, 0.0f, "Camera world X")
    UI_PARAM_FLOAT(positionY, 0.0f, "Camera world Y")
    UI_PARAM_FLOAT(positionZ, 0.0f, "Camera world Z")
    UI_PARAM_FLOAT(rotationPitch, 0.0f, "Up/down rotation in radians")
    UI_PARAM_FLOAT(rotationYaw, 0.0f, "Left/right rotation in radians")
    UI_PARAM_FLOAT(rotationRoll, 0.0f, "Roll in radians")

    UI_SEPARATOR("Projection")
    UI_PARAM_FLOAT(fov, 47.0f, "Field of view in degrees")
    UI_PARAM_FLOAT(nearClip, 0.1f, "Near clip plane")
    UI_PARAM_FLOAT(farClip, 1000.0f, "Far clip plane")
    UI_PARAM_BOOL(isPerspective, true, "Use perspective projection")

    UI_SEPARATOR("Control")
	UI_MESSAGE(activateCamera, "", "Message that triggers camera activate")

END_LOGIC_UNIT
#endif

namespace GV
{
    class Camera
    {
    public:
        float posX, posY, posZ;
        float rotX, rotY, rotZ;
        float fov;
        float nearClip;
        float farClip;

        static void Load(const std::vector<uint8_t>& bytes, uint32_t start, uint32_t end);

        
        static void HandleMessage(uint32_t index, const std::string& msg);
    };
}