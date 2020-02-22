#include "angband.h"

void _precognition_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Precognition");
        break;
    case SPELL_DESC:
    {
        if (p_ptr->lev < 5)
            var_set_string(res, "Detects visible monsters in your vicinity.");
        else if (p_ptr->lev < 15)
            var_set_string(res, "Detects visible monsters, traps, and doors in your vicinity.");
        else if (p_ptr->lev < 20)
            var_set_string(res, "Detects monsters, traps, and doors in your vicinity.");
        else if (p_ptr->lev < 25)
            var_set_string(res, "Detects monsters, traps, and doors in your vicinity and maps nearby area.");
        else if (p_ptr->lev < 30)
            var_set_string(res, "Detects monsters, traps, and doors in your vicinity and maps nearby area. Grants temporary ESP.");
        else if (p_ptr->lev < 40)
            var_set_string(res, "Detects monsters, traps, doors, stairs and objects in your vicinity and maps nearby area. Grants temporary ESP.");
        else if (p_ptr->lev < 45)
            var_set_string(res, "Detects monsters, traps, doors, stairs and objects in your vicinity and maps nearby area.");
        else
            var_set_string(res, "Detects monsters, traps, doors, stairs and objects in your vicinity and maps the entire level.");
        break;
    }
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Detects monsters (L1), traps and doors (L5), invisible monsters (L15) and items (L30). Gives magic mapping (L20) and telepathy (L25). Enlightens level (L45).");
        break;
    case SPELL_CAST:
    {
        int b = 0;
        if (p_ptr->lev > 44)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
        }
        else if (p_ptr->lev > 19)
            map_area(DETECT_RAD_MAP);

        if (p_ptr->lev < 30)
        {
            b = detect_monsters_normal(DETECT_RAD_DEFAULT);
            if (p_ptr->lev > 14) b |= detect_monsters_invis(DETECT_RAD_DEFAULT);
            if (p_ptr->lev > 4)  {
                b |= detect_traps(DETECT_RAD_DEFAULT, TRUE);
                b |= detect_doors(DETECT_RAD_DEFAULT);
            }
        }
        else
        {
            b = detect_all(DETECT_RAD_DEFAULT);
        }

        if ((p_ptr->lev > 24) && (p_ptr->lev < 40))
            set_tim_esp(p_ptr->lev + randint1(p_ptr->lev), FALSE);

        if (!b) msg_print("You feel safe.");

        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
    {
        int n = 0;

        if (p_ptr->lev >= 45)
            n += 9;
        else if (p_ptr->lev >= 30)
            n += 4;
        else if (p_ptr->lev >= 25)
            n += 3;
        else if (p_ptr->lev >= 20)
            n += 1;
        else if (p_ptr->lev >= 15)
            n += 0;
        else if (p_ptr->lev >= 5)
            n += 0;

        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _neural_blast_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Neural Blast");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam or ball which inflicts psionic damage.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Fires a beam or ball (Radius 0) which inflicts (3 + (L-1)/4)d(3 + L/15) psionic damage.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(3 + ((p_ptr->lev - 1) / 4)), 3 + p_ptr->lev / 15, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dice = 3 + ((p_ptr->lev - 1) / 4);
        int sides = (3 + p_ptr->lev / 15);
        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;

        if (randint1(100) < p_ptr->lev * 2)
            fire_beam(GF_PSI, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        else
            fire_ball(GF_PSI, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell), 0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _minor_displacement_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->lev >= 45)
            var_set_string(res, "Dimension Door");
        else
            var_set_string(res, "Minor Displacement");
        break;
    case SPELL_SPOIL_NAME:
        var_set_string(res, "Minor Displacement");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 45)
            var_set_string(res, "Attempts to teleport to a specific location.");
        else
            var_set_string(res, "Teleports a short distance.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Teleports the player (Range 10). At L45, gives dimension door instead (Range L/2 + 10).");
        break;
    case SPELL_CAST:
    {
        if (p_ptr->lev >= 45)
            var_set_bool(res, dimension_door(p_ptr->lev / 2 + 10));
        else
        {
            teleport_player(10, 0L);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        if (p_ptr->lev >= 45)
            n += 40;
        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _major_displacement_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Major Displacement");
        break;
    case SPELL_DESC:
        var_set_string(res, "Teleports a long distance.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Teleports the player (Range L*5).");
        break;
    case SPELL_CAST:
    {
        teleport_player(p_ptr->lev * 5, 0L);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _domination_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Domination");
        break;
    case SPELL_DESC:
        var_set_string(res, "Stuns, confuses, scares or charms a monster. From level 30, attempts to charm all monsters in sight.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Stuns, confuses, scares or charms a monster. Or attempts to charm all monsters in sight at L30.");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;

            fire_ball(GF_DOMINATION, dir, spell_power(p_ptr->lev), 0);
        }
        else
        {
            charm_monsters(spell_power((p_ptr->lev * 3 / 2) + 15));
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _pulverise_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Pulverise");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball which hurts monsters with telekinesis.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Fires a ball (Radius 0 or (L-20)/8 + 1) of Telekinesis (Damage (8 + (L-5)/4)d8).");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(8 + ((p_ptr->lev - 5) / 4)), 8, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dice = 8 + ((p_ptr->lev - 5) / 4);
        int sides = 8;
        int rad = p_ptr->lev > 20 ? spell_power((p_ptr->lev - 20) / 8 + 1) : 0;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(
            GF_TELEKINESIS,
            dir,
            spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
            rad
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _character_armor_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Character Armour");
        break;
    case SPELL_DESC:
        var_set_string(res, "Turns your skin to stone and provides temporary resistance to the elements. "
                              "More resistances are given at higher levels.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Gives Stone Skin, Resist Acid (L15), Resist Fire (L20), Resist Cold (L25), Resist Lightning (L30) and Resist Poison (L35).");
        break;
    case SPELL_CAST:
    {
        int dur = spell_power(p_ptr->lev + randint1(p_ptr->lev));
        set_shield(dur, FALSE);
        if (p_ptr->lev > 14) set_oppose_acid(dur, FALSE);
        if (p_ptr->lev > 19) set_oppose_fire(dur, FALSE);
        if (p_ptr->lev > 24) set_oppose_cold(dur, FALSE);
        if (p_ptr->lev > 29) set_oppose_elec(dur, FALSE);
        if (p_ptr->lev > 34) set_oppose_pois(dur, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _psychometry_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Psychometry");
        break;
    case SPELL_DESC:
        var_set_string(res, "Returns a feeling about an item. Identifies the item at level 20 and above.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Pseudo-identifies an object. At L20, identifies an object instead.");
        break;
    case SPELL_CAST:
    {
        if (p_ptr->lev < 20)
            var_set_bool(res, psychometry());
        else
            var_set_bool(res, ident_spell(NULL));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _mind_wave_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mind Wave");
        break;
    case SPELL_DESC:
        var_set_string(res, "At low levels, generates a ball of psionic energy centered on you. "
                              "At level 25 and above, inflicts psionic damage on all monsters in line of sight.");
        break;

    case SPELL_SPOIL_DESC:
        var_set_string(res, "Generates a ball (Radius 2 + L/10) of psionic energy (Damage L*3). At L25, damages all monsters in line of sight instead (Damage 1d(L*((L-5)/10 + 1))).");
        break;

    case SPELL_INFO:
        if (p_ptr->lev < 25)
            var_set_string(res, format("dam %d", spell_power(p_ptr->lev * 3 / 2 + p_ptr->to_d_spell)));
        else
            var_set_string(res, info_damage(1, p_ptr->lev * ((p_ptr->lev - 5) / 10 + 1), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        msg_print("Mind-warping forces emanate from your brain!");

        if (p_ptr->lev < 25)
        {
            project(0, 2 + p_ptr->lev / 10, py, px,
                        spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell), GF_PSI, PROJECT_KILL);
        }
        else
        {
            int ds = p_ptr->lev * ((p_ptr->lev - 5) / 10 + 1);
            mindblast_monsters(spell_power(randint1(ds) + p_ptr->to_d_spell));
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _adrenaline_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Adrenaline Channeling");
        break;
    case SPELL_DESC:
        var_set_string(res, "Removes fear and stunning. Gives heroism and speed. Heals HP a little unless "
                              "you already have heroism and temporary speed boost.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Removes stun. Heals 10 + 1d(L*3/2). Grants heroism and haste.");
        break;
    case SPELL_CAST:
    {
        int dur = spell_power(15 + randint1(p_ptr->lev*3/2));
        bool heal = !IS_FAST() || !IS_HERO(); /* Prevent spamming this as a weak healing spell */

        set_stun(0, TRUE);

        set_hero(dur, FALSE);
        set_fast(dur, FALSE);

        if (heal) /* Heal after granting Heroism to fill the +10 mhp */
            hp_player(p_ptr->lev);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _telekinesis_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Telekinesis");
        break;
    case SPELL_DESC:
        var_set_string(res, "Pulls a distant item close to you.");        
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Fetch a nearby object (Weight <= L*15).");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;

        fetch(dir, p_ptr->lev * 15, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _psychic_drain_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Psychic Drain");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball which damages monsters and absorbs their mind power. "
                            "A successful absorption also consumes some of your own energy, costing an additional 0.01 to 1.50 turns.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Drain target monster (Damage (L/2)d6) to regain 5d(damage)/4 spell points. But this spell also consumes 1d150 extra energy.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(p_ptr->lev/2), 6, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(damroll(p_ptr->lev / 2, 6) + p_ptr->to_d_spell);
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        /* Only charge extra energy if the drain succeeded */
        if (fire_ball(GF_PSI_DRAIN, dir, dam, 0))
            p_ptr->energy_need += randint1(150);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void psycho_spear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Psycho-Spear");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam of pure energy which penetrates invulnerability globes.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 3), spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_PSY_SPEAR, dir, spell_power(randint1(p_ptr->lev*3)+p_ptr->lev*3 + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _psycho_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Psycho-Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large ball of pure mental energy.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 5 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_PSI_STORM, dir, spell_power(p_ptr->lev * 5 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
static spell_info _spells[] = 
{
    /*lvl cst fail spell */
    { 1,   1,  15, _neural_blast_spell},
    { 2,   1,  20, _precognition_spell},
    { 3,   2,  25, _minor_displacement_spell},
    { 7,   6,  35, _major_displacement_spell},
    { 9,   7,  50, _domination_spell},
    { 11,  7,  30, _pulverise_spell},
    { 13, 12,  50, _psychometry_spell},
    { 15, 12,  60, _character_armor_spell},
    { 18, 10,  45, _mind_wave_spell},
    { 23, 15,  50, _adrenaline_spell},
    { 26, 28,  60, _telekinesis_spell},
    { 28, 10,  40, _psychic_drain_spell},
    { 35, 35,  75, psycho_spear_spell},
    { 45, 50,  80, _psycho_storm_spell},
    { -1, -1,  -1, NULL}
};

static power_info _powers[] =
{
    { A_WIS, {15, 0, 30, clear_mind_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};

static int _get_spells(spell_info* spells, int max)
{
    return get_spells_aux(spells, max, _spells);
}

static int _get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _powers);
}

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 10) res_add(RES_FEAR);
    if (p_ptr->lev >= 15) p_ptr->clear_mind = TRUE;
    if (p_ptr->lev >= 20) p_ptr->sustain_wis = TRUE;
    if (p_ptr->lev >= 25) p_ptr->auto_id_sp = 12;
    if (p_ptr->lev >= 30) res_add(RES_CONF);
    if (p_ptr->lev >= 40) p_ptr->telepathy = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_RES_FEAR);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_SUST_WIS);
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_CONF);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_TELEPATHY);
}

static void _on_fail(const spell_info *spell)
{
    if (randint1(100) < (spell->fail / 2))
    {
        int b = randint1(100);

        if (b < 5)
        {
            msg_print("Oh, no! Your mind has gone blank!");
            lose_all_info();
        }
        else if (b < 15)
        {
            msg_print("Weird visions seem to dance before your eyes...");
            set_image(p_ptr->image + 5 + randint1(10), FALSE);
        }
        else if (b < 45)
        {
            msg_print("Your brain is addled!");
            set_confused(p_ptr->confused + randint1(8), FALSE);
        }
        else if (b < 90)
        {
            set_stun(p_ptr->stun + randint1(8), FALSE);
        }
        else
        {
            msg_print("Your mind unleashes its power in an uncontrollable storm!");

            project(PROJECT_WHO_UNCTRL_POWER, 2 + p_ptr->lev / 10, py, px, p_ptr->lev * 2,
                GF_MANA, PROJECT_JUMP | PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM);
            p_ptr->csp = MAX(0, p_ptr->csp - p_ptr->lev * MAX(1, p_ptr->lev / 10));
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "mindcraft";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.on_fail = _on_fail;
        init = TRUE;
    }
    return &me;
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    py_display_spells(doc, spells, ct);
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SMALL_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, rand_range(2, 5));
}

class_t *mindcrafter_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  33,  38,   3,  22,  16,  48,  40 };
    skills_t xs = { 10,  11,  10,   0,   0,   0,  12,  17 };

        me.name = "Mindcrafter";
        me.desc = "Mindcrafters rely not on book magic, but on the powers of the mind. "
                    "These mind powers are specific to this class, and range "
                    "from extrasensory perception to psychic attacks and the mental domination "
                    "of others. Most mind powers become accessible fairly early, but gain in "
                    "strength with more experience. Wisdom determines a Mindcrafter's ability to "
                    "use mind powers.\n \n"
                    "Mindcrafters' combat skills are reasonably good but far from outstanding; "
                    "a combination of physical and mental attacks often works better "
                    "for them than either one alone. A seasoned mindcrafter is very hard to "
                    "confuse, and can effortlessly detect the minds of others. Mindcrafters "
                    "have a class power, 'Clear Mind', which allows them to rapidly regenerate "
                    "mana."; 
        me.stats[A_STR] = -1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 99;
        me.base_hp = 4;
        me.exp = 125;
        me.pets = 35;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
