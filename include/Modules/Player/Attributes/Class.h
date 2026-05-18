#pragma once

#include <cstdint>
#include "Modules/Player/Attributes/Skills.h"
#include "Modules/Player/Attributes/Attributes.h"

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(ClassDefinition, GV_CHUNK_CLASS)

    UI_SEPARATOR("Identity")

    UI_PARAM_INT(classID, 0, "Class Enum Value")
    UI_PARAM_STRING(className, "", "Class Name")

    UI_SEPARATOR("UI")
    UI_PARAM_ASSET(texture, "", "Path to Class Texture")

    UI_SEPARATOR("Specialization")

    UI_PARAM_INT(specialization, 0, "Combat Magic Stealth")

    UI_SEPARATOR("Favored Attributes")

    UI_PARAM_INT(favoredAttribute0, 0, "Favored Attribute")
    UI_PARAM_INT(favoredAttribute1, 0, "Favored Attribute")

    UI_SEPARATOR("Combat Skills")

    UI_PARAM_INT(armorer, 0, "Armorer Modifier")
    UI_PARAM_BOOL(armorerMajor, false, "Armorer Major Skill")

    UI_PARAM_INT(athletics, 0, "Athletics Modifier")
    UI_PARAM_BOOL(athleticsMajor, false, "Athletics Major Skill")

    UI_PARAM_INT(blade, 0, "Blade Modifier")
    UI_PARAM_BOOL(bladeMajor, false, "Blade Major Skill")

    UI_PARAM_INT(block, 0, "Block Modifier")
    UI_PARAM_BOOL(blockMajor, false, "Block Major Skill")

    UI_PARAM_INT(blunt, 0, "Blunt Modifier")
    UI_PARAM_BOOL(bluntMajor, false, "Blunt Major Skill")

    UI_PARAM_INT(handToHand, 0, "Hand To Hand Modifier")
    UI_PARAM_BOOL(handToHandMajor, false, "Hand To Hand Major Skill")

    UI_PARAM_INT(heavyArmor, 0, "Heavy Armor Modifier")
    UI_PARAM_BOOL(heavyArmorMajor, false, "Heavy Armor Major Skill")

    UI_SEPARATOR("Magic Skills")

    UI_PARAM_INT(alchemy, 0, "Alchemy Modifier")
    UI_PARAM_BOOL(alchemyMajor, false, "Alchemy Major Skill")

    UI_PARAM_INT(alteration, 0, "Alteration Modifier")
    UI_PARAM_BOOL(alterationMajor, false, "Alteration Major Skill")

    UI_PARAM_INT(conjuration, 0, "Conjuration Modifier")
    UI_PARAM_BOOL(conjurationMajor, false, "Conjuration Major Skill")

    UI_PARAM_INT(destruction, 0, "Destruction Modifier")
    UI_PARAM_BOOL(destructionMajor, false, "Destruction Major Skill")

    UI_PARAM_INT(illusion, 0, "Illusion Modifier")
    UI_PARAM_BOOL(illusionMajor, false, "Illusion Major Skill")

    UI_PARAM_INT(mysticism, 0, "Mysticism Modifier")
    UI_PARAM_BOOL(mysticismMajor, false, "Mysticism Major Skill")

    UI_PARAM_INT(restoration, 0, "Restoration Modifier")
    UI_PARAM_BOOL(restorationMajor, false, "Restoration Major Skill")

    UI_SEPARATOR("Stealth Skills")

    UI_PARAM_INT(acrobatics, 0, "Acrobatics Modifier")
    UI_PARAM_BOOL(acrobaticsMajor, false, "Acrobatics Major Skill")

    UI_PARAM_INT(lightArmor, 0, "Light Armor Modifier")
    UI_PARAM_BOOL(lightArmorMajor, false, "Light Armor Major Skill")

    UI_PARAM_INT(marksman, 0, "Marksman Modifier")
    UI_PARAM_BOOL(marksmanMajor, false, "Marksman Major Skill")

    UI_PARAM_INT(mercantile, 0, "Mercantile Modifier")
    UI_PARAM_BOOL(mercantileMajor, false, "Mercantile Major Skill")

    UI_PARAM_INT(security, 0, "Security Modifier")
    UI_PARAM_BOOL(securityMajor, false, "Security Major Skill")

    UI_PARAM_INT(sneak, 0, "Sneak Modifier")
    UI_PARAM_BOOL(sneakMajor, false, "Sneak Major Skill")

    UI_PARAM_INT(speechcraft, 0, "Speechcraft Modifier")
    UI_PARAM_BOOL(speechcraftMajor, false, "Speechcraft Major Skill")

END_LOGIC_UNIT

#endif

namespace GV
{
    enum ClassType
    {
        CLASS_ACROBAT = 0,
        CLASS_AGENT,
        CLASS_ARCHER,
        CLASS_ASSASSIN,
        CLASS_BARBARIAN,
        CLASS_BARD,
        CLASS_BATTLEMAGE,
        CLASS_CRUSADER,
        CLASS_HEALER,
        CLASS_KNIGHT,
        CLASS_MAGE,
        CLASS_MONK,
        CLASS_NIGHTBLADE,
        CLASS_PILGRIM,
        CLASS_ROGUE,
        CLASS_SCOUT,
        CLASS_SORCERER,
        CLASS_SPELLSWORD,
        CLASS_THIEF,
        CLASS_WARRIOR,
        CLASS_WITCHHUNTER,

        CLASS_CUSTOM,
        CLASS_NONE
    };

    struct PlayerSkills
    {
        SkillBase MajorSkills[7];
        SkillBase MinorSkills[14];
    };

    struct PlayerAttributes
    {
        BaseAttributes baseAttributes[6];
        BaseAttributes classAttributes[2];
    };


    class CharacterClass
    {
        public:
        static void GetClass();
        static void GetSkills();
        static void GetAttributes();
        static void GetBirthSign();

        private:
        

    };
}