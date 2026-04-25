#pragma once

#include <vector>
#include "Modules/Camera/Camera.h"

namespace GV
{
    class CameraSystem
    {
    public:
        static void AddCamera(const Camera& cam);

        static Camera& GetActiveCamera();
        static const Camera& GetActiveCameraConst();

        static void SetActiveCamera(int index);

        static void MoveForward(float amount);
        static void MoveRight(float amount);
        static void MoveUp(float amount);

        static void AddRotation(float pitchDelta, float yawDelta);
        static void ClampPitch();

        static void SetPosition(float x, float y, float z);
        static void SetRotation(float pitch, float yaw, float roll);
        static void SetLens(float fov, float nearClip, float farClip);

        static void LookAt(float x, float y, float z);
        static void BlendTo(const Camera& from, const Camera& to, float t);

        static void Clear();

        static int GetCount();

    private:
        static float LerpAngle(float a, float b, float t);

        static std::vector<Camera> s_cameras;
        static int s_activeCamera;

        static Camera s_defaultCamera;
    };
}