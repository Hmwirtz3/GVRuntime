
#pragma once 
#include "Modules/Player/Attributes/Attributes.h"
#include "Modules/Player/Attributes/Class.h"
#include "Modules/Player/Attributes/Race.h"
#include "Modules/Player/Attributes/Skills.h"

#include <stdint.h>
#include <string>
#include <vector>


#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(PlayerManager, GV_CHUNK_PLAYER_MANAGER)

    UI_SEPARATOR("Identity")

    UI_PARAM_STRING(playerName, "", "Player Name")

    UI_SEPARATOR("Build")

    UI_PARAM_INT(classType, 22, "Default Class Value = None")
    UI_PARAM_INT(raceType, 0, "Default Race Value")
    UI_PARAM_INT(birthSign, 0, "Default Birth Sign")

    UI_PARAM_BOOL(isFemale, false, "Player Gender")

    UI_SEPARATOR("Progression")

    UI_PARAM_INT(level, 1, "Starting Level")
    UI_PARAM_INT(experience, 0, "Current Experience")

    UI_SEPARATOR("Vitals")

    UI_PARAM_FLOAT(baseHealth, 100.0f, "Base Health")
    UI_PARAM_FLOAT(baseMagicka, 100.0f, "Base Magicka")
    UI_PARAM_FLOAT(baseFatigue, 100.0f, "Base Fatigue")

    UI_SEPARATOR("Spawn")

    UI_PARAM_FLOAT(startPosX, 0.0f, "Start Position X")
    UI_PARAM_FLOAT(startPosY, 0.0f, "Start Position Y")
    UI_PARAM_FLOAT(startPosZ, 0.0f, "Start Position Z")

END_LOGIC_UNIT

#endif

namespace GV
{




    class Player
    {

        public:
        static void Load(const std::vector<uint8_t>& bytes, uint32_t start, uint32_t end);
        static void HandleMessage(uint32_t index,
                                    const std::string senderType, 
                                    uint32_t senderIndex, 
                                    const void* payload, 
                                    uint32_t payloadSize);

        

    
        private:
        std::string playerName;
        uint32_t classType = CLASS_NONE;
        uint32_t playerRace = RACE_NONE;
        uint32_t birthsign = 0;

        bool isFemale = false;

        uint32_t playerLevel = 1;
        uint32_t playerExperience = 0;

        uint32_t playerHealth = 100;
        uint32_t playerMagicka = 100;
        uint32_t playerStamina = 100;

        float playerPosX = 0.0f;
        float playerPosY = 0.0f;
        float playerPosZ = 0.0f;

        CharacterClass playerClass;
        RaceModifier playerRace;

        PlayerSkills playerSkills;

        PlayerAttributes playerAttributes;

       
    };










} 