#pragma once

#include <cstdint>
#include "Modules/Player/Attributes/Attributes.h"
#include "Modules/Player/Attributes/Skills.h"



#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(RaceDefinition, GV_CHUNK_RACE)

    UI_SEPARATOR("Identity")

    UI_PARAM_INT(raceID, 0, "Race Enum Value")
    UI_PARAM_STRING(raceName, "", "Race Name")

    UI_SEPARATOR("Meshes")

    UI_PARAM_ASSET(bodyMeshMale, "", "Mesh for Male Body")
    UI_PARAM_ASSET(headMeshMale, "", "Mesh for Male Head")

    UI_PARAM_ASSET(bodyMeshFemale, "", "Mesh for Female Body")
    UI_PARAM_ASSET(headMeshFemale, "", "Mesh for Female Head")

    UI_SEPARATOR("Animations")

    UI_PARAM_STRING(skeleton, "", "Name of Skeleton Used with Mesh")
    UI_PARAM_STRING(animationSet, "", "Name of Animation Set")

    UI_SEPARATOR("Male Attributes")

    UI_PARAM_INT(maleStrength, 0, "Male Strength")
    UI_PARAM_INT(maleIntelligence, 0, "Male Intelligence")
    UI_PARAM_INT(maleWillpower, 0, "Male Willpower")
    UI_PARAM_INT(maleAgility, 0, "Male Agility")
    UI_PARAM_INT(maleSpeed, 0, "Male Speed")
    UI_PARAM_INT(maleEndurance, 0, "Male Endurance")
    UI_PARAM_INT(malePersonality, 0, "Male Personality")
    UI_PARAM_INT(maleLuck, 50, "Male Luck")

    UI_SEPARATOR("Female Attributes")

    UI_PARAM_INT(femaleStrength, 0, "Female Strength")
    UI_PARAM_INT(femaleIntelligence, 0, "Female Intelligence")
    UI_PARAM_INT(femaleWillpower, 0, "Female Willpower")
    UI_PARAM_INT(femaleAgility, 0, "Female Agility")
    UI_PARAM_INT(femaleSpeed, 0, "Female Speed")
    UI_PARAM_INT(femaleEndurance, 0, "Female Endurance")
    UI_PARAM_INT(femalePersonality, 0, "Female Personality")
    UI_PARAM_INT(femaleLuck, 50, "Female Luck")

    UI_SEPARATOR("Combat Skills")

    UI_PARAM_INT(armorer, 0, "Armorer Modifier")
    UI_PARAM_INT(athletics, 0, "Athletics Modifier")
    UI_PARAM_INT(blade, 0, "Blade Modifier")
    UI_PARAM_INT(block, 0, "Block Modifier")
    UI_PARAM_INT(blunt, 0, "Blunt Modifier")
    UI_PARAM_INT(handToHand, 0, "Hand To Hand Modifier")
    UI_PARAM_INT(heavyArmor, 0, "Heavy Armor Modifier")

    UI_SEPARATOR("Magic Skills")

    UI_PARAM_INT(alchemy, 0, "Alchemy Modifier")
    UI_PARAM_INT(alteration, 0, "Alteration Modifier")
    UI_PARAM_INT(conjuration, 0, "Conjuration Modifier")
    UI_PARAM_INT(destruction, 0, "Destruction Modifier")
    UI_PARAM_INT(illusion, 0, "Illusion Modifier")
    UI_PARAM_INT(mysticism, 0, "Mysticism Modifier")
    UI_PARAM_INT(restoration, 0, "Restoration Modifier")

    UI_SEPARATOR("Stealth Skills")

    UI_PARAM_INT(acrobatics, 0, "Acrobatics Modifier")
    UI_PARAM_INT(lightArmor, 0, "Light Armor Modifier")
    UI_PARAM_INT(marksman, 0, "Marksman Modifier")
    UI_PARAM_INT(mercantile, 0, "Mercantile Modifier")
    UI_PARAM_INT(security, 0, "Security Modifier")
    UI_PARAM_INT(sneak, 0, "Sneak Modifier")
    UI_PARAM_INT(speechcraft, 0, "Speechcraft Modifier")

    UI_SEPARATOR("Resistances")

    UI_PARAM_INT(fireResist, 0, "Fire Resistance")
    UI_PARAM_INT(frostResist, 0, "Frost Resistance")
    UI_PARAM_INT(shockResist, 0, "Shock Resistance")
    UI_PARAM_INT(magicResist, 0, "Magic Resistance")
    UI_PARAM_INT(poisonResist, 0, "Poison Resistance")
    UI_PARAM_INT(diseaseResist, 0, "Disease Resistance")

    UI_SEPARATOR("Bonuses")

    UI_PARAM_INT(magickaBonus, 0, "Magicka Bonus")

    UI_SEPARATOR("Physical")

    UI_PARAM_FLOAT(heightMale, 1.0f, "Male Height")
    UI_PARAM_FLOAT(heightFemale, 1.0f, "Female Height")

    UI_PARAM_FLOAT(weightMale, 1.0f, "Male Weight")
    UI_PARAM_FLOAT(weightFemale, 1.0f, "Female Weight")

END_LOGIC_UNIT

#endif

namespace GV
{
    enum Race
    {
        RACE_ARGONIAN = 0,
        RACE_BRETON,
        RACE_DARK_ELF,
        RACE_HIGH_ELF,
        RACE_IMPERIAL,
        RACE_KHAJIIT,
        RACE_NORD,
        RACE_ORC,
        RACE_REDGUARD,
        RACE_WOOD_ELF,

        RACE_CUSTOM,
        RACE_NONE
    };

    struct RaceModifier
    {
        SkillBase skills[21];
        BaseAttributes attributes[8];
        uint32_t raceBonus = 0;

    };
}