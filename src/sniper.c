#include "angband.h"

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_BOW, SV_LIGHT_XBOW, 1);
    py_birth_obj_aux(TV_BOLT, SV_BOLT, rand_range(20, 30));
}

/************************************************************************
 * Bonuses
 ***********************************************************************/
static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if ((info_ptr->tval_ammo == TV_BOLT) || (info_ptr->tval_ammo == TV_ANY_AMMO))
    {
        info_ptr->to_h += 10 + p_ptr->lev/5;
        info_ptr->dis_to_h += 10 + p_ptr->lev/5;
    }
    if (info_ptr->base_shot > 100)
        info_ptr->base_shot = 100 + (info_ptr->base_shot - 100) / 2;
}

/************************************************************************
 * Concentration
 ***********************************************************************/
static int _max_concentration(void)
{
    return 2 + (p_ptr->lev + 5)/10;
}

void reset_concentration(bool msg)
{
    if (msg)
        msg_print("You stop concentrating.");

    p_ptr->concent = 0;
    reset_concent = FALSE;
    p_ptr->update |= PU_BONUS;
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_MONSTERS;
}

int boost_concentration_damage(int tdam)
{
    return tdam * (10 + p_ptr->concent) / 10;
}

static void _concentrate(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Concentration");
        break;
    case SPELL_DESC:
        var_set_string(res,
            "Concentrate your mind for more powerful shooting. As you increase "
            "your focus, you will gain access to more deadly archery techniques. "
            "In addition, you will land more critical shots as your aim improves.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("(%d of %d)", p_ptr->concent, _max_concentration()));
        break;
    case SPELL_CAST: {
        int max = _max_concentration();
        if (p_ptr->concent < max)
        {
            p_ptr->concent++;
            msg_format("You concentrate deeply (<color:%c>%dx</color>).",
                p_ptr->concent == max ? 'r' : 'B',
                p_ptr->concent);
            p_ptr->update |= PU_BONUS;
            p_ptr->redraw |= PR_STATUS;
            p_ptr->update |= PU_MONSTERS;
        }
        else
            msg_format("You maintain maximum focus (<color:r>%dx</color>).", p_ptr->concent);
        reset_concent = FALSE;
        var_set_bool(res, TRUE);
        break; }
    default:
        default_spell(cmd, res);
    }
}

/************************************************************************
 * Snipe Techniques: Callbacks for do_cmd_fire based on shoot_hack
 ***********************************************************************/
int sniper_multiplier(int which, obj_ptr ammo, monster_type *m_ptr)
{
    int           mult = 10;
    monster_race *r_ptr = NULL;
    u32b          flgs[OF_ARRAY_SIZE] = {0};

    if (m_ptr)
        r_ptr = &r_info[m_ptr->r_idx];
    if (ammo)
        missile_flags(ammo, flgs);

    switch (which)
    {
    case SP_LITE:
        if (!r_ptr || (r_ptr->flags3 & RF3_HURT_LITE))
        {
            mult = 20 + p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_HURT_LITE);
        }
        break;
    case SP_FIRE:
        if (r_ptr && (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK))
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
        }
        else
        {
            mult = 15 + 3*p_ptr->concent;
            if (have_flag(flgs, OF_BRAND_FIRE))
                mult += 5;
            if (r_ptr && (r_ptr->flags3 & RF3_HURT_FIRE))
            {
                mult *= 2;
                mon_lore_3(m_ptr, RF3_HURT_FIRE);
            }
        }
        break;
    case SP_COLD:
        if (r_ptr && (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK))
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
        }
        else
        {
            mult = 15 + 3*p_ptr->concent;
            if (have_flag(flgs, OF_BRAND_COLD))
                mult += 5;
            if (r_ptr && (r_ptr->flags3 & RF3_HURT_COLD))
            {
                mult *= 2;
                mon_lore_3(m_ptr, RF3_HURT_COLD);
            }
        }
        break;
    case SP_ELEC:
        if (r_ptr && (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK))
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_ELEC_MASK);
        }
        else
        {
            mult = 18 + 4*p_ptr->concent;
            if (have_flag(flgs, OF_BRAND_ELEC))
                mult += 7;
        }
        break;
    case SP_KILL_WALL:
        if (!r_ptr || (r_ptr->flags3 & RF3_HURT_ROCK))
        {
            mult = 15 + 2*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_HURT_ROCK);
        }
        else if (!r_ptr || (r_ptr->flags3 & RF3_NONLIVING))
        {
            mult = 15 + 2*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_NONLIVING);
        }
        break;
    case SP_EVILNESS:
        if (!r_ptr || (r_ptr->flags3 & RF3_GOOD))
        {
            mult = 15 + 4*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_GOOD);
			if (have_flag(flgs, OF_KILL_GOOD))
				mult += 10;
            if (have_flag(flgs, OF_SLAY_GOOD))
                mult += 5;
        }
        break;
    case SP_HOLYNESS:
        if (!r_ptr || (r_ptr->flags3 & RF3_EVIL))
        {
            mult = 15 + 4*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_EVIL);
            if (r_ptr && (r_ptr->flags3 & RF3_HURT_LITE))
            {
                mult += 3*p_ptr->concent;
                mon_lore_3(m_ptr, RF3_HURT_LITE);
            }
            if (have_flag(flgs, OF_KILL_EVIL))
                mult += 10;
            if (have_flag(flgs, OF_SLAY_EVIL))
                mult += 5;
        }
        break;
    case SP_FINAL:
        mult = 50;
        break;
    }

    return mult;
}
/************************************************************************
 * Spells
 ***********************************************************************/
static bool _do_shot(int which)
{
    bool result = FALSE;
    if (!equip_find_obj(TV_BOW, SV_ANY))
    {
        msg_print("You need to wield a bow!");
        return FALSE;
    }
    shoot_hack = which;
    command_cmd = 'f'; /* hack for @fa inscriptions */
    result = do_cmd_fire();
    shoot_hack = 0;
    return result;
}
static char *_mult_info(int mult)
{
    return format("%d.%dx", mult/10, mult%10);
}
static void _default(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_INFO:
        var_set_string(res, _mult_info(sniper_multiplier(which, NULL, NULL)));
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_shot(which));
        break;
    case SPELL_ON_BROWSE: {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        display_shooter_mode = which;
        do_cmd_knowledge_shooter();
        display_shooter_mode = 0;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break; }
    case SPELL_ENERGY:
        var_set_int(res, energy_use); /* roundabout ... but do_cmd_fire already set this */
        break;
    default:
        default_spell(cmd, res);
    }
}
static void _shining_arrow(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shining Arrow");
        break;
    case SPELL_DESC:
        var_set_string(res,
            "Shoot a glowing arrow that lights up the dungeon. This "
            "shot also does increased damage from light against "
            "enemies that are hurt by bright light.");
        break;
    default:
        _default(SP_LITE, cmd, res);
    }
}
static void _shoot_and_away(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shoot and Away");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoot at target and then blink away in a single move.");
        break;
    default:
        _default(SP_AWAY, cmd, res);
    }
}
static void _disarming_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Disarming Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoot an arrow able to shatter traps.");
        break;
    default:
        _default(SP_KILL_TRAP, cmd, res);
    }
}
static void _burning_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Burning Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals extra damage of fire.");
        break;
    default:
        _default(SP_FIRE, cmd, res);
    }
}
static void _shatter(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shatter");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoot an arrow able to shatter rocks.");
        break;
    default:
        _default(SP_KILL_WALL, cmd, res);
    }
}
static void _freezing_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Freezing Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals extra damage of cold.");
        break;
    default:
        _default(SP_COLD, cmd, res);
    }
}
static void _knockback(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Knockback");
        break;
    case SPELL_DESC:
        var_set_string(res, "A powerful shot that knocks an enemy target backwards.");
        break;
    default:
        _default(SP_RUSH, cmd, res);
    }
}
static void _piercing_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Piercing Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "An arrow pierces some monsters.");
        break;
    default:
        _default(SP_PIERCE, cmd, res);
    }
}
static void _evil_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Evil Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals more damage to good monsters.");
        break;
    default:
        _default(SP_EVILNESS, cmd, res);
    }
}
static void _holy_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Holy Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals more damage to evil monsters.");
        break;
    default:
        _default(SP_HOLYNESS, cmd, res);
    }
}
static void _exploding_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Exploding Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "An arrow explodes when it hits a monster.");
        break;
    default:
        _default(SP_EXPLODE, cmd, res);
    }
}
static void _double_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Double Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoot arrows twice.");
        break;
    default:
        _default(SP_DOUBLE, cmd, res);
    }
}
static void _thunder_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Thunder Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals great extra damage of lightning.");
        break;
    default:
        _default(SP_ELEC, cmd, res);
    }
}
static void _needle_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Needle Shot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals quick death or 1 damage.");
        break;
    default:
        _default(SP_NEEDLE, cmd, res);
    }
}
static void _saint_stars_arrow(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Saint Stars Arrow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Deals great damage to all monsters, and some side effects to you.");
        break;
    default:
        _default(SP_FINAL, cmd, res);
    }
}
static spell_info _get_spells[] =
{
   /*lvl  cst fail  spell */
    {  1,   0,   0, _concentrate },
    {  2,   1,   0, _shining_arrow },
    {  3,   1,   0, _shoot_and_away },
    {  5,   1,   0, _disarming_shot },
    {  8,   2,   0, _burning_shot },
    { 10,   2,   0, _shatter },
    { 13,   2,   0, _freezing_shot },
    { 18,   2,   0, _knockback },
    { 22,   3,   0, _piercing_shot },
    { 25,   4,   0, _evil_shot },
    { 26,   4,   0, _holy_shot },
    { 30,   3,   0, _exploding_shot },
    { 32,   4,   0, _double_shot },
    { 36,   3,   0, _thunder_shot },
    { 40,   3,   0, _needle_shot },
    { 48,   7,   0, _saint_stars_arrow },
    { -1,  -1,  -1, NULL}
};
static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "sniping";
        me.options = CASTER_USE_CONCENTRATION;
        me.which_stat = A_DEX;
        init = TRUE;
    }
    return &me;
}

/************************************************************************
 * Powers
 ***********************************************************************/
static power_info _get_powers[] =
{
    { A_INT, {15, 20, 80, probing_spell}},
    { -1, {-1, -1, -1, NULL} }
};

/************************************************************************
 * Class
 ***********************************************************************/
class_t *sniper_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  24,  28,   5,  32,  18,  35,  72};
    skills_t xs = { 12,  10,  10,   0,   0,   0,  12,  28};

        me.name = "Sniper";
        me.desc = "Snipers are specialists in marksmanship, but not like archers who "
                    "fire off arrow after arrow in swift succession. They don't just "
                    "increase accuracy and power of shots by concentration, they can use "
                    "fearsome archery techniques.\n \n"
                    "What they require is powerful bows or crossbows, good quality "
                    "ammunition and the fortitude to bear up without flinching under " 
                    "any situation.\n \n"
                    "Snipers know their enemies well and can shoot them from the shadows. "
                    "They have no time for magic.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 110;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG;
        
        me.birth = _birth;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        me.get_powers = _get_powers;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.character_dump = py_dump_spells;
        init = TRUE;
    }

    return &me;
}
