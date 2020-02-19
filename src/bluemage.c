#include "angband.h"

/* Tera-Hack: The old blue-mage spell system with semi-normal spell codes,
 * spell casting levels, mana costs, etc. is completely inapplicable in modern
 * versions (monster spells are now essentially open-ended, no longer exist as
 * flags and cannot be presented in a single neat table, so this data can't be
 * assigned for each of them individually in a sane manner). To get around
 * this, we piggyback on Possessor code by teaching all spells learned to the
 * monster race Sexy Swimsuit (which can never generate in the wild, since it
 * is itself a hack to accommodate Sexy Rags as a player-monster), and then
 * cast spells like a Sexy Swimsuit Possessor but with a secondary hack to
 * guesstimate reasonable spell powers and levels. */

static void _dump(doc_ptr doc)
{
    monster_race *r_ptr = &r_info[MON_SEXY_SWIMSUIT];
    int i;
    vec_ptr spells;
    if ((!r_ptr) || (!r_ptr->name) || (!r_ptr->spells)) return;
    spells = mon_spells_all(r_ptr->spells);
    blue_mage_update_parms(spells);
    doc_insert(doc, "<topic:Blue Magic>================================== <color:keypress>B</color>lue Magic =================================\n\n");

    doc_insert(doc, "<color:G><tab:27>Lvl Cost Fail Desc           Cast</color>\n");

    for (i = 0; i < vec_length(spells); i++)
    {
        mon_spell_ptr spell = vec_get(spells, i);
        mon_spell_doc(spell, doc);
        doc_printf(doc, "<tab:27>%3d%5d%4d%%", spell->prob, mon_spell_cost(spell, r_ptr), blue_mage_spell_fail_rate(spell));
        list_spell_info(doc, spell, r_ptr);
        doc_printf(doc, "<tab:56>%4d\n", spell->lore);
    }
    vec_free(spells);

/*    for (i = 0; i < MST_COUNT; i++)
    {
        mon_spell_group_ptr _group;
        if (!r_ptr->spells->groups[i]) continue;
        _group = r_ptr->spells->groups[i];
        if (!_group->count) continue;
        for (j = 0; j < _group->count; j++)
        {
            mon_spell_ptr spell = &_group->spells[j];
            mon_spell_doc(spell, doc);
            doc_printf(doc, "<tab:27>%3d%5d%4d%%", spell->prob, mon_spell_cost(spell, r_ptr), blue_mage_spell_fail_rate(spell));
            list_spell_info(doc, spell, r_ptr);
            doc_printf(doc, "<tab:56>%4d\n", spell->lore);
        }
    }*/

    doc_printf(doc, "\n");
}

void _learning_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Learning");
        break;
    case SPELL_DESC:
        var_set_string(res, "Study the spells monsters use against you.");
        break;
    case SPELL_CAST:
        if (p_ptr->action == ACTION_LEARN)
            set_action(ACTION_NONE);
        else
            set_action(ACTION_LEARN);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
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
    spell->fn = _learning_spell;

    return ct;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "power";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_WAND, EFFECT_BOLT_MISSILE, 1);
    new_mane = FALSE;
}

static void _save(savefile_ptr file)
{
    monster_race *r_ptr = &r_info[MON_SEXY_SWIMSUIT];
    if ((!r_ptr) || (!r_ptr->name) || (!r_ptr->spells))
    {
        savefile_write_u16b(file, 0xFFF7);
        return;
    }
    else
    {
        int i;
        savefile_write_u16b(file, MST_COUNT);
        for (i = 0; i < MST_COUNT; i++)
        {
            if (!mon_race_has_spell_type(r_ptr, i))
            {
                savefile_write_byte(file, 0);
            }
            else
            {
                int j;
                mon_spell_group_ptr spells = r_ptr->spells->groups[i];
                savefile_write_byte(file, spells->count);
                for (j = 0; j < spells->count; j++)
                {
                    savefile_write_s16b(file, spells->spells[j].id.effect);
                    savefile_write_s16b(file, spells->spells[j].lore);
                    savefile_write_s16b(file, (s16b)blue_mage_spell_order(i, spells->spells[j].id.effect));
                }
            }
        }
    }
}

static void _load(savefile_ptr file)
{
    int i;
    u16b tmp16u = savefile_read_u16b(file);
    if (tmp16u == 0xFFF7) return;
    for (i = 0; i < (int)tmp16u; i++)
    {
        int j;
        byte count = savefile_read_byte(file);
        for (j = 0; j < count; j++)
        {
            s16b effect = savefile_read_s16b(file);
            s16b lore = savefile_is_older_than(file, 7, 1, 1, 2) ? 0 : savefile_read_s16b(file);
            s16b seniority = savefile_is_older_than(file, 7, 1, 1, 3) ? 0 : savefile_read_s16b(file);
            if ((i < MST_COUNT) && (!p_ptr->is_dead)) blue_mage_learn_spell_aux(i, effect, lore, seniority, FALSE);
        }
    }
    new_mane = FALSE;
}

class_t *blue_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  40,  36,   3,  20,  16,  40,  25};
    skills_t xs = {  7,  15,  11,   0,   0,   0,   6,   7};

        me.name = "Blue-Mage";
        me.desc = "Blue-Mages, like all mages, rely on magic rather than muscle; "
                    "but they are unique in the way they gain new spells, as they "
                    "learn from monsters rather than books. Blue-Mages' primary "
                    "spellcasting stat is Intelligence.\n \n"
                    "A Blue-Mage can learn and cast monster spells, summons, and even "
                    "ranged attacks as their own spells; this technique is called Blue magic. "
                    "Blue-Mages remember their spells permanently, but to learn a spell they "
                    "must be directly hit by it while their 'Learning' class power is "
                    "active. Because of this requirement, Blue-Mages never learn spells like "
                    "Haste or Healing that affect the monster itself.";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  4;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 98;
        me.base_hp = 4;
        me.exp = 130;
        me.pets = 35;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.caster_info = _caster_info;
        me.save_player = _save;
        me.load_player = _load;
        /*TODO: me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = _dump;
        init = TRUE;
    }

    return &me;
}
