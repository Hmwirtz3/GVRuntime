#include "Framework/MessageHandler/MessageHandler.h"
#include "Framework/Chunk/ChunkTypes.h"

#include "Modules/UI/TexturedQuad.h"
#include "Modules/Camera/Camera.h"
#include "Modules/StaticMesh/StaticMesh.h"
#include "Modules/ControlInput/Controller.h"

namespace GV
{
    std::unordered_map<std::string, std::vector<MessageTarget>> MessageHandler::s_routes;

    void MessageHandler::Register(const std::string& msg, uint32_t receiverType, uint32_t receiverIndex)
    {
        s_routes[msg].push_back({ receiverType, receiverIndex });
    }

    void MessageHandler::Send(const std::string& msg)
    {
        auto it = s_routes.find(msg);
        if (it == s_routes.end())
            return;

        const auto& targets = it->second;

        for (const auto& t : targets)
        {
            switch (t.type)
            {
                case GV_CHUNK_CAMERA:
                    Camera::HandleMessage(t.index, msg);
                    break;
                case GV_CHUNK_TEXTURE:
                    TexturedQuad::HandleMessage(t.index, msg);
                    break;

                case GV_CHUNK_CONTROLLER:
                   InputController::HandleMessage(t.index, msg);
                    break;

                case GV_CHUNK_STATIC_MESH:
                    StaticMesh::HandleMessage(t.index, msg);
                    break;

                default:
                    break;
            }
        }
    }

    void MessageHandler::Clear()
    {
        s_routes.clear();
    }
}