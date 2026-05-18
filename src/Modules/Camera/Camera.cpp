#include "Modules/Camera/Camera.h"
#include "Modules/Camera/CameraSystem.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <cstdio>
#include <pspiofilemgr.h>
#include <stdarg.h>

static uint32_t g_cameraCount = 0;

namespace GV
{
    static void WriteLog(const char* fmt, ...)
    {
        SceUID fd = sceIoOpen("Cameralog.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
        if (fd < 0) return;

        char buf[256];
        buf[0] = '\0';

        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if (len > 0 && len < (int)sizeof(buf))
            sceIoWrite(fd, buf, len);

        sceIoClose(fd);
    }

    static uint32_t Align16(uint32_t value)
    {
        return (value + 15u) & ~15u;
    }

    void Camera::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        printf("Camera::Load [%u - %u]\n", start, end);
        WriteLog("[DISPATCH] Camera [%u - %u]\n", start, end);

        const uint8_t* base = bytes.data();
        const uint8_t* ptr  = base + start;

        // skip SceneObject header + padding
        ptr += sizeof(GV_ChunkHeader) + 4;

        // skip Camera header + padding
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = ReadUInt32(ptr);
        WriteLog("  ParamCount: %u\n", paramCount);

        Camera cam{};

        if (paramCount >= 11)
        {
            cam.posX = ReadFloat(ptr);
            cam.posY = ReadFloat(ptr);
            cam.posZ = ReadFloat(ptr);

            cam.rotX = ReadFloat(ptr);
            cam.rotY = ReadFloat(ptr);
            cam.rotZ = ReadFloat(ptr);

            cam.fov      = ReadFloat(ptr);
            cam.nearClip = ReadFloat(ptr);
            cam.farClip  = ReadFloat(ptr);

            bool isPerspective = ReadBool(ptr);

            const char* activateMsg = ReadString(ptr);

            WriteLog("  ---- Camera Values ----\n");
            WriteLog("  Position: (%f, %f, %f)\n", cam.posX, cam.posY, cam.posZ);
            WriteLog("  Rotation: (%f, %f, %f)\n", cam.rotX, cam.rotY, cam.rotZ);
            WriteLog("  FOV: %f\n", cam.fov);
            WriteLog("  Near: %f\n", cam.nearClip);
            WriteLog("  Far: %f\n", cam.farClip);
            WriteLog("  Perspective: %d\n", isPerspective ? 1 : 0);
            WriteLog("  ActivateMsg: %s\n", activateMsg);

            
            uint32_t cameraIndex = g_cameraCount++;

            WriteLog("  Before Register\n");
            MessageHandler::Register(activateMsg, GV_CHUNK_CAMERA, cameraIndex);
        }
        else
        {
            WriteLog("  WARNING: Unexpected param count: %u\n", paramCount);
        }

        uint32_t offset = (uint32_t)(ptr - base);
        offset = Align16(offset);
        ptr = base + offset;

        CameraSystem::AddCamera(cam);
    }

    void Camera::HandleMessage(uint32_t index,
                           const std::string& msg,
                           uint32_t senderType,
                           uint32_t senderIndex,
                           const void* payload,
                           uint32_t payloadSize)
{
    WriteLog("[Camera::HandleMessage] index=%u msg=%s\n", index, msg.c_str());

    WriteLog("  Activating camera %u\n", index);

    CameraSystem::SetActiveCamera(index);
}
}