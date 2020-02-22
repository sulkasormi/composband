#include "angband.h"

static byte *photos_sold; /* List of monster races by photographs sold */

static void _take_photo_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Take Photograph");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates something to show the kids back home!");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, TRUE);
        if (!get_fire_dir(&dir)) return;
        project_length = 1;
        fire_beam(GF_PHOTO, dir, 1);
        var_set_bool(res, TRUE);

        /* Maybe vampires shouldn't do photography... */
        if ((prace_is_(RACE_VAMPIRE)) && (!res_save(RES_LITE, 20)))
        {
            int dam = res_calc_dam(RES_LITE, 20);
            msg_print("You cringe from the flash!");
            take_hit(DAMAGE_NOESCAPE, dam, "taking a photograph");
        }
        break;
    }
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
    spell->fn = _take_photo_spell;

    spell = &spells[ct++];
    spell->level = 25;
    spell->cost = 20;
    spell->fail = calculate_fail_rate(spell->level, 30, p_ptr->stat_ind[A_INT]);
    spell->fn = identify_fully_spell;

    return ct;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 67;
        me.encumbrance.enc_wgt = 800;
        me.min_fail = 5;
        me.min_level = 5;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _ini_photo_list(void)
{
    C_MAKE(photos_sold, max_r_idx, byte);
}

static void _load_player(savefile_ptr file)
{
    u32b old_max_race;
    unsigned int i;
    _ini_photo_list();
    if (savefile_is_older_than(file, 7, 0, 5, 2)) return;
    old_max_race = savefile_read_u32b(file);
    for (i = 0; i < old_max_race; i++)
    {
        if (i < max_r_idx) photos_sold[i] = savefile_read_byte(file);
        else (void)savefile_read_byte(file);
    }
}

static void _save_player(savefile_ptr file)
{
    int i;
    savefile_write_u32b(file, (u32b)max_r_idx);
    for (i = 0; i < max_r_idx; i++)
    {
        savefile_write_byte(file, photos_sold[i]);
    }
}

static void _birth(void)
{
    py_birth_obj_aux(TV_FOOD, SV_FOOD_BISCUIT, rand_range(2, 4));
    py_birth_obj_aux(TV_FOOD, SV_FOOD_WAYBREAD, rand_range(2, 4));
    py_birth_obj_aux(TV_FOOD, SV_FOOD_JERKY, rand_range(1, 3));
    py_birth_obj_aux(TV_FOOD, SV_FOOD_PINT_OF_ALE, rand_range(2, 4));
    py_birth_obj_aux(TV_FOOD, SV_FOOD_PINT_OF_WINE, rand_range(2, 4));
    py_birth_obj_aux(TV_FOOD, SV_FOOD_BISCUIT, rand_range(2, 4));
    py_birth_obj_aux(TV_SCROLL, SV_SCROLL_MAPPING, rand_range(2, 5));
    py_birth_obj_aux(TV_BOW, SV_SLING, 1);
    py_birth_obj_aux(TV_SHOT, SV_PEBBLE, rand_range(20, 40));
    p_ptr->au += 2000;
    _ini_photo_list();
}

int tourist_sell_photo_aux(object_type *o_ptr, int amount, bool merkitse)
{
    monster_race *r_ptr;
    int myyty, laji, hyvaksy, tarjous = 0;
    if (!o_ptr->number) return 0;
    laji = o_ptr->pval;
    if ((o_ptr->tval != TV_STATUE) || (o_ptr->sval != SV_PHOTO) || (laji < 1) || (laji >= max_r_idx)) return 0;
    if (merkitse)
    {
        photos_sold[laji] += amount;
        return 0;
    }
    myyty = photos_sold[laji];
    r_ptr = &r_info[laji];
    if (!r_ptr || !r_ptr->name) return 0;
    hyvaksy = MIN(15 - myyty, amount);
    if (hyvaksy < 1) return -1;
    tarjous = ((r_ptr->level + 10) / 10) * hyvaksy;
    if (r_ptr->flags1 & RF1_UNIQUE) tarjous += (15 * hyvaksy);
    if ((!myyty) && (laji == MON_SASQUATCH)) tarjous += (((seed_town % 3) * 1000) + 3000);
    else if ((!myyty) && (laji == MON_TSUCHINOKO)) tarjous += (((seed_town % 3) * 800) + 2400);
    return tarjous;
}

class_t *tourist_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 15,  18,  28,   1,  12,   2,  46,  19};
    skills_t xs = {  5,   6,   9,   0,   0,   0,  12,  10};

        me.name = "Tourist";
        me.desc = "Tourists have visited this world for the purpose of sightseeing. "
                    "Their fighting skills are bad, and they cannot cast powerful "
                    "spells. They are the most difficult class to win the game with. "
                    "Intelligence determines a tourist's spellcasting ability.\n \n"
                    "Tourists are always seeing more of the world to add to their stock "
                    "of information; no other class can compete with their "
                    "identification skills. They have two class powers - 'Take a "
                    "Photograph' and 'Identify True'. Their magic is based on Arcane, "
                    "and - aside from identify - is very weak indeed.";

        me.stats[A_STR] = -2;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -3;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 94;
        me.base_hp = 0;
        me.exp = 70;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        me.load_player = _load_player;
        me.save_player = _save_player;
        init = TRUE;
    }

    return &me;
}
