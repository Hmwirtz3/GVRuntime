#include "Modules/Camera/CameraSystem.h"
#include <cmath>

namespace GV
{
    std::vector<Camera> CameraSystem::s_cameras;
    int CameraSystem::s_activeCamera = -1;

    Camera CameraSystem::s_defaultCamera =
    {
        0.0f, 0.0f, -5.0f,
        0.0f, 0.0f, 0.0f,
        47.0f,
        0.1f,
        1000.0f
    };

    static float PI() { return 3.14159265359f; }
    static float TWO_PI() { return 6.28318530718f; }

    Camera& CameraSystem::GetActiveCamera()
    {
        if (s_cameras.empty())
            return s_defaultCamera;

        if (s_activeCamera < 0 || s_activeCamera >= (int)s_cameras.size())
            return s_defaultCamera;

        return s_cameras[s_activeCamera];
    }

    const Camera& CameraSystem::GetActiveCameraConst()
    {
        if (s_cameras.empty())
            return s_defaultCamera;

        if (s_activeCamera < 0 || s_activeCamera >= (int)s_cameras.size())
            return s_defaultCamera;

        return s_cameras[s_activeCamera];
    }

    void CameraSystem::AddCamera(const Camera& cam)
    {
        s_cameras.push_back(cam);

        if (s_activeCamera < 0)
            s_activeCamera = 0;
    }

    void CameraSystem::SetActiveCamera(int index)
    {
        if (index >= 0 && index < (int)s_cameras.size())
            s_activeCamera = index;
    }

    int CameraSystem::GetCount()
    {
        return (int)s_cameras.size();
    }

    void CameraSystem::Clear()
    {
        s_cameras.clear();
        s_activeCamera = -1;
    }

    void CameraSystem::MoveForward(float amount)
    {
        Camera& cam = GetActiveCamera();

        float yaw = -cam.rotY;

        float cp = std::cos(cam.rotX);
        float sp = std::sin(cam.rotX);
        float cy = std::cos(yaw);
        float sy = std::sin(yaw);

        float fx = -sy * cp;
        float fy = sp;
        float fz = cy * cp;

        cam.posX += fx * amount;
        cam.posY += fy * amount;
        cam.posZ += fz * amount;
    }

    void CameraSystem::MoveRight(float amount)
    {
        Camera& cam = GetActiveCamera();

        float yaw = -cam.rotY;

        float cy = std::cos(yaw);
        float sy = std::sin(yaw);

        float rx = cy;
        float rz = sy;

        cam.posX += rx * amount;
        cam.posZ += rz * amount;
    }

    void CameraSystem::MoveUp(float amount)
    {
        Camera& cam = GetActiveCamera();
        cam.posY += amount;
    }

    void CameraSystem::AddRotation(float pitchDelta, float yawDelta)
    {
        Camera& cam = GetActiveCamera();

        cam.rotX += pitchDelta;
        cam.rotY += yawDelta;

        while (cam.rotY > PI())  cam.rotY -= TWO_PI();
        while (cam.rotY < -PI()) cam.rotY += TWO_PI();

        ClampPitch();
    }

    void CameraSystem::ClampPitch()
    {
        Camera& cam = GetActiveCamera();

        float limit = 1.55f;

        if (cam.rotX >  limit) cam.rotX =  limit;
        if (cam.rotX < -limit) cam.rotX = -limit;
    }

    void CameraSystem::SetPosition(float x, float y, float z)
    {
        Camera& cam = GetActiveCamera();

        cam.posX = x;
        cam.posY = y;
        cam.posZ = z;
    }

    void CameraSystem::SetRotation(float pitch, float yaw, float roll)
    {
        Camera& cam = GetActiveCamera();

        cam.rotX = pitch;
        cam.rotY = yaw;
        cam.rotZ = roll;

        ClampPitch();
    }

    void CameraSystem::SetLens(float fov, float nearClip, float farClip)
    {
        Camera& cam = GetActiveCamera();

        cam.fov = fov;
        cam.nearClip = nearClip;
        cam.farClip = farClip;
    }

    void CameraSystem::LookAt(float x, float y, float z)
    {
        Camera& cam = GetActiveCamera();

        float dx = x - cam.posX;
        float dy = y - cam.posY;
        float dz = z - cam.posZ;

        float yaw = std::atan2(dx, dz);
        float dist = std::sqrt(dx * dx + dz * dz);
        float pitch = std::atan2(-dy, dist);

        cam.rotX = pitch;
        cam.rotY = yaw;
        cam.rotZ = 0.0f;

        ClampPitch();
    }

    float CameraSystem::LerpAngle(float a, float b, float t)
    {
        float diff = b - a;

        while (diff > PI())  diff -= TWO_PI();
        while (diff < -PI()) diff += TWO_PI();

        return a + diff * t;
    }

    void CameraSystem::BlendTo(const Camera& from, const Camera& to, float t)
    {
        if (t < 0) t = 0;
        if (t > 1) t = 1;

        Camera& cam = GetActiveCamera();

        cam.posX = from.posX + (to.posX - from.posX) * t;
        cam.posY = from.posY + (to.posY - from.posY) * t;
        cam.posZ = from.posZ + (to.posZ - from.posZ) * t;

        cam.rotX = LerpAngle(from.rotX, to.rotX, t);
        cam.rotY = LerpAngle(from.rotY, to.rotY, t);
        cam.rotZ = LerpAngle(from.rotZ, to.rotZ, t);

        cam.fov = from.fov + (to.fov - from.fov) * t;
        cam.nearClip = from.nearClip + (to.nearClip - from.nearClip) * t;
        cam.farClip = from.farClip + (to.farClip - from.farClip) * t;

        ClampPitch();
    }
}