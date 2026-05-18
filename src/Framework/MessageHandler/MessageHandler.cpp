#include "Framework/MessageHandler/MessageHandler.h"
#include "Framework/Chunk/ChunkTypes.h"

#include "Modules/UI/TexturedQuad.h"
#include "Modules/UI/Crosshair.h"
#include "Modules/UI/ScrollingMap.h"
#include "Modules/Camera/Camera.h"
#include "Modules/StaticMesh/StaticMesh.h"
#include "Modules/ControlInput/Controller.h"
#include "Modules/AreaTriggerBox/AreaTriggerBox.h"
#include "Modules/ZoneTrigger/ZoneTrigger.h"
#include "Modules/Audio/AudioSource.h"
#include "Modules/MessageRelay/MessageRelay.h"
#include "Modules/UI/Button.h"
//#include "Modules/Networking/Network.h"

namespace GV
{
    std::unordered_map<std::string, std::vector<MessageTarget>> MessageHandler::s_routes;

    void MessageHandler::Register(const std::string& msg, uint32_t receiverType, uint32_t receiverIndex)
    {
        s_routes[msg].push_back({ receiverType, receiverIndex });
    }

    void MessageHandler::Send(const std::string& msg,
                              uint32_t senderType,
                              uint32_t senderIndex)
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
                    Camera::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;

                case GV_CHUNK_TEXTURE:
                    TexturedQuad::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;

                case GV_CHUNK_CONTROLLER:
                    InputController::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;

                case GV_CHUNK_STATIC_MESH:
                    StaticMesh::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;

                case GV_CHUNK_AREA_TRIGGER_BOX:
                    AreaTriggerBox::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;

                case GV_CHUNK_AUDIO_SOURCE:
                    AudioSource::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;
                case GV_CHUNK_BUTTON:
                    Button::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;
                case GV_CHUNK_CROSSHAIR:
                    Crosshair::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;
                case GV_CHUNK_ZONE_TRIGGER:
                    ZoneTrigger::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;
                case GV_CHUNK_MESSAGE_RELAY:
                    MessageRelay::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;
                case GV_CHUNK_SCROLLING_MAP:
                    ScrollingMap::HandleMessage(t.index, msg, senderType, senderIndex, nullptr, 0);
                    break;                   
                default:
                    break;
            }
        }
    }

    void MessageHandler::SendWithPayload(const std::string& msg,
                                          uint32_t senderType,
                                          uint32_t senderIndex,
                                          const void* payload,
                                          uint32_t payloadSize,
                                          bool replicate)
    {
        if (replicate)
        {
            //Network::QueuePacket(msg, senderType, senderIndex, payload, payloadSize);
        }

        auto it = s_routes.find(msg);
        if (it == s_routes.end())
            return;

        const auto& targets = it->second;

        for (const auto& t : targets)
        {
            switch (t.type)
            {
                case GV_CHUNK_CAMERA:
                    Camera::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;

                case GV_CHUNK_TEXTURE:
                    TexturedQuad::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;

                case GV_CHUNK_CONTROLLER:
                    InputController::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;

                case GV_CHUNK_STATIC_MESH:
                    StaticMesh::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;

                case GV_CHUNK_AREA_TRIGGER_BOX:
                    AreaTriggerBox::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;

                case GV_CHUNK_AUDIO_SOURCE:
                    AudioSource::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;
                case GV_CHUNK_BUTTON:
                    Button::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;
                case GV_CHUNK_CROSSHAIR:
                    Crosshair::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;
                 case GV_CHUNK_ZONE_TRIGGER:
                    ZoneTrigger::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;               
                 case GV_CHUNK_MESSAGE_RELAY:
                    MessageRelay::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break; 
                 case GV_CHUNK_SCROLLING_MAP:
                    ScrollingMap::HandleMessage(t.index, msg, senderType, senderIndex, payload, payloadSize);
                    break;                                    

                default:
                    break;
            }
        }
    }

    void MessageHandler::DebugSend(const std::string& msg)
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
                Camera::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;

            case GV_CHUNK_TEXTURE:
                TexturedQuad::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;

            case GV_CHUNK_CONTROLLER:
                InputController::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;

            case GV_CHUNK_STATIC_MESH:
                StaticMesh::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;

            case GV_CHUNK_AREA_TRIGGER_BOX:
                AreaTriggerBox::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;

            case GV_CHUNK_AUDIO_SOURCE:
                AudioSource::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;
            case GV_CHUNK_BUTTON:
                Button::HandleMessage(t.index, msg, 0, 0, nullptr, 0);
                break;
            case GV_CHUNK_CROSSHAIR:
                Crosshair::HandleMessage(t.index, msg, 0,0,nullptr,0);
                break;
             case GV_CHUNK_ZONE_TRIGGER:
                ZoneTrigger::HandleMessage(t.index, msg, 0,0,nullptr,0);
                break;               
             case GV_CHUNK_MESSAGE_RELAY:
                MessageRelay::HandleMessage(t.index, msg, 0,0,nullptr,0);
                break;  
             case GV_CHUNK_SCROLLING_MAP:
                ScrollingMap::HandleMessage(t.index, msg, 0,0,nullptr,0);
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