/****************************************************************
 * The Rune Knight
 ****************************************************************/

#include "angband.h"

/****************************************************************
 * Public Helpers
 ****************************************************************/
int rune_knight_absorption(int m_idx, int type, int dam)
{
    int drain = 0;

    if (p_ptr->pclass != CLASS_RUNE_KNIGHT) return dam;
    if (!p_ptr->magic_resistance) return dam;
    if (type == GF_ARROW || type == GF_ROCK) return dam;

    drain = dam * p_ptr->magic_resistance / 100;
    /* XXX Decline mana gain if player is scumming weak casters? */
    sp_player(MAX(drain, 2 + p_ptr->lev/10));

    return dam - drain;
}

void rune_calc_bonuses(object_type *o_ptr)
{
    if (o_ptr->rune == RUNE_ABSORPTION)
        p_ptr->magic_resistance += 15;
    if (o_ptr->rune == RUNE_UNDERSTANDING && object_is_helmet(o_ptr))
        p_ptr->auto_id = TRUE;
    if (o_ptr->rune == RUNE_SHADOW)
    {
        if (object_is_body_armour(o_ptr) || o_ptr->tval == TV_CLOAK)
            p_ptr->skills.stl += 5 * p_ptr->lev / 50;
    }
    if (o_ptr->rune == RUNE_HASTE)
    {
        if (o_ptr->tval == TV_BOOTS)
            p_ptr->pspeed += 3 * p_ptr->lev / 50;
    }
}

void rune_calc_stats(object_type *o_ptr, s16b stats[MAX_STATS])
{
    if (o_ptr->rune == RUNE_UNDERSTANDING)
    {
        if (o_ptr->tval == TV_LITE)
            stats[A_INT] += 1;
        else
            stats[A_INT] += 2;
    }
    if (o_ptr->rune == RUNE_HASTE)
    {
        if (o_ptr->tval == TV_GLOVES)
            stats[A_DEX] += 2;
    }
    if (o_ptr->rune == RUNE_LIFE)
    {
        if (object_is_body_armour(o_ptr))
            stats[A_CON] += 1;
    }
    if (o_ptr->rune == RUNE_MIND)
    {
        if (object_is_helmet(o_ptr))
            stats[A_INT] += 2;
    }
    if (o_ptr->rune == RUNE_MIGHT)
    {
        stats[A_STR] += 2;
        stats[A_CON] += 2;
        if (object_is_body_armour(o_ptr))
            stats[A_DEX] += 2;
    }
}

cptr rune_desc(int which)
{
    switch (which)
    {
    case RUNE_ABSORPTION:
        return "{Absorption}";
    case RUNE_PROTECTION:
        return "{Protection}";
    case RUNE_REGENERATION:
        return "{Regeneration}";
    case RUNE_FIRE:
        return "{Fire}";
    case RUNE_AIR:
        return "{Air}";
    case RUNE_WATER:
        return "{Water}";
    case RUNE_LIGHT:
        return "{Light}";
    case RUNE_SHADOW:
        return "{Shadow}";
    case RUNE_EARTH:
        return "{Earth}";
    case RUNE_UNDERSTANDING:
        return "{Understanding}";
    case RUNE_ELEMENTAL_PROTECTION:
        return "{Preservation}";
    case RUNE_HASTE:
        return "{Haste}";
    case RUNE_SEEING:
        return "{Seeing}";
    case RUNE_SACRIFICE:
        return "{Sacrifice}";
    case RUNE_LIFE:
        return "{Life}";
    case RUNE_STABILITY:
        return "{Stability}";
    case RUNE_REFLECTION:
        return "{Reflection}";
    case RUNE_DEATH:
        return "{Death}";
    case RUNE_MIND:
        return "{Mind}";
    case RUNE_MIGHT:
        return "{Might}";
    case RUNE_DESTRUCTION:
        return "{Destruction}";
    case RUNE_GOOD_FORTUNE:
        return "{Luck}";
    case RUNE_IMMORTALITY:
        return "{Immortality}";
    }
    return "{Unknown}";
}

void _add_flag(obj_ptr obj, int which)
{
    add_flag(obj->flags, which);
    add_flag(obj->known_flags, which);
}

bool rune_add(object_type *o_ptr, int which, bool prompt)    /* Birthing needs access to this ... */
{
    char o_name[MAX_NLEN];

    if (!which) return FALSE;
    object_desc(o_name, o_ptr, 0);

    if (o_ptr->rune)
    {
        msg_format("%^s already has an attached rune.", o_name);
        return FALSE;
    }

    if (object_is_(o_ptr, TV_SWORD, SV_RUNESWORD))
    {
        msg_print("Failed! Rune Swords already have runes, albeit ones you fail to comprehend.");
        return FALSE;
    }

    if (o_ptr->number > 1)
    {
        msg_print("Failed! You may only add a rune to a single object at a time.");
        return FALSE;
    }

    if (prompt)
    {
        if (!get_check(
                format("Really add %^s to %s?",
                    rune_desc(which), o_name))) return FALSE;
    }

    o_ptr->rune = which;
    if (object_is_nameless(o_ptr))
        o_ptr->discount = 99;

    /* Note: Any effect that requires a pval will need to be handled
       silently in calc_bonuses(). This is because we keep the pval
       of the original object (e.g. Crown of Might (+3) <<Might>>
       Gives +5 Str/Con and +3 Dex, where the Rune adds +2 Str/Con) */
    switch (which)
    {
    case RUNE_PROTECTION:
        _add_flag(o_ptr, OF_IGNORE_ACID);
        o_ptr->to_a += 2 + randint1(8);
        break;

    case RUNE_REGENERATION:
        _add_flag(o_ptr, OF_REGEN);
        break;

    case RUNE_FIRE:
        if (object_is_melee_weapon(o_ptr) || o_ptr->tval == TV_GLOVES)
            _add_flag(o_ptr, OF_BRAND_FIRE);
        if (object_is_shield(o_ptr))
            _add_flag(o_ptr, OF_RES_FIRE);
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_RES_FIRE);
            _add_flag(o_ptr, OF_AURA_FIRE);
        }
        if (o_ptr->tval == TV_LITE || o_ptr->tval == TV_CLOAK)
            _add_flag(o_ptr, OF_AURA_FIRE);
        break;

    case RUNE_AIR:
        if (!object_is_melee_weapon(o_ptr))
            _add_flag(o_ptr, OF_LEVITATION);
        break;

    case RUNE_WATER:
        _add_flag(o_ptr, OF_IGNORE_ACID);
        if (object_is_melee_weapon(o_ptr) || o_ptr->tval == TV_GLOVES)
            _add_flag(o_ptr, OF_BRAND_ACID);
        else
            _add_flag(o_ptr, OF_RES_ACID);
        break;

    case RUNE_LIGHT:
        _add_flag(o_ptr, OF_RES_LITE);
        break;

    case RUNE_SHADOW:
        if (o_ptr->tval != TV_CLOAK)
            _add_flag(o_ptr, OF_RES_DARK);
        break;

    case RUNE_EARTH:
        if (object_is_melee_weapon(o_ptr))
            _add_flag(o_ptr, OF_VORPAL);
        else if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_RES_SHARDS);
            _add_flag(o_ptr, OF_AURA_SHARDS);
        }
        else if (object_is_shield(o_ptr))
            _add_flag(o_ptr, OF_RES_SHARDS);
        else if (o_ptr->tval == TV_CLOAK)
            _add_flag(o_ptr, OF_AURA_SHARDS);
        break;

    case RUNE_SEEING:
        _add_flag(o_ptr, OF_RES_BLIND);
        if (object_is_helmet(o_ptr))
            _add_flag(o_ptr, OF_SEE_INVIS);
        break;

    case RUNE_LIFE:
        _add_flag(o_ptr, OF_HOLD_LIFE);
        break;

    case RUNE_STABILITY:
        _add_flag(o_ptr, OF_RES_NEXUS);
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_RES_CHAOS);
            _add_flag(o_ptr, OF_RES_DISEN);
        }
        break;
    
    case RUNE_REFLECTION:
        _add_flag(o_ptr, OF_REFLECT);
        break;

    case RUNE_DEATH:
        if (object_is_melee_weapon(o_ptr))
            _add_flag(o_ptr, OF_BRAND_VAMP);
        else
        {
            _add_flag(o_ptr, OF_RES_NETHER);
            if (object_is_body_armour(o_ptr))
                _add_flag(o_ptr, OF_RES_POIS);
        }
        break;

    case RUNE_MIND:
        _add_flag(o_ptr, OF_TELEPATHY);
        if (object_is_helmet(o_ptr))
            _add_flag(o_ptr, OF_SUST_INT);
        break;

    case RUNE_MIGHT:
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_SUST_STR);
            _add_flag(o_ptr, OF_SUST_DEX);
            _add_flag(o_ptr, OF_SUST_CON);
        }
        break;

    case RUNE_DESTRUCTION:
        if (object_is_melee_weapon(o_ptr))
            o_ptr->dd += 2;
        else
        {
            o_ptr->to_h += 3 + randint1(8);
            o_ptr->to_d += 3 + randint1(8);
        }
        break;

    case RUNE_IMMORTALITY:
        _add_flag(o_ptr, OF_RES_TIME);
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_SUST_STR);
            _add_flag(o_ptr, OF_SUST_INT);
            _add_flag(o_ptr, OF_SUST_WIS);
            _add_flag(o_ptr, OF_SUST_DEX);
            _add_flag(o_ptr, OF_SUST_CON);
            _add_flag(o_ptr, OF_SUST_CHR);
            _add_flag(o_ptr, OF_HOLD_LIFE);
        }
        break;

    case RUNE_ELEMENTAL_PROTECTION:
    case RUNE_GOOD_FORTUNE:
        _add_flag(o_ptr, OF_IGNORE_ACID);
        _add_flag(o_ptr, OF_IGNORE_FIRE);
        _add_flag(o_ptr, OF_IGNORE_COLD);
        _add_flag(o_ptr, OF_IGNORE_ELEC);
        break;
    }

    if (prompt)
        msg_format("%^s gleams.", o_name);
    p_ptr->update |= PU_BONUS;

    return TRUE;
}

/****************************************************************
 * Runes of Creation
 ****************************************************************/
static object_type *_rune_object_prompt(obj_p filter)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Enchant which item?";
    prompt.error = "You have nothing to enchant.";
    prompt.filter = filter;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_FLOOR;

    obj_prompt(&prompt);
    return prompt.obj;
}

static void _rune_default_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(25, p_ptr->msp));
        break;
    /*case SPELL_COLOR:
        var_set_int(res, TERM_L_BLUE);
        break; */
    default:
        default_spell(cmd, res);
    }
}

static bool _obj_absorption_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr) )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_absorption_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Absorption");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_absorption");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Absorption on chosen melee weapon, body armor or shield. This rune grants a special magical defense that absorbs damage from all monster spells, restoring your mana in the process.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_absorption_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_ABSORPTION, TRUE));

        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, 0); /* was MAX(1, p_ptr->msp * 3 / 10)*/
        break;
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_protection_pred(object_type *o_ptr)
{
    if (object_is_armour(o_ptr))
        return TRUE;
    return FALSE;
}
static void _obj_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Protection");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_protection");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Protection on chosen armor, making it immune to acid while slightly enhancing its protective capabilities.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_protection_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_PROTECTION, TRUE));
        
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(5, p_ptr->msp * 5 / 10));
        break;
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_regeneration_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_regeneration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Regeneration");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_regeneration");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Regeneration on chosen item, granting powers of regeneration.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_regeneration_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_REGENERATION, TRUE));
        
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(10, p_ptr->msp * 5 / 10));
        break;
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_fire_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr) 
      || o_ptr->tval == TV_CLOAK
      || (o_ptr->tval == TV_GLOVES && p_ptr->lev >= 45)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Fire on chosen item, granting special fire-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_fire_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_FIRE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_air_pred(object_type *o_ptr)
{
    if ( (object_is_melee_weapon(o_ptr) && p_ptr->lev >= 40)
      || o_ptr->tval == TV_CLOAK
      || o_ptr->tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_air_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Air");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_air");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Air on chosen item, granting special wind-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_air_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_AIR, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_water_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr) 
      || o_ptr->tval == TV_CLOAK
      || (o_ptr->tval == TV_GLOVES && p_ptr->lev >= 45) )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_water_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_water");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Water on chosen item, granting special acid-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_water_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_WATER, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_light_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Light");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_light");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Light on chosen item, granting special light-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_light_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_LIGHT, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_shadow_pred(object_type *o_ptr)
{
    if ( object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr)
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_shadow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shadow");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_shadow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Shadow on chosen item, granting special darkness-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_shadow_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_SHADOW, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_earth_pred(object_type *o_ptr)
{
    if ( (object_is_melee_weapon(o_ptr) && p_ptr->lev >= 35)
      || object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earth");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Earth on chosen item, granting special shard-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_earth_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_EARTH, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_understanding_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_understanding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Understanding");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_understanding");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Understanding on chosen item, granting special knowledge-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_understanding_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_UNDERSTANDING, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static void _obj_elemental_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Preservation");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_preservation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates a standalone rune. As long as you have this rune in your inventory, your inventory items are less likely to be destroyed by elemental attacks.");
        break;
    case SPELL_CAST:
    {
        object_type forge;

        object_prep(&forge, lookup_kind(TV_RUNE, SV_RUNE));
        rune_add(&forge, RUNE_ELEMENTAL_PROTECTION, FALSE);
        drop_near(&forge, -1, py, px);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_haste_pred(object_type *o_ptr)
{
    if ( o_ptr->tval == TV_GLOVES
      || o_ptr->tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_haste_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Haste");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_haste");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Haste on chosen item, granting special speed-based powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_haste_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_HASTE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_seeing_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_seeing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Seeing");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_seeing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Seeing on chosen item, granting powers of sight.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_seeing_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_SEEING, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

/* XXX see comments in the spell table below ...
static void _obj_sacrifice_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sacrifice");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_sacrifice");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Sacrifice on an artifact. You can now destroy (with 'k' command) the artifact, and if you do so, you restore HP and SP.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(object_is_artifact);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_SACRIFICE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}
*/

static bool _obj_life_pred(object_type *o_ptr)
{
    if ( object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Life");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_life");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Life on chosen item, protecting your living essence.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_life_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_LIFE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_stability_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr)
      || o_ptr->tval == TV_CLOAK
      || o_ptr->tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_stability_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stability");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_stability");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Stability on chosen item, protecting you from the vicissitudes of the world around you.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_stability_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_STABILITY, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static void _obj_reflection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Reflection");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_reflection");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Reflection on your shield.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(object_is_shield);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_REFLECTION, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_death_pred(object_type *o_ptr)
{
    if ( object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr) )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_death_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Death");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_death");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Death on chosen item, granting powers of the nether world.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_death_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_DEATH, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_mind_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr) 
      || o_ptr->tval == TV_LITE ) 
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_mind_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mind");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_mind");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Mind on chosen item, granting powers of thought and awareness.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_mind_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_MIND, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_might_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr) 
      || object_is_helmet(o_ptr) ) 
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_might_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Might");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_might");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Might on chosen item, granting powers of strength and fortitude.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_might_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_MIGHT, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_destruction_pred(object_type *o_ptr)
{
    if ( object_is_melee_weapon(o_ptr) 
      || o_ptr->tval == TV_GLOVES ) 
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_destruction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Destruction");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_destruction");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Destruction on chosen item, granting great combat powers.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_destruction_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_DESTRUCTION, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static void _obj_good_fortune_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Luck");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_luck");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates a standalone rune. As long as you have this rune in your inventory, you will experience better luck during your adventures.");
        break;
    case SPELL_CAST:
    {
        object_type forge;

        object_prep(&forge, lookup_kind(TV_RUNE, SV_RUNE));
        rune_add(&forge, RUNE_GOOD_FORTUNE, FALSE);
        drop_near(&forge, -1, py, px);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_immortality_pred(object_type *o_ptr)
{
    if ( object_is_shield(o_ptr) 
      || object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr) 
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_immortality_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Immortality");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_immortality");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Immortality on chosen item, granting power over time itself.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_immortality_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_IMMORTALITY, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

/****************************************************************
 * Runes of Enhancement
 *
 * Note: Durations are long since mana is often scarce. Feel free
 * to tweak upwards as playtesting dictates.
 ****************************************************************/
#define _DURATION 100

void _self_darkness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Darkness");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_darkness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Place a temporary Rune of Darkness on your person, granting enhanced stealth.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_tim_dark_stalker(_DURATION + randint1(_DURATION), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_seeing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Seeing");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_seeing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Place a temporary Rune of Seeing on your person, granting telepathic powers.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_tim_esp(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_understanding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Understanding");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_understanding");
        break;
    case SPELL_DESC:
        var_set_string(res, "Place a temporary Rune of Understanding on your person, granting knowledge of yourself.");
        break;
    default:
        self_knowledge_spell(cmd, res);
    }
}

void _self_haste_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Haste");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_haste");
        break;
    case SPELL_DESC:
        var_set_string(res, "Place a temporary Rune of Haste on your person, granting enhanced speed of motion.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_fast(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Protection");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_protection");
        break;
    case SPELL_DESC:
        var_set_string(res, "Place a temporary Rune of Protection on your person, granting temporary resistance to the basic elements.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_oppose_acid(randint1(_DURATION) + _DURATION, FALSE);
        set_oppose_elec(randint1(_DURATION) + _DURATION, FALSE);
        set_oppose_fire(randint1(_DURATION) + _DURATION, FALSE);
        set_oppose_cold(randint1(_DURATION) + _DURATION, FALSE);
        set_oppose_pois(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earth");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "Place a temporary Rune of Earth on your person, hardening your skin to ward off enemy blows.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_shield(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Life");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_life");
        break;
    case SPELL_DESC:
        var_set_string(res, "By placing a temporary Rune of Life on your person, you may recover that which was lost.");
        break;
    default:
        restore_life_spell(cmd, res);
    }
}

void _self_daemon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Daemon");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_daemon");
        break;
    case SPELL_DESC:
        var_set_string(res, "By placing a temporary Demonic Rune on your person you transform yourself into a more powerful form.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_mimic(_DURATION + randint1(_DURATION), MIMIC_DEMON, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_might_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Might");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_might");
        break;
    case SPELL_DESC:
        var_set_string(res, "By placing a temporary Rune of Might on your person you gain the strength of giants.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION/2, _DURATION/2));
        break;
    case SPELL_CAST:
        set_tim_building_up(_DURATION/2 + randint1(_DURATION/2), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

/****************************************************************
 * Runes of Alteration
 ****************************************************************/
void _feat_spell(int feat, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST: {
        int        dir, x, y;
        cave_type *c_ptr;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        if (!in_bounds(y, x)) return;
        c_ptr = &cave[y][x];
        if ((c_ptr->info & CAVE_OBJECT) || c_ptr->o_idx)
            msg_print("<color:r>The object resists your rune.</color>");
        else if (c_ptr->m_idx)
            msg_print("<color:r>There is a monster in your way.</color>");
        else
            cave_set_feat(y, x, feat);

        var_set_bool(res, TRUE);
        break; }
    default:
        default_spell(cmd, res);
    }
}

void _feat_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Light");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_light");
        break;
    case SPELL_DESC:
        var_set_string(res, "By placing a Rune of Light at your current location you may light up your surroundings.");
        break;
    default:
        light_area_spell(cmd, res);
    }
}

void _feat_water_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_water");
        break;
    case SPELL_DESC:
        var_set_string(res, "This rune washes away all in its path.");
        break;
    default:
        _feat_spell(p_ptr->lev < 35 ? feat_shallow_water : feat_deep_water, cmd, res);
    }
}

void _feat_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earth");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "This rune blocks the passage of your foes.");
        break;
    default:
        _feat_spell(p_ptr->lev < 45 ? feat_rubble : feat_granite, cmd, res);
    }
}

void _feat_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "This rune burns the surrounding landscape.");
        break;
    default:
        _feat_spell(p_ptr->lev < 40 ? feat_shallow_lava : feat_deep_lava, cmd, res);
    }
}

void _feat_air_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Air");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_air");
        break;
    case SPELL_DESC:
        var_set_string(res, "This rune summons air where once there was none.");
        break;
    default:
        _feat_spell(feat_dark_pit, cmd, res);
    }
}

void _feat_stability_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stability");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_stability");
        break;
    case SPELL_DESC:
        var_set_string(res, "This rune creates solid footing of the most ordinary sort.");
        break;
    default:
        _feat_spell(feat_floor, cmd, res);
    }
}

void _feat_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Life");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_life");
        break;
    case SPELL_DESC:
        var_set_string(res, "This rune causes rapid plant growth wherever it is placed.");
        break;
    default:
        _feat_spell(feat_tree, cmd, res);
    }
}

void _feat_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Protection");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_protection");
        break;
    case SPELL_DESC:
        var_set_string(res, "By placing a Rune of Protection at your current location, you may block the passage of all save the mightiest of enemies.");
        break;
    default:
        glyph_of_warding_spell(cmd, res);
    }
}

void _feat_destruction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Destruction");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_destruction");
        break;
    case SPELL_DESC:
        var_set_string(res, "By placing a highly unstable Rune of Destruction at your current location you may destroy your nearby surroundings.");
        break;
    case SPELL_CAST:
        destroy_area(py, px, 12 + randint1(4), spell_power(8 * p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

/****************************************************************
 * Runes of Battle
 ****************************************************************/
void _blow_confusion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confusion");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_confusion");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune enhances your melee attacks to baffle your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_CONF));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune enhances your melee attacks to burn your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_FIRE));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_water_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_water");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune enhances your melee attacks to corrode your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_ACID));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earth");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune enhances your melee attacks to cut your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_VORPAL));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_death_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Death");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_death");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune enhances your melee attacks to drain life from your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_VAMP));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_elec_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_lightning");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune enhances your melee attacks to shock your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_ELEC));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_air_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Air");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_air");
        break;
    case SPELL_DESC:
        var_set_string(res, "This temporary rune allows you to attack all adjacent foes in a whirlwind of destruction.");
        break;
    default:
        massacre_spell(cmd, res);
    }
}

void _blow_mana_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_mana");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your most powerful battle rune uses your mana to powerfully rend your enemies.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_MANA));
        break;
    default:
        default_spell(cmd, res);
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
 #define _MAX_SPELLS_PER_GROUP  25
 #define _MAX_SPELL_GROUPS       4

typedef struct {
    cptr name;
    cptr help;
    int color;
    spell_info spells[_MAX_SPELLS_PER_GROUP];    /* There is always a sentinel at the end */
} _spell_group, *_spell_group_ptr;

static _spell_group _spell_groups[_MAX_SPELL_GROUPS] = {
    { "Runes of Creation",
      "Augment your equipment by attaching runes of various powers. Also, you may create "
      "certain stand-alone runes that grant powers by virtue of being present in your "
      "inventory. Be sure to always keep Absorption handy, for it is your only means "
      "of regaining spell points!",
      TERM_L_BLUE,
      { {  1,   0, 0, _obj_absorption_spell },
        {  5,   0, 0, _obj_protection_spell },
        {  7,   0, 0, _obj_regeneration_spell },
        {  9,   0, 0, _obj_fire_spell },
        { 11,   0, 0, _obj_air_spell },
        { 13,   0, 0, _obj_water_spell },
        { 15,   0, 0, _obj_light_spell },
        { 17,   0, 0, _obj_shadow_spell },
        { 19,   0, 0, _obj_earth_spell },
        { 21,   0, 0, _obj_understanding_spell },
        { 23,   0, 0, _obj_elemental_protection_spell },
        { 25,   0, 0, _obj_haste_spell },
        { 27,   0, 0, _obj_seeing_spell },
        /* XXX I really don't like the playstyle that {Sacrifice} encourages:
         * viz., a tedious process of storing up mana, denying interesting spell
         * casting opportunities, in order to stock pile for the end game. Look,
         * there will be hundreds of junk artifacts, yet it takes about 30 minutes
         * to recover full mana for each rune (Might be faster later on ... and, no,
         * I don't scum for sp). How long should the game be? Personally, I found
         * myself using {Sacrifice} to speed up equipment reshuffles, which is
         * probably not the intended purpose!
        { 29,   0, 0, _obj_sacrifice_spell }, */
        { 29,   0, 0, _obj_life_spell },
        { 31,   0, 0, _obj_stability_spell },
        { 33,   0, 0, _obj_reflection_spell },
        { 35,   0, 0, _obj_death_spell },
        { 37,   0, 0, _obj_mind_spell },
        { 39,   0, 0, _obj_might_spell },
        { 41,   0, 0, _obj_destruction_spell },
        { 43,   0, 0, _obj_good_fortune_spell },
        { 45,   0, 0, _obj_immortality_spell },
        { -1,   0, 0, NULL },
      }
    },
    { "Runes of Enhancement",
      "Place runes on yourself for temporary or one-time effects.",
      TERM_L_GREEN, {
        { 10,   5, 20, _self_darkness_spell },
        { 20,   9, 30, _self_seeing_spell },
        { 25,  50, 35, _self_understanding_spell },
        { 30,  25, 35, _self_haste_spell },
        { 32,  15, 35, _self_protection_spell },
        { 37,  20, 50, _self_earth_spell },
        { 39,  15, 50, _self_life_spell },
        { 41,  60, 70, _self_daemon_spell },
        { 50, 100, 80, _self_might_spell },
        { -1,   0,  0, NULL },
      }
    },
    { "Runes of Alteration",
      "These runes of change allow you to permanently alter your surroundings.",
      TERM_UMBER, { 
        {  5,  1, 20, _feat_light_spell },
        {  7,  5, 30, _feat_water_spell },
        { 15, 15, 50, _feat_earth_spell },
        { 20, 10, 50, _feat_fire_spell },
        { 25, 15, 60, _feat_air_spell },
        { 30, 15, 65, _feat_stability_spell },
        { 35, 20, 70, _feat_life_spell },
        { 40, 70, 70, _feat_protection_spell },
        { 45, 35, 80, _feat_destruction_spell },
        { -1,  0,  0, NULL },
      }
    },
    { "Runes of Battle",
      "These runes allow you to effect adjacent enemies. By creating a temporary rune "
      "attached to your melee weapon, you may make a single attack with enhanced power.",
      TERM_RED, {
        { 10,  5,  0, _blow_confusion_spell },
        { 15,  7,  0, _blow_fire_spell },
        { 25,  9,  0, _blow_water_spell },
        { 30,  5,  0, _blow_earth_spell },
        { 35, 12,  0, _blow_death_spell },
        { 40, 20,  0, _blow_elec_spell },
        { 45, 30, 80, _blow_air_spell },
        { 50, 20,  0, _blow_mana_spell },
        { -1,  0,  0, NULL },
      }
    },
};

static int _get_spells_imp(spell_info* spells, int max, _spell_group *spell_group)
{
    int i;
    int ct = 0;
    int stat_idx = p_ptr->stat_ind[A_INT];
    
    for (i = 0; ; i++)
    {
        spell_info *base = &spell_group->spells[i];
        if (base->level < 0) break;
        if (ct >= max) break;
        if (base->level <= p_ptr->lev)
        {
            spell_info* current = &spells[ct];
            current->fn = base->fn;
            current->level = base->level;
            current->cost = base->cost;

            current->fail = calculate_fail_rate(base->level, base->fail, stat_idx);            
            ct++;
        }
    }
    return ct;
}

static void _character_dump(doc_ptr doc)
{
    int i;
    doc_printf(doc, "<topic:Spells>==================================== <color:keypress>S</color>pells ===================================\n\n");
    for (i = 0; i < _MAX_SPELL_GROUPS; i++)
    {
        _spell_group_ptr group = &_spell_groups[i];
        spell_info       spells[_MAX_SPELLS_PER_GROUP];
        int              ct = _get_spells_imp(spells, _MAX_SPELLS_PER_GROUP, group); 

        if (!ct) continue;
        doc_printf(doc, "<color:%c>%s</color>\n", attr_to_attr_char(group->color), group->name);
        py_display_spells_aux(doc, spells, ct);
    }
}

static void _spell_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, _spell_groups[which].name);
        break;
    case MENU_HELP:
        var_set_string(res, _spell_groups[which].help);
        break;
    case MENU_COLOR:
        var_set_int(res, _spell_groups[which].color);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static int _get_spells(spell_info* spells, int max)
{
    int idx = -1;
    int ct = 0;
    menu_t menu = { "Use which group of spells?", "Browse which group of spells?", NULL,
                    _spell_menu_fn, _spell_groups, _MAX_SPELL_GROUPS, 0};

    idx = menu_choose(&menu);
    if (idx < 0) return 0;
    ct = _get_spells_imp(spells, max, &_spell_groups[idx]);
    if (ct == 0)
        msg_print("You don't know any of those spells yet!");
    return ct;
}

static void _calc_bonuses(void)
{
    p_ptr->spell_cap += 7;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 3000;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 1200;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    object_type forge = {0};
    object_prep(&forge, lookup_kind(TV_SWORD, SV_BROAD_SWORD));
    rune_add(&forge, RUNE_ABSORPTION, FALSE);
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

static bool _destroy_object(obj_ptr obj)
{
    if (obj->rune == RUNE_SACRIFICE)
    {   /* XXX Score the object and scale the effect. The bigger the 'sacrifice'
         * the greater the benefit, no? Perhaps allow non-artifacts as well? */
        bool is_equipped = obj->loc.where == INV_EQUIP;
        int add_hp = is_equipped ? p_ptr->mhp : p_ptr->mhp/3;
        int add_sp = is_equipped ? p_ptr->msp : p_ptr->msp/3;

        msg_print("You feel a surge of wondrous power enter your body.");

        p_ptr->chp = MIN(p_ptr->mhp, p_ptr->chp + add_hp);
        p_ptr->chp_frac = 0;
        p_ptr->csp = MIN(p_ptr->msp, p_ptr->csp + add_sp);
        p_ptr->csp_frac = 0;

        p_ptr->redraw |= (PR_MANA);
        p_ptr->window |= (PW_SPELL);
        p_ptr->redraw |= (PR_HP);

        if (is_equipped)
        {
            blast_object(obj);
            obj->curse_flags = OFC_HEAVY_CURSE;
        }
        return TRUE;
    }
    return FALSE;
}

class_t *rune_knight_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  25,  36,   2,  18,  16,  50,  35};
    skills_t xs = {  7,   9,  10,   0,   0,   0,  14,  11};

        me.name = "Rune-Knight";
        me.desc = 
            "The Rune-Knight is a mythical warrior, "
            "dedicated to the discovery of ancient runes that hold immense power. They "
            "may fix mystical runes of various types to their equipment in order to "
            "gain permanent bonuses; even artifacts can be improved this way! "
            "Alternatively, they may conjure a temporary rune which allows "
            "them to attack with enhanced effects, or place one directly on "
            "their person for a one-time or temporary bonus. "
            "Finally, they may alter their surroundings with various runes of "
            "change.\n \n"
            "All runes (except <color:B>{Absorption}</color>) require mana for "
            "creation; however, unlike ordinary spellcasters, Rune-Knights do not "
            "regenerate mana on their own. Rather, they must siphon mana from "
            "magical or elemental attacks directed against them, and doing so "
            "requires the special rune of <color:B>{Absorption}</color>. This "
            "rune should be worn at all times, or at least kept handy.\n \n"
            "Despite having mana, the Rune-Knight does not play like an ordinary "
            "spellcaster; they are more like a weaponsmith but can, on occasion, "
            "cast useful spells. If you have mana available, consider using these "
            "spells; otherwise, play as a warrior and wait to absorb mana. This "
            "can take time, depending on the foes you face, but the Rune-Knight's "
            "honor should prevent them from seeking out weak, defenseless spellcasters; "
            "being one of the strongest classes, Rune-Knights have no need to "
            "resort to exploits...";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 101;
        me.base_hp = 7;
        me.exp = 150;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.destroy_object = _destroy_object;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
