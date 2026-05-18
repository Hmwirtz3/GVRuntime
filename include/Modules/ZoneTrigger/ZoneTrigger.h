#pragma once 

#include <string>
#include "Framework/ScenesHandler/SceneLoader.h"

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(ZoneTrigger, "GV_CHUNK_ZONE_TRIGGER")

	UI_PARAM_STRING(sceneFile, "", "Scene bin file name")

	UI_SEPARATOR("Message Listener")

	UI_MESSAGE(loadZone, "", "Message recieved to load zone")

END_LOGIC_UNIT

#endif


namespace GV
{

    struct ZoneTriggerData
    {
        std::string sceneFile;
        std::string loadMessage;
        SceneFile scene;

    };


    class ZoneTrigger
    {
        public:

        static void Load(const std::vector<uint8_t>& bytes,uint32_t start,uint32_t end);

        static void LoadZone();

            static void HandleMessage(uint32_t index,
                                  const std::string& msg,
                                  uint32_t senderType,
                                  uint32_t senderIndex,
                                  const void* payload,
                                  uint32_t payloadSize);

        

    };

    



}