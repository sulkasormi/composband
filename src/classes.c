/*
 *  Hooks and Callbacks for various classes
 */

#include "angband.h"

#include <assert.h>

bool class_is_deprecated(int i)
{
    return (i == CLASS_XXX12);
}

int lookup_class_idx(cptr name)
{
    int i;
    for (i = 0; i < MAX_CLASS; i++)
    {
        if (class_is_deprecated(i)) continue;
        if (strcmp(name, get_class_aux(i, 0)->name) == 0)
            return i;
    }
    return -1;
}

/* In general, the class index is given by player_type.pclass. However, for various
 * monster races, the "class" behavior is actually determined by the race, and we
 * encode this desire with race_t.pseudo_class_idx. As another twist, consider the
 * possessor. Here, the "class" behavior depends on the current body (see r_info.text).
 */
int get_class_idx(void)
{
    int result = p_ptr->pclass;

    if (result == CLASS_MONSTER)
    {
        switch (p_ptr->prace)
        {
        case RACE_MON_POSSESSOR:
        case RACE_MON_MIMIC:
            if (p_ptr->current_r_idx)
            {
                result = r_info[p_ptr->current_r_idx].body.class_idx;
                break;
            }
            /* vvv Fall Through vvv */
        default:
        {
            race_t *race_ptr = get_race();
            /*if (race_ptr->pseudo_class_idx) Note: CLASS_WARRIOR = 0! */
                result = race_ptr->pseudo_class_idx;
        }
        }
    }
    return result;
}

/* Goal: This should be the one and only switch off of p_ptr->pclass in the
   entire system! */
class_t *get_class_aux(int pclass, int psubclass)
{
class_t *result = NULL;

    switch (pclass)
    {
    case CLASS_ALCHEMIST:
        result = alchemist_get_class();
        break;
    case CLASS_ARCHAEOLOGIST:
        result = archaeologist_get_class();
        break;
    case CLASS_ARCHER:
        result = archer_get_class();
        break;
    case CLASS_BARD:
        result = bard_get_class();
        break;
    case CLASS_BEASTMASTER:
        result = beastmaster_get_class();
        break;
    case CLASS_BERSERKER:
        result = berserker_get_class();
        break;
    case CLASS_BLOOD_KNIGHT:
        result = blood_knight_get_class();
        break;
    case CLASS_BLOOD_MAGE:
        result = blood_mage_get_class();
        break;
    case CLASS_BLUE_MAGE:
        result = blue_mage_get_class();
        break;
    case CLASS_CAVALRY:
        result = cavalry_get_class();
        break;
    case CLASS_CHAOS_WARRIOR:
        result = chaos_warrior_get_class();
        break;
    case CLASS_DEVICEMASTER:
        result = devicemaster_get_class(psubclass);
        break;
    case CLASS_DISCIPLE:
        result = disciple_get_class(psubclass);
        break;
    case CLASS_DUELIST:
        result = duelist_get_class();
        break;
    case CLASS_FORCETRAINER:
        result = force_trainer_get_class();
        break;
    case CLASS_GRAY_MAGE:
        result = gray_mage_get_class(psubclass);
        break;
    case CLASS_HIGH_MAGE:
        result = high_mage_get_class();
        break;
    case CLASS_LAWYER:
        result = lawyer_get_class();
        break;
    case CLASS_MAGE:
        result = mage_get_class();
        break;
    case CLASS_MAGIC_EATER:
        result = magic_eater_get_class();
        break;
    case CLASS_MAULER:
        result = mauler_get_class();
        break;
    case CLASS_MINDCRAFTER:
        result = mindcrafter_get_class();
        break;
    case CLASS_MIRROR_MASTER:
        result = mirror_master_get_class();
        break;
    case CLASS_MONK:
        result = monk_get_class();
        break;
    case CLASS_MONSTER:
        result = monster_get_class();
        break;
    case CLASS_MYSTIC:
        result = mystic_get_class();
        break;
    case CLASS_NECROMANCER:
        result = necromancer_get_class();
        break;
    case CLASS_NINJA:
        result = ninja_get_class();
        break;
    case CLASS_NINJA_LAWYER:
        result = ninja_lawyer_get_class();
        break;
    case CLASS_PALADIN:
        result = paladin_get_class();
        break;
    case CLASS_POLITICIAN:
        result = politician_get_class();
        break;
    case CLASS_PRIEST:
        result = priest_get_class();
        break;
    case CLASS_PSION:
        result = psion_get_class();
        break;
    case CLASS_RANGER:
        result = ranger_get_class();
        break;
    case CLASS_RAGE_MAGE:
        result = rage_mage_get_class();
        break;
    case CLASS_RED_MAGE:
        result = red_mage_get_class();
        break;
    case CLASS_ROGUE:
        result = rogue_get_class();
        break;
    case CLASS_RUNE_KNIGHT:
        result = rune_knight_get_class();
        break;
    case CLASS_SAMURAI:
        result = samurai_get_class();
        break;
    case CLASS_SCOUT:
        result = scout_get_class();
        break;
    case CLASS_SKILLMASTER:
        result = skillmaster_get_class();
        break;
    case CLASS_SNIPER:
        result = sniper_get_class();
        break;
    case CLASS_SORCERER:
        result = sorcerer_get_class();
        break;
    case CLASS_TIME_LORD:
        result = time_lord_get_class();
        break;
    case CLASS_TOURIST:
        result = tourist_get_class();
        break;
    case CLASS_WARLOCK:
        result = warlock_get_class(psubclass);
        break;
    case CLASS_WARRIOR:
        result = warrior_get_class();
        break;
    case CLASS_WARRIOR_MAGE:
        result = warrior_mage_get_class();
        break;
    case CLASS_WEAPONSMITH:
        result = weaponsmith_get_class();
        break;
    case CLASS_WEAPONMASTER:
        result = weaponmaster_get_class(psubclass);
        break;
    case CLASS_WILD_TALENT:
        result = wild_talent_get_class();
        break;
    case CLASS_YELLOW_MAGE:
        result = yellow_mage_get_class();
        break;
    }

    assert(result);
    result->id = pclass;
    result->subid = psubclass;
    return result;
}

class_t *get_class(void)
{
    return get_class_aux(p_ptr->pclass, p_ptr->psubclass);
}

caster_info *get_caster_info(void)
{
    caster_info *result = NULL;
    class_t *class_ptr = get_class();
    race_t *race_ptr = get_race();

    if (race_ptr->caster_info) /* Monster Races: Lich, Angel, Demon */
        result = (race_ptr->caster_info)();
    else if (class_ptr->caster_info)
        result = (class_ptr->caster_info)();
    return result;
}

int get_spell_stat(void)
{
    int          result = A_NONE;
    caster_info *caster_ptr = get_caster_info();

    if (caster_ptr)
        result = caster_ptr->which_stat;

    return result;
}

