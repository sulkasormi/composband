#include "angband.h"
#include <assert.h>

static int _rank(void)
{
    int rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 25) rank++;
    if (p_ptr->lev >= 35) rank++;
    if (p_ptr->lev >= 45) rank++;
    return rank;
}

/**********************************************************************
 * Essences
 **********************************************************************/
#define _MAX_ESSENCE 255
#define _MIN_SPECIAL 240  /* cf TR_FLAG_MAX */
enum {
    _ESSENCE_AC = _MIN_SPECIAL,
    _ESSENCE_TO_HIT,
    _ESSENCE_TO_DAM,
    _ESSENCE_XTRA_DICE
};

static int _essences[_MAX_ESSENCE] = {0};

static void _load(savefile_ptr file)
{
    int ct, i;

    for (i = 0; i < _MAX_ESSENCE; i++)
        _essences[i] = 0;

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        int j = savefile_read_s16b(file);
        int n = savefile_read_s16b(file);

        if (0 <= j && j < _MAX_ESSENCE)
            _essences[j] += n;
    }
}

static void _save(savefile_ptr file)
{
    int ct = 0, i;

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        if (_essences[i])
            ct++;
    }

    savefile_write_s16b(file, ct);

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        if (_essences[i])
        {
            savefile_write_s16b(file, i);
            savefile_write_s16b(file, _essences[i]);
        }
    }
}

static bool _skip_flag(int which)
{
    switch (which)
    {
    case OF_RIDING:
    case OF_THROWING:
    case OF_HIDE_TYPE:
    case OF_SHOW_MODS:
    case OF_IGNORE_ACID:
    case OF_IGNORE_ELEC:
    case OF_IGNORE_FIRE:
    case OF_IGNORE_COLD:
    case OF_ACTIVATE:
    case OF_FULL_NAME:
    case OF_FIXED_FLAVOR:
        return TRUE;
    }
    return FALSE;
}

static bool _add_essence(int which, int amount)
{
    int n = _essences[which];

    if (amount > 0)
        n += amount;

    if (n > 10000)
        n = 10000;

    if (n != _essences[which])
    {
        _essences[which] = n;
        return TRUE;
    }

    return FALSE;
}

void obj_essence_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    obj_flags(o_ptr, flgs);
    
    /* Make sure the Slay flag exists if the Kill flag exists */
    if (have_flag(flgs, OF_KILL_EVIL)) add_flag(flgs, OF_SLAY_EVIL);
    if (have_flag(flgs, OF_KILL_GOOD)) add_flag(flgs, OF_SLAY_GOOD);
    if (have_flag(flgs, OF_KILL_LIVING)) add_flag(flgs, OF_SLAY_LIVING);
    if (have_flag(flgs, OF_KILL_DRAGON)) add_flag(flgs, OF_SLAY_DRAGON);
    if (have_flag(flgs, OF_KILL_DEMON)) add_flag(flgs, OF_SLAY_DEMON);
    if (have_flag(flgs, OF_KILL_ANIMAL)) add_flag(flgs, OF_SLAY_ANIMAL);
    if (have_flag(flgs, OF_KILL_UNDEAD)) add_flag(flgs, OF_SLAY_UNDEAD);
    if (have_flag(flgs, OF_KILL_HUMAN)) add_flag(flgs, OF_SLAY_HUMAN);
    if (have_flag(flgs, OF_KILL_ORC)) add_flag(flgs, OF_SLAY_ORC);
    if (have_flag(flgs, OF_KILL_TROLL)) add_flag(flgs, OF_SLAY_TROLL);
    if (have_flag(flgs, OF_KILL_GIANT)) add_flag(flgs, OF_SLAY_GIANT);
}

static bool _absorb(object_type *o_ptr)
{
    bool result = FALSE;
    int i;
    int mult = o_ptr->number, div = 1;
    object_kind *k_ptr = &k_info[o_ptr->k_idx];
    u32b flags[OF_ARRAY_SIZE];
    obj_essence_flags(o_ptr, flags);

    /* Check whether the item we are absorbing completes a quest */
    quests_on_get_obj(o_ptr);

    /* No absorbing the same artifact repeatedly... */
    if (o_ptr->name1) 
    {
        if (!p_ptr->noscore) assert(a_info[o_ptr->name1].generated);
        a_info[o_ptr->name1].found = TRUE;
    }
    else if ((o_ptr->art_name) && (!(o_ptr->marked & OM_ART_COUNTED))) /* Bookkeeping */
    {
        stats_rand_art_counts.found += o_ptr->number;
    }

    if (o_ptr->curse_flags & OFC_AGGRAVATE)
        div++;
    if (o_ptr->curse_flags & (OFC_TY_CURSE | OFC_HEAVY_CURSE))
        div++;

    if (!have_flag(flags, OF_BRAND_ORDER) && !have_flag(flags, OF_BRAND_WILD))
    {
        if (_add_essence(_ESSENCE_XTRA_DICE, (o_ptr->ds - k_ptr->ds)*mult/1/*div?*/))
            result = TRUE;
        if (_add_essence(_ESSENCE_XTRA_DICE, (o_ptr->dd - k_ptr->dd)*mult/1/*div?*/))
            result = TRUE;
    }

    for (i = 0; i < OF_COUNT; i++)
    {
        if (_skip_flag(i)) continue;
        if (have_flag(flags, i))
        {
            if (is_pval_flag(i))
            {
                if (_add_essence(i, o_ptr->pval*mult/div))
                    result = TRUE;
            }
            else
            {
                _essences[i] += mult;
                result = TRUE;
            }
        }
    }

    if (_add_essence(_ESSENCE_AC, o_ptr->to_a*mult/div))
        result = TRUE;
    if (_add_essence(_ESSENCE_TO_HIT, o_ptr->to_h*mult/div))
        result = TRUE;
    if (_add_essence(_ESSENCE_TO_DAM, o_ptr->to_d*mult/div))
        result = TRUE;

    if (result)
    {
        p_ptr->update |= PU_BONUS;
        msg_print("You grow stronger!");
    }
    return result;
}

static bool _absorb_object(object_type *o_ptr)
{
    if (object_is_melee_weapon(o_ptr))
    {
        char o_name[MAX_NLEN];
        bool tunnettu = (((obj_is_identified(o_ptr)) || (o_ptr->feeling == FEEL_AVERAGE)) && (obj_is_identified_fully(o_ptr)));
        /* bizarrely, an object can be "fully identified" without being "identified", so we need to check both */

        if (!tunnettu)
        {
            obj_identify_fully(o_ptr);
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
        }
        else object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_COLOR_CODED);
        msg_format("You attempt to drain power from %s.", o_name);

        _absorb(o_ptr);
        return TRUE;
    }
    return FALSE;
}


static int _calc_amount(int amount, int power, int rep)
{
    int result = 0;
    int ct = 0;

    while (amount > 0)
    {
        if (amount >= power)
        {
            result++;
            ct++;
        }
        amount -= power;

        if (ct % rep == 0)
        {
            power *= 2;
            if (power > 1024)
                power = 1024;
        }
    }

    return result;
}

static int _calc_needed(int amount, int power, int rep)
{
    int result = 0;
    int ct = 0;
    /* It would be cleaner to loop until _calc_amount increased, but the
       power required increases exponentially! */
    for (;;)
    {
        if (amount >= power)
            ct++;
        result += power;
        amount -= power;
        if (amount < 0)
            break;
        if (ct % rep == 0)
        {
            power *= 2;
            if (power > 1024)
                power = 1024;
        }
    }

    return result;
}

static int _free_act_needed(void)
{
    int paljonko = 2;
    if (_essences[OF_FREE_ACT] >= 2) paljonko = 6;
    if (_essences[OF_FREE_ACT] >= 6) paljonko = 16;
    return paljonko;
}

static int _see_invis_needed(void)
{
    int paljonko = 3;
    if (_essences[OF_SEE_INVIS] >= 3) paljonko = 9;
    if (_essences[OF_SEE_INVIS] >= 9) paljonko = 19;
    if (_essences[OF_SEE_INVIS] >= 19) paljonko = 33;
    return paljonko;
}

static int _calc_stat_bonus(int flag)
{
    return _calc_amount(_essences[flag], 3, 1);
}

static void _add_stat_flag(int flag, u32b flgs[OF_ARRAY_SIZE])
{
    if (_calc_stat_bonus(flag))
        add_flag(flgs, flag);
}

static int _res_power(int which)
{
    switch (which)
    {
    case RES_ACID:
    case RES_FIRE:
    case RES_COLD:
    case RES_ELEC:
    case RES_CONF:
    case RES_FEAR:
    case RES_TIME:
    case RES_TELEPORT:
        return 2;

    case RES_SOUND:
    case RES_SHARDS:
    case RES_CHAOS:
        return 4;

    case RES_DISEN:
    case RES_POIS:
    case RES_LITE:
    case RES_DARK:
    case RES_NETHER:
    case RES_NEXUS:
        return 3;
    }

    return 2;
}

/**********************************************************************
 * Attacks and Bonuses
 **********************************************************************/
typedef struct {
    int flag;
    int power;
    cptr name;
} _flag_info_t;

static _flag_info_t _slay_flag_info[] = {
    { OF_SLAY_EVIL, 64, "Slay Evil" },
    { OF_SLAY_GOOD, 16, "Slay Good" },
    { OF_SLAY_UNDEAD, 16, "Slay Undead" },
    { OF_SLAY_DEMON, 16, "Slay Demon" },
    { OF_SLAY_DRAGON, 16, "Slay Dragon" },
    { OF_SLAY_HUMAN, 16, "Slay Human" },
    { OF_SLAY_ANIMAL, 8, "Slay Animal" },
    { OF_SLAY_ORC, 4, "Slay Orc" },
    { OF_SLAY_TROLL, 8, "Slay Troll" },
    { OF_SLAY_GIANT, 8, "Slay Giant" },
    { OF_SLAY_LIVING, 8, "Slay Living" },

    { OF_BRAND_ACID, 24, "Brand Acid" },
    { OF_BRAND_ELEC, 24, "Brand Elec" },
    { OF_BRAND_FIRE, 24, "Brand Fire" },
    { OF_BRAND_COLD, 24, "Brand Cold" },
    { OF_BRAND_POIS, 24, "Brand Poison" },
    { OF_BRAND_CHAOS, 32, "Chaotic" },
    { OF_BRAND_VAMP, 32, "Vampiric" },
    { OF_VORPAL, 16, "Vorpal" },
    { OF_VORPAL2, 32, "*Vorpal*" },

    /* Alt: These could be achieved from corresponding slay flags with enough essences */
    { OF_KILL_EVIL, 16, "Kill Evil" },
	{ OF_KILL_GOOD, 8, "Kill Good" },
    { OF_KILL_UNDEAD, 8, "Kill Undead" },
    { OF_KILL_DEMON, 8, "Kill Demon" },
    { OF_KILL_DRAGON, 8, "Kill Dragon" },
    { OF_KILL_HUMAN, 8, "Kill Human" },
    { OF_KILL_ANIMAL, 8, "Kill Animal"},
    { OF_KILL_ORC, 8, "Kill Orc" },
    { OF_KILL_TROLL, 8, "Kill Troll" },
    { OF_KILL_GIANT, 8, "Kill Giant" },
	{ OF_KILL_LIVING, 8, "Kill Living" },

    { -1, 0, NULL}
};

static int _rank_decay(int p)
{
    int r = _rank();
    for (; r < 4; r++)
        p /= 2;

    return MAX(1, p);
}

static int _slay_power(int i)
{
    return _rank_decay(_slay_flag_info[i].power);
}

static int _blows_mult(void)
{
    switch (_rank())
    {
    case 0: return 100;
    case 1: return 100;
    case 2: return 75;
    case 3: return 50;
    case 4:
    default: return 20;
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int i;
    int to_hit = _calc_amount(_essences[_ESSENCE_TO_HIT], 1, 1);
    int to_dam = _calc_amount(_essences[_ESSENCE_TO_DAM], 1, 1);
    int to_dd = _calc_amount(_essences[_ESSENCE_XTRA_DICE], _rank_decay(64), 1);
    int blows = _calc_amount(_essences[OF_BLOWS], _rank_decay(32), 1);

    for (i = 0; i < OF_ARRAY_SIZE; i++)
        o_ptr->flags[i] = 0;

    add_flag(o_ptr->flags, OF_NO_REMOVE);

    for (i = 0; ; i++)
    {
        int j = _slay_flag_info[i].flag;
        if (j < 0) break;
        if (_essences[j] >= _slay_power(i))
        {
            add_flag(o_ptr->flags, j);
            add_flag(o_ptr->known_flags, j);
        }
    }

    info_ptr->xtra_blow += blows * _blows_mult();

    info_ptr->to_h += to_hit;
    info_ptr->dis_to_h += to_hit;

    info_ptr->to_d += to_dam;
    info_ptr->dis_to_d += to_dam;

    info_ptr->to_dd += to_dd;
}

static void _calc_bonuses(void) 
{
    int i;
    int to_a = py_prorata_level(135);

    to_a += _calc_amount(_essences[_ESSENCE_AC], 2, 10);
    if (p_ptr->current_r_idx == MON_DEATH_SCYTHE)
        to_a -= 50;
    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    p_ptr->levitation = TRUE;
    p_ptr->no_cut = TRUE;
    res_add(RES_BLIND);
    res_add(RES_POIS);
    p_ptr->hold_life++;

    if (p_ptr->lev >= 10)
        p_ptr->pspeed += 1;

    if (p_ptr->lev >= 25)
        p_ptr->pspeed += 1;

    if (p_ptr->lev >= 35)
        p_ptr->pspeed += 2;

    if (p_ptr->lev >= 45)
    {
        p_ptr->pspeed += 2;
        p_ptr->sh_retaliation = TRUE;
    }

    for (i = 0; i < RES_MAX; i++)
    {
        int j = res_get_object_flag(i);
        int n = _calc_amount(_essences[j], _res_power(i), 1);

        for (; n; --n)
            res_add(i);
    }
    if (_essences[OF_IM_ACID] >= 3)
        res_add_immune(RES_ACID);
    if (_essences[OF_IM_ELEC] >= 3)
        res_add_immune(RES_ELEC);
    if (_essences[OF_IM_FIRE] >= 3)
        res_add_immune(RES_FIRE);
    if (_essences[OF_IM_COLD] >= 3)
        res_add_immune(RES_COLD);

    p_ptr->life += 3*_calc_amount(_essences[OF_LIFE], 7, 1);

    p_ptr->skills.stl += _calc_amount(_essences[OF_STEALTH], 2, 1);
    p_ptr->pspeed += _calc_amount(_essences[OF_SPEED], 1, 6);
    p_ptr->skills.dev += 8*_calc_amount(_essences[OF_MAGIC_MASTERY], 2, 1);
    p_ptr->device_power += _calc_amount(_essences[OF_DEVICE_POWER], 2, 1);
    p_ptr->skills.srh += 5*_calc_amount(_essences[OF_SEARCH], 2, 1);
    p_ptr->skills.fos += 5*_calc_amount(_essences[OF_SEARCH], 2, 1);
    p_ptr->see_infra += _calc_amount(_essences[OF_INFRA], 2, 1);
    p_ptr->skill_dig += 20*_calc_amount(_essences[OF_TUNNEL], 2, 1);

    if (_essences[OF_SUST_STR] >= 5)
        p_ptr->sustain_str = TRUE;
    if (_essences[OF_SUST_INT] >= 5)
        p_ptr->sustain_int = TRUE;
    if (_essences[OF_SUST_WIS] >= 5)
        p_ptr->sustain_wis = TRUE;
    if (_essences[OF_SUST_DEX] >= 5)
        p_ptr->sustain_dex = TRUE;
    if (_essences[OF_SUST_CON] >= 5)
        p_ptr->sustain_con = TRUE;
    if (_essences[OF_SUST_CHR] >= 5)
        p_ptr->sustain_chr = TRUE;

    if (_essences[OF_TELEPATHY] >= 2)
        p_ptr->telepathy = TRUE;
    if (_essences[OF_ESP_ANIMAL] >= 2)
        p_ptr->esp_animal = TRUE;
    if (_essences[OF_ESP_UNDEAD] >= 2)
        p_ptr->esp_undead = TRUE;
    if (_essences[OF_ESP_DEMON] >= 2)
        p_ptr->esp_demon = TRUE;
    if (_essences[OF_ESP_ORC] >= 2)
        p_ptr->esp_orc = TRUE;
    if (_essences[OF_ESP_TROLL] >= 2)
        p_ptr->esp_troll = TRUE;
    if (_essences[OF_ESP_GIANT] >= 2)
        p_ptr->esp_giant = TRUE;
    if (_essences[OF_ESP_DRAGON] >= 2)
        p_ptr->esp_dragon = TRUE;
    if (_essences[OF_ESP_HUMAN] >= 2)
        p_ptr->esp_human = TRUE;
    if (_essences[OF_ESP_EVIL] >= 2)
        p_ptr->esp_evil = TRUE;
    if (_essences[OF_ESP_GOOD] >= 2)
        p_ptr->esp_good = TRUE;
    if (_essences[OF_ESP_NONLIVING] >= 2)
        p_ptr->esp_nonliving = TRUE;
	if (_essences[OF_ESP_LIVING] >= 2)
		p_ptr->esp_living = TRUE;
    if (_essences[OF_ESP_UNIQUE] >= 2)
        p_ptr->esp_unique = TRUE;

    if (_essences[OF_NO_MAGIC] >= 5)
        p_ptr->anti_magic = TRUE;
    if (_essences[OF_FREE_ACT] >= 2)
        p_ptr->free_act++;
    if (_essences[OF_FREE_ACT] >= 6)
        p_ptr->free_act++;
    if (_essences[OF_FREE_ACT] >= 16)
        p_ptr->free_act++;
    if (_essences[OF_SEE_INVIS] >= 3)
        p_ptr->see_inv++;
    if (_essences[OF_SEE_INVIS] >= 9)
        p_ptr->see_inv++;
    if (_essences[OF_SEE_INVIS] >= 19)
        p_ptr->see_inv++;
    if (_essences[OF_SEE_INVIS] >= 33)
        p_ptr->see_inv++;
    if (_essences[OF_SLOW_DIGEST] >= 2)
        p_ptr->slow_digest = TRUE;
    if (_essences[OF_REGEN] >= 7)
        p_ptr->regen += 100;
    if (_essences[OF_REFLECT] >= 3)
        p_ptr->reflect = TRUE;

    if (_essences[OF_AURA_FIRE] >= 7)
        p_ptr->sh_fire++;
    if (_essences[OF_AURA_ELEC] >= 7)
        p_ptr->sh_elec++;
    if (_essences[OF_AURA_COLD] >= 7)
        p_ptr->sh_cold++;
}

static void _calc_stats(s16b stats[MAX_STATS])
{
    int i;
    for (i = 0; i < 6; i++)
        stats[i] += _calc_stat_bonus(OF_STR + i);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    int i;

    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_LITE);
    add_flag(flgs, OF_RES_BLIND);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_LEVITATION);
    if (p_ptr->lev >= 45) add_flag(flgs, OF_AURA_REVENGE);

    for (i = 0; i < 6; i++) /* Assume in order */
    {
        if (i != 0) /* Bug: Giving TR_STR marks the player as cursed ... */
            _add_stat_flag(OF_STR + i, flgs);
        if (_essences[OF_SUST_STR + i] >= 5)
            add_flag(flgs, OF_SUST_STR + i);
    }

    for (i = 0; i < RES_MAX; i++)
    {
        int j = res_get_object_flag(i);
        int n = _calc_amount(_essences[j], _res_power(i), 1);

        if (n)
            add_flag(flgs, j);
    }

    if (_calc_amount(_essences[OF_STEALTH], 2, 1))
        add_flag(flgs, OF_STEALTH);
    if (_calc_amount(_essences[OF_MAGIC_MASTERY], 2, 1))
        add_flag(flgs, OF_MAGIC_MASTERY);
    if (_calc_amount(_essences[OF_SEARCH], 2, 1))
        add_flag(flgs, OF_SEARCH);
    if (_calc_amount(_essences[OF_INFRA], 2, 1))
        add_flag(flgs, OF_INFRA);
    if (_calc_amount(_essences[OF_TUNNEL], 2, 1))
        add_flag(flgs, OF_TUNNEL);

    if (_essences[OF_TELEPATHY] >= 2)
        add_flag(flgs, OF_TELEPATHY);

    if (_essences[OF_ESP_EVIL] >= 2)
        add_flag(flgs, OF_ESP_EVIL);
    if (_essences[OF_ESP_GOOD] >= 2)
        add_flag(flgs, OF_ESP_GOOD);
    if (_essences[OF_ESP_NONLIVING] >= 2)
        add_flag(flgs, OF_ESP_NONLIVING);
	if (_essences[OF_ESP_LIVING] >= 2)
		add_flag(flgs, OF_ESP_LIVING);
    if (_essences[OF_ESP_UNIQUE] >= 2)
        add_flag(flgs, OF_ESP_UNIQUE);
    if (_essences[OF_ESP_DRAGON] >= 2)
        add_flag(flgs, OF_ESP_DRAGON);
    if (_essences[OF_ESP_DEMON] >= 2)
        add_flag(flgs, OF_ESP_DEMON);
    if (_essences[OF_ESP_UNDEAD] >= 2)
        add_flag(flgs, OF_ESP_UNDEAD);
    if (_essences[OF_ESP_ANIMAL] >= 2)
        add_flag(flgs, OF_ESP_ANIMAL);
    if (_essences[OF_ESP_HUMAN] >= 2)
        add_flag(flgs, OF_ESP_HUMAN);
    if (_essences[OF_ESP_ORC] >= 2)
        add_flag(flgs, OF_ESP_ORC);
    if (_essences[OF_ESP_TROLL] >= 2)
        add_flag(flgs, OF_ESP_TROLL);
    if (_essences[OF_ESP_GIANT] >= 2)
        add_flag(flgs, OF_ESP_GIANT);

    if (_essences[OF_NO_MAGIC] >= 5)
        add_flag(flgs, OF_NO_MAGIC);
    if (_essences[OF_FREE_ACT] >= 2)
        add_flag(flgs, OF_FREE_ACT);
    if (_essences[OF_SEE_INVIS] >= 3)
        add_flag(flgs, OF_SEE_INVIS);
    if (_essences[OF_SLOW_DIGEST] >= 2)
        add_flag(flgs, OF_SLOW_DIGEST);
    if (_essences[OF_REGEN] >= 7)
        add_flag(flgs, OF_REGEN);
    if (_essences[OF_REFLECT] >= 3)
        add_flag(flgs, OF_REFLECT);

    if (_essences[OF_AURA_FIRE] >= 7)
        add_flag(flgs, OF_AURA_FIRE);
    if (_essences[OF_AURA_ELEC] >= 7)
        add_flag(flgs, OF_AURA_ELEC);
    if (_essences[OF_AURA_COLD] >= 7)
        add_flag(flgs, OF_AURA_COLD);

    if (_essences[OF_IM_ACID] >= 3)
        add_flag(flgs, OF_IM_ACID);
    if (_essences[OF_IM_ELEC] >= 3)
        add_flag(flgs, OF_IM_ELEC);
    if (_essences[OF_IM_FIRE] >= 3)
        add_flag(flgs, OF_IM_FIRE);
    if (_essences[OF_IM_COLD] >= 3)
        add_flag(flgs, OF_IM_COLD);
}

/**********************************************************************
 * Powers
 **********************************************************************/
static void _absorb_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Absorb Weapon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroys a single weapon, absorbing the essence of its power.");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        char o_name[MAX_NLEN];
        bool tunnettu;

        var_set_bool(res, FALSE);
        prompt.prompt = "Absorb which item?";
        prompt.error = "You have nothing to absorb.";
        prompt.filter = object_is_melee_weapon;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;

        tunnettu = (((obj_is_identified(prompt.obj)) || (prompt.obj->feeling == FEEL_AVERAGE)) && (obj_is_identified_fully(prompt.obj)));
        if (!tunnettu) obj_identify_fully(prompt.obj);
        object_desc(o_name, prompt.obj, OD_NAME_ONLY);
        msg_format("You absorb the power of %s!", o_name);

        _absorb(prompt.obj);

        prompt.obj->number = 0;
        obj_release(prompt.obj, 0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Weapons");
        break;
    case SPELL_DESC:
        var_set_string(res, "Locate nearby weapons, animated or not.");
        break;
    case SPELL_CAST:
    {
        int rng = DETECT_RAD_DEFAULT;
        int i, y, x;
        bool detect = FALSE;

        if (d_info[dungeon_type].flags1 & DF1_DARKNESS) rng /= 3;

        for (i = 1; i < o_max; i++)
        {
            object_type *o_ptr = &o_list[i];

            if (!o_ptr->k_idx) continue;
            if (o_ptr->held_m_idx) continue;
            y = o_ptr->loc.y;
            x = o_ptr->loc.x;
            if (distance(py, px, y, x) > rng) continue;
            if (!object_is_melee_weapon(o_ptr)) continue;
            o_ptr->marked |= OM_FOUND;
            p_ptr->window |= PW_OBJECT_LIST;
            lite_spot(y, x);
            detect = TRUE;
        }
        if (detect_monsters_string(DETECT_RAD_DEFAULT, "|/"))
            detect = TRUE;

        if (detect)
            msg_print("You sense your kind.");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _judge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Identify Weapon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Identifies a weapon.");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 35)
            var_set_bool(res, identify_fully(object_is_melee_weapon));
        else
            var_set_bool(res, ident_spell(object_is_melee_weapon));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _get_powers[] =
{
    { A_STR, {  1,  0,  0, _absorb_spell } },
    { A_STR, {  5,  1, 30, _detect_spell } },
    { A_STR, { 10, 10, 50, _judge_spell } },
    /*{ A_STR, { 25, 30, 60, _animate_spell } },*/
    {    -1, { -1, -1, -1, NULL}}
};

/**********************************************************************
 * Birth and Evolution
 **********************************************************************/
static void _birth(void) 
{ 
    object_type forge;
    int i;

    for (i = 0; i < _MAX_ESSENCE; i++)
        _essences[i] = 0;

    p_ptr->current_r_idx = MON_BROKEN_DEATH_SWORD;
    equip_on_change_race();

    object_prep(&forge, lookup_kind(TV_SWORD, SV_BROKEN_SWORD));
    add_flag(forge.flags, OF_NO_REMOVE);
    forge.to_h =  1;
    forge.to_d =  3;
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
}

static object_type *_weapon(void)
{
    return equip_obj(p_ptr->weapon_info[0].slot);
}

static void _upgrade_weapon(int tval, int sval)
{
    object_type *o_ptr = _weapon();

    object_prep(o_ptr, lookup_kind(tval, sval));
    o_ptr->to_h = p_ptr->lev / 5;
    o_ptr->to_d = p_ptr->lev / 3;

    add_flag(o_ptr->flags, OF_NO_REMOVE);
    obj_identify_fully(o_ptr);

    p_ptr->update |= PU_BONUS;
    p_ptr->window |= PW_INVEN | PW_EQUIP;
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_BROKEN_DEATH_SWORD && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_DEATH_SWORD;
        equip_on_change_race();
        _upgrade_weapon(TV_SWORD, SV_LONG_SWORD);
        msg_print("You have evolved into a Death Sword.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEATH_SWORD && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_POLEAXE_OF_ANIMATED_ATTACK;
        equip_on_change_race();
        _upgrade_weapon(TV_POLEARM, SV_BATTLE_AXE);
        msg_print("You have evolved into a Poleaxe of Animated Attack.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_POLEAXE_OF_ANIMATED_ATTACK && new_level >= 35)
    {
        p_ptr->current_r_idx = MON_HELLBLADE;
        equip_on_change_race();
        _upgrade_weapon(TV_SWORD, SV_BLADE_OF_CHAOS);
        msg_print("You have evolved into a Hellblade.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_HELLBLADE && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_DEATH_SCYTHE;
        equip_on_change_race();
        _upgrade_weapon(TV_POLEARM, SV_DEATH_SCYTHE_HACK);
        msg_print("You have evolved into a Death Scythe.");
        p_ptr->redraw |= PR_MAP;
    }

    /* Note: These boosts will be completely wiped out next evolution */
    {
    object_type *o_ptr = _weapon();

        if (one_in_(15) && new_level < 45) /* Sorry, Mr. Death Scythe, but you are too powerful! */
        {
            o_ptr->dd++;
            msg_print("You grow sharper!");
        }
        else if (one_in_(2) || (new_level % 7) == 0)
        {
            o_ptr->to_d++;
            msg_print("You grow more deadly!");
        }
        else
        {
            o_ptr->to_h++;
            msg_print("You grow more accurate!");
        }
    }
}

/**********************************************************************
 * Character Dump
 **********************************************************************/
static void _dump_ability_flag(doc_ptr doc, int which, int threshold, cptr name)
{
    int n = _essences[which];

    if (n > 0)
    {
        if ((p_ptr->free_act) && (which == OF_FREE_ACT))
        {
            doc_printf(doc, "   %-22.22s %5d %5d %4dx\n",
                name,
                n, 
                threshold,
                p_ptr->free_act
            );
        }
        else if ((p_ptr->see_inv) && (which == OF_SEE_INVIS))
        {
            doc_printf(doc, "   %-22.22s %5d %5d %4dx\n",
                name,
                n, 
                threshold,
                p_ptr->see_inv
            );
        }
        else
        {
            doc_printf(doc, "   %-22.22s %5d %5d %5.5s\n",
                name,
                n, 
                threshold,
                n >= threshold ? "Y" : ""
            );
        }
    }
}
static void _dump_bonus_flag(doc_ptr doc, int which, int power, int rep, cptr name)
{
    int n = _essences[which];
    if (n > 0)
    {
        doc_printf(doc, "   %-22.22s %5d %5d %+5d\n",
            name,
            n, 
            _calc_needed(n, power, rep),
            _calc_amount(n, power, rep)
        );
    }
}

static void _character_dump(doc_ptr doc)
{
    int i;
    doc_printf(doc, "<topic:Essences>=================================== <color:keypress>E</color>ssences ==================================\n\n");
    doc_printf(doc, "   <color:G>%-22.22s Total  Need Bonus</color>\n", "Stats");
    for (i = 0; i < 6; i++) /* Assume in order */
        _dump_bonus_flag(doc, OF_STR + i, 3, 1, stat_name_true[A_STR + i]);

    doc_printf(doc, "\n   <color:G>%-22.22s Total  Need Bonus</color>\n", "Skills");
    _dump_bonus_flag(doc, _ESSENCE_TO_HIT, 1, 1, "To Hit");
    _dump_bonus_flag(doc, _ESSENCE_TO_DAM, 1, 1, "To Dam");
    _dump_bonus_flag(doc, _ESSENCE_AC, 2, 10, "To AC");
    _dump_bonus_flag(doc, OF_STEALTH, 2, 1, "Stealth");
    _dump_bonus_flag(doc, OF_SPEED, 1, 6, "Speed");
    if (_essences[OF_BLOWS])
    {
        int blows = _calc_amount(_essences[OF_BLOWS], _rank_decay(32), 1);

        blows = blows * _blows_mult();
        doc_printf(doc, "   %-22.22s %5d %5d %5.5s\n",
            "Attacks",
            _essences[OF_BLOWS], 
            _calc_needed(_essences[OF_BLOWS], _rank_decay(32), 1),
            blows ? format("+%d.%2.2d", blows / 100, blows % 100) : ""
        );
    }
    _dump_bonus_flag(doc, _ESSENCE_XTRA_DICE, _rank_decay(64), 1, "Extra Dice");
    _dump_bonus_flag(doc, OF_LIFE, 7, 1, "Life");
    _dump_bonus_flag(doc, OF_SEARCH, 2, 1, "Searching");
    _dump_bonus_flag(doc, OF_INFRA, 2, 1, "Infravision");
    _dump_bonus_flag(doc, OF_TUNNEL, 2, 1, "Digging");
    _dump_bonus_flag(doc, OF_MAGIC_MASTERY, 2, 1, "Magic Mastery");
    _dump_bonus_flag(doc, OF_LITE, 1, 1, "Light");
 
    doc_printf(doc, "\n   <color:G>%-22.22s Total  Need Bonus</color>\n", "Slays");
    for (i = 0; ; i++)
    {
        int j = _slay_flag_info[i].flag;
        if (j < 0) break;
        _dump_ability_flag(doc, j, _slay_power(i), _slay_flag_info[i].name);
    }

    doc_printf(doc, "\n   <color:G>%-22.22s Total  Need Bonus</color>\n", "Resistances");
    for (i = 0; i < RES_MAX; i++)
        _dump_bonus_flag(doc, res_get_object_flag(i), _res_power(i), 1, format("%^s", res_name(i)));

    _dump_ability_flag(doc, OF_IM_ACID, 3, "Immune Acid");
    _dump_ability_flag(doc, OF_IM_ELEC, 3, "Immune Elec");
    _dump_ability_flag(doc, OF_IM_FIRE, 3, "Immune Fire");
    _dump_ability_flag(doc, OF_IM_COLD, 3, "Immune Cold");

    doc_printf(doc, "\n   <color:G>%-22.22s Total  Need Bonus</color>\n", "Abilities");
    _dump_ability_flag(doc, OF_FREE_ACT, _free_act_needed(), "Free Action");
    _dump_ability_flag(doc, OF_SEE_INVIS, _see_invis_needed(), "See Invisible");
    _dump_ability_flag(doc, OF_SLOW_DIGEST, 2, "Slow Digestion");
    _dump_ability_flag(doc, OF_REGEN, 7, "Regeneration");
    _dump_ability_flag(doc, OF_NO_MAGIC, 5, "Antimagic");
    _dump_ability_flag(doc, OF_REFLECT, 3, "Reflection");
    _dump_ability_flag(doc, OF_AURA_FIRE, 7, "Aura Fire");
    _dump_ability_flag(doc, OF_AURA_ELEC, 7, "Aura Elec");
    _dump_ability_flag(doc, OF_AURA_COLD, 7, "Aura Cold");
    for (i = 0; i < 6; i++) /* Assume in order */
        _dump_ability_flag(doc, OF_SUST_STR + i, 5, format("Sustain %s", stat_name_true[A_STR + i]));

    doc_printf(doc, "\n   <color:G>%-22.22s Total  Need Bonus</color>\n", "ESP");
    _dump_ability_flag(doc, OF_TELEPATHY, 2, "Telepathy");
    _dump_ability_flag(doc, OF_ESP_ANIMAL, 2, "ESP Animals");
    _dump_ability_flag(doc, OF_ESP_UNDEAD, 2, "ESP Undead");
    _dump_ability_flag(doc, OF_ESP_DEMON, 2, "ESP Demon");
    _dump_ability_flag(doc, OF_ESP_ORC, 2, "ESP Orc");
    _dump_ability_flag(doc, OF_ESP_TROLL, 2, "ESP Troll");
    _dump_ability_flag(doc, OF_ESP_GIANT, 2, "ESP Giant");
    _dump_ability_flag(doc, OF_ESP_DRAGON, 2, "ESP Dragon");
    _dump_ability_flag(doc, OF_ESP_HUMAN, 2, "ESP Human");
    _dump_ability_flag(doc, OF_ESP_EVIL, 2, "ESP Evil");
    _dump_ability_flag(doc, OF_ESP_GOOD, 2, "ESP Good");
    _dump_ability_flag(doc, OF_ESP_NONLIVING, 2, "ESP Nonliving");
	_dump_ability_flag(doc, OF_ESP_LIVING, 2, "ESP Living");
    _dump_ability_flag(doc, OF_ESP_UNIQUE, 2, "ESP Unique");

    doc_newline(doc);
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_sword_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[5] =  {"Broken Death Sword", "Death Sword", "Poleaxe of Animated Attack", 
                                "Hellblade", "Death Scythe"};    
    int           rank = _rank();

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  24,  40,   4,  14,   5,  56,  20};
    skills_t xs = { 12,   8,  12,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Death-Sword";
        me.desc = "Death-Swords are mighty weapons animated by magical means. As such, "
                    "they are unable to use equipment the way other players can; instead, "
                    "they simply are a weapon of their current form. But never fear: Death "
                    "Swords have the power to absorb magical essences from the weapons they "
                    "find, gaining power in the process.\n\n"
                    "As a Death-Sword gains levels, it will evolve into stronger and stronger weapons. "
                    "More advanced weapons require more essences to enchant, sometimes causing a "
                    "temporary loss of abilities; but magical weapons also become easier to "
                    "find as you descend deeper into the pits, and so any lost powers are usually "
                    "quickly recovered.";

        me.infra = 3;
        me.exp = 150;
        me.base_hp = 30;
        me.shop_adjust = 110; /* I think shopkeepers are puzzled, more than anything else! */

        me.calc_bonuses = _calc_bonuses;
        me.calc_stats = _calc_stats;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.character_dump = _character_dump;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.get_powers = _get_powers;
        me.birth = _birth;

        me.load_player = _load;
        me.save_player = _save;
        me.destroy_object = _absorb_object;

        me.flags = RACE_IS_MONSTER | RACE_IS_NONLIVING | RACE_EATS_DEVICES;
        me.pseudo_class_idx = CLASS_WARRIOR;

        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -5;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  0 + rank/2;
    me.stats[A_CON] =  1 + rank;
    me.stats[A_CHR] =  0;
    me.life = 85 + 5*rank;
    me.boss_r_idx = MON_STORMBRINGER;

    me.equip_template = mon_get_equip_template();

    if (birth_hack || spoiler_hack)
    {
        me.subname = NULL;
        me.subdesc = NULL;
    }
    return &me;
}

bool sword_disenchant(void)
{
    bool result = FALSE;
    int  r = _rank();
    int  i;

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        int n = _essences[i];
        
        if (!n) continue;
        if (i == OF_SPEED) continue;
        if (res_save(RES_DISEN, 44)) continue;
        
        _essences[i] -= isompi(1, _essences[i] * randint1(r) / 20);
        if (_essences[i] < n)
            result = TRUE;
    }
    if (result)
    {
        p_ptr->update |= PU_BONUS;
        msg_print("You feel power draining from your body!");
    }
    return result;
}


int sword_calc_torch(void)
{
    return 1 + _calc_amount(_essences[OF_LITE], 1, 1);
}
