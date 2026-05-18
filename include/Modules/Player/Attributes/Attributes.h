#pragma once

#include <cstdint>

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(Attributes, GV_CHUNK_ATTRIBUTES)

    UI_SEPARATOR("Base Attributes")

    UI_PARAM_INT(Agility, 0, "Base Agility")
    UI_PARAM_INT(Endurance, 0, "Base Endurance")
    UI_PARAM_INT(Intelligence, 0, "Base Intelligence")
    UI_PARAM_INT(Personality, 0, "Base Personality")
    UI_PARAM_INT(Speed, 0, "Base Speed")
    UI_PARAM_INT(Strength, 0, "Base Strength")
    UI_PARAM_INT(Willpower, 0, "Base Willpower")

    UI_SEPARATOR("Specialization")

    UI_PARAM_INT(Combat, 0, "Combat Specialization Enum")
    UI_PARAM_INT(Magic, 1, "Magic Specialization Enum")
    UI_PARAM_INT(Stealth, 2, "Stealth Specialization Enum")

END_LOGIC_UNIT

#endif

namespace GV
{
    enum AttributeType 
    {
        

        ATTRIBUTE_AGILITY = 0,
        ATTRIBUTE_ENDURANCE,
        ATTRIBUTE_INTELLIGENCE,
        ATTRIBUTE_PERSONALITY,
        ATTRIBUTE_SPEED,
        ATTRIBUTE_STRENGTH,
        ATTRIBUTE_WILLPOWER,
        ATTRIBUTE_LUCK,

        ATTRIBUTE_COUNT,
        ATTRIBUTE_NULL
    };

    enum SpecializationType 
    {
        SPECIALIZATION_NULL = 0,

        SPECIALIZATION_COMBAT,
        SPECIALIZATION_MAGIC,
        SPECIALIZATION_STEALTH,

        SPECIALIZATION_COUNT
    };

    struct BaseAttributes
    {
        uint32_t attributeID = ATTRIBUTE_NULL;
        uint32_t baseAttributeValue = 0;
        uint32_t classAttributeModifier = 0;
    }
}