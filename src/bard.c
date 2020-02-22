#include "angband.h"

void bard_check_music(void)
{
    magic_type *s_ptr;
    int spell;
    s32b need_mana;
    u32b need_mana_frac;

    if (p_ptr->pclass != CLASS_BARD) return;
    if (!p_ptr->magic_num1[0] && !p_ptr->magic_num1[1]) return;

    if (p_ptr->anti_magic)
    {
        bard_stop_singing();
        return;
    }

    spell = p_ptr->magic_num2[0];
    s_ptr = &technic_info[REALM_MUSIC - MIN_TECHNIC][spell];

    need_mana = mod_need_mana(s_ptr->smana, spell, REALM_MUSIC);
    need_mana_frac = 0;

    /* Divide by 2 */
    s64b_RSHIFT(need_mana, need_mana_frac, 1);

    if (s64b_cmp(p_ptr->csp, p_ptr->csp_frac, need_mana, need_mana_frac) < 0)
    {
        bard_stop_singing();
        return;
    }
    else
    {
        s64b_sub(&(p_ptr->csp), &(p_ptr->csp_frac), need_mana, need_mana_frac);

        p_ptr->redraw |= PR_MANA;
        if (p_ptr->magic_num1[1])
        {
            p_ptr->magic_num1[0] = p_ptr->magic_num1[1];
            p_ptr->magic_num1[1] = 0;
            msg_print("You restart singing.");
            p_ptr->action = ACTION_SING;

            p_ptr->update |= (PU_BONUS | PU_HP);
            p_ptr->redraw |= (PR_MAP | PR_STATUS | PR_STATE);
            p_ptr->update |= (PU_MONSTERS);
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        }
    }
    if (p_ptr->spell_exp[spell] < SPELL_EXP_BEGINNER)
        p_ptr->spell_exp[spell] += 5;
    else if(p_ptr->spell_exp[spell] < SPELL_EXP_SKILLED)
    { if (one_in_(2) && (dun_level > 4) && ((dun_level + 10) > p_ptr->lev)) p_ptr->spell_exp[spell] += 1; }
    else if(p_ptr->spell_exp[spell] < SPELL_EXP_EXPERT)
    { if (one_in_(5) && ((dun_level + 5) > p_ptr->lev) && ((dun_level + 5) > s_ptr->slevel)) p_ptr->spell_exp[spell] += 1; }
    else if(p_ptr->spell_exp[spell] < SPELL_EXP_MASTER)
    { if (one_in_(5) && ((dun_level + 5) > p_ptr->lev) && (dun_level > s_ptr->slevel)) p_ptr->spell_exp[spell] += 1; }

    /* Do any effects of continual song */
    do_spell(REALM_MUSIC, spell, SPELL_CONT);
}

void bard_start_singing(int spell, int song)
{
    if (p_ptr->pclass != CLASS_BARD) return;

    /* Remember the song index */
    p_ptr->magic_num1[0] = song;

    /* Remember the index of the spell which activated the song */
    p_ptr->magic_num2[0] = spell;

    set_action(ACTION_SING);
    p_ptr->update |= (PU_BONUS);
    p_ptr->redraw |= (PR_STATUS);
}

void bard_stop_singing(void)
{
    if (p_ptr->pclass != CLASS_BARD) return;

     /* Are there interupted song? */
    if (p_ptr->magic_num1[1])
    {
        /* Forget interupted song */
        p_ptr->magic_num1[1] = 0;
        return;
    }

    /* The player is singing? */
    if (!p_ptr->magic_num1[0]) return;

    /* Hack -- if called from set_action(), avoid recursive loop */
    if (p_ptr->action == ACTION_SING) set_action(ACTION_NONE);

    /* Message text of each song or etc. */
    do_spell(REALM_MUSIC, p_ptr->magic_num2[0], SPELL_STOP);

    p_ptr->magic_num1[0] = MUSIC_NONE;
    p_ptr->magic_num2[0] = 0;
    p_ptr->update |= (PU_BONUS);
    p_ptr->redraw |= (PR_STATUS);
}

static void _stop_singing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stop Singing");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_FLAGS:
        var_set_int(res, PWR_AFRAID | PWR_CONFUSED);
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!p_ptr->magic_num1[0] && !p_ptr->magic_num1[1]) return;
        bard_stop_singing();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}



static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    spell_info* spell = &spells[ct++];
    spell->level = 1;
    spell->cost = 0;
    spell->fail = 0;
    spell->fn = _stop_singing_spell;

    return ct;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "song";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 67;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

static void _calc_bonuses(void)
{
    res_add(RES_SOUND);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SOUND);
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_spellbooks();
}

class_t *bard_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  33,  34,  -5,  16,  20,  34,  20};
    skills_t xs = {  8,  13,  11,   0,   0,   0,  10,   8};

        me.name = "Bard";
        me.desc = "Bards are traditional musicians, and songs and sound are the "
                    "foundation of their magic. Four songbooks are available to a Bard, "
                    "two of them sold in towns. Some songs are over in a quick yelp, "
                    "consuming no more time than a magic spell would; but most continue to be "
                    "sung, with continuing effects, until the bard either chooses to stop or "
                    "runs out of mental energy. A bard can only ever sing one tune at "
                    "a time. The primary spell stat of a bard is Charisma.";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] =  4;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 140;
        me.pets = 25;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;        
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
