#pragma once

#include <cstdint>

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(Skills, GV_CHUNK_SKILLS)

    UI_SEPARATOR("Combat")

    UI_PARAM_INT(Armorer, 5, "Base Armorer")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Athletics, 5, "Base Athletics")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Blade, 5, "Base Blade")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Block, 5, "Base Block")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Blunt, 5, "Base Blunt")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(HandToHand, 5, "Base Hand To Hand")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(HeavyArmor, 5, "Base Heavy Armor")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_SEPARATOR("Magic")

    UI_PARAM_INT(Alchemy, 5, "Base Alchemy")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Alteration, 5, "Base Alteration")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Conjuration, 5, "Base Conjuration")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Destruction, 5, "Base Destruction")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Illusion, 5, "Base Illusion")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Mysticism, 5, "Base Mysticism")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Restoration, 5, "Base Restoration")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_SEPARATOR("Stealth")

    UI_PARAM_INT(Acrobatics, 5, "Base Acrobatics")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(LightArmor, 5, "Base Light Armor")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Marksman, 5, "Base Marksman")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Mercantile, 5, "Base Mercantile")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Security, 5, "Base Security")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Sneak, 5, "Base Sneak")
    UI_PARAM_ASSET(texture, "", "Texture")

    UI_PARAM_INT(Speechcraft, 5, "Base Speechcraft")
    UI_PARAM_ASSET(texture, "", "Texture")

END_LOGIC_UNIT

#endif

namespace GV
{
    enum SkillType
    {
        SKILL_ARMORER = 0,
        SKILL_ATHLETICS,
        SKILL_BLADE,
        SKILL_BLOCK,
        SKILL_BLUNT,
        SKILL_HAND_TO_HAND,
        SKILL_HEAVY_ARMOR,

        SKILL_ALCHEMY,
        SKILL_ALTERATION,
        SKILL_CONJURATION,
        SKILL_DESTRUCTION,
        SKILL_ILLUSION,
        SKILL_MYSTICISM,
        SKILL_RESTORATION,

        SKILL_ACROBATICS,
        SKILL_LIGHT_ARMOR,
        SKILL_MARKSMAN,
        SKILL_MERCANTILE,
        SKILL_SECURITY,
        SKILL_SNEAK,
        SKILL_SPEECHCRAFT,

        SKILL_COUNT,
        SKILL_NULL
    };

    struct SkillBase
    {
        uint32_t skillID = SKILL_NULL;
        uint32_t baseSkillValue = 0;
        uint32_t classModifier = 0;
    };
}